#include "snake.h"
#include "gop.h"
#include "rng.h"
#include "utils.h"
#include <efi/efi.h>

static const UINT16 GAME_WIDTH = 10;
static const UINT16 GAME_HEIGHT = 10;

static void* notifyHandles[8];
static CHAR16 notifyChars[8] = {L'w', L'W', L'd', L'D', L's', L'S', L'a', L'A'};

BOOLEAN snakeWon = FALSE;
UINTN snakeLen = 0;

static struct vec2 fruit;
static struct snakeNode* snakeFront = NULL;
static struct snakeNode* snakeEnd = NULL;

static enum snakeDirection directionCurrent;
static volatile enum snakeDirection directionNext;

static UINTN pxPerBlk;
static UINTN pxOffsetX;
static UINTN pxOffsetY;

static EFI_STATUS EFIAPI snakeKeyNotifyHandler(EFI_KEY_DATA* keyData) {
    switch (keyData->Key.UnicodeChar) {
    case L'w':
    case L'W':
        directionNext = snakeUp;
        break;
    case L'd':
    case L'D':
        directionNext = snakeRight;
        break;
    case L's':
    case L'S':
        directionNext = snakeDown;
        break;
    case L'a':
    case L'A':
        directionNext = snakeLeft;
        break;
    }
    return EFI_SUCCESS;
}

static void snakeFrontAppend(struct vec2 value) {
    drawRect(
        pxOffsetX + (pxPerBlk * value.x),
        pxOffsetY + (pxPerBlk * value.y),
        pxPerBlk,
        pxPerBlk,
        0xA6E3A1
    );
    struct snakeNode* node = bmalloc(sizeof(struct snakeNode));
    node->value = value;
    node->prev = NULL;
    if (snakeFront == NULL) {
        node->next = NULL;
        snakeFront = node;
        snakeEnd = node;
    } else {
        node->next = snakeFront;
        snakeFront->prev = node;
        snakeFront = node;
    }
    ++snakeLen;
}

static struct vec2 snakeEndPop(void) {
    struct vec2 value = snakeEnd->value;

    drawRect(
        pxOffsetX + (pxPerBlk * value.x),
        pxOffsetY + (pxPerBlk * value.y),
        pxPerBlk,
        pxPerBlk,
        0x11111B
    );

    if (snakeFront == snakeEnd) {
        bfree(snakeEnd);
        snakeEnd = NULL;
        snakeFront = NULL;
    } else {
        struct snakeNode* prev = snakeEnd->prev;
        prev->next = NULL;
        bfree(snakeEnd);
        snakeEnd = prev;
    }
    --snakeLen;
    return value;
}

inline static struct vec2 snakeFrontPeak(void) { return snakeFront->value; }

static void spawnFruit(void) {
    struct vec2 newFruit =
        (struct vec2){rand() % GAME_WIDTH, rand() % GAME_HEIGHT};

    struct snakeNode* node = snakeFront;
    while (TRUE) {
        if (newFruit.x == node->value.x && newFruit.y == node->value.y) {
            return spawnFruit();
        }
        if (node->next == NULL) break;
        node = node->next;
    }

    fruit = newFruit;
    drawRect(
        pxOffsetX + (pxPerBlk * fruit.x),
        pxOffsetY + (pxPerBlk * fruit.y),
        pxPerBlk,
        pxPerBlk,
        0xF38BA8
    );
}

void snakeInit(EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* inputEx) {
    const EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* const info = gop->Mode->Info;
    const UINT32 hSize = info->HorizontalResolution / GAME_WIDTH;
    const UINT32 vSize = info->VerticalResolution / GAME_HEIGHT;
    pxPerBlk = hSize < vSize ? hSize : vSize;

    pxOffsetX = (info->HorizontalResolution - (pxPerBlk * GAME_WIDTH)) / 2;
    pxOffsetY = (info->VerticalResolution - (pxPerBlk * GAME_HEIGHT)) / 2;

    drawRect(
        0, 0, info->HorizontalResolution, info->VerticalResolution, 0x1E1E2E
    );
    drawRect(
        pxOffsetX,
        pxOffsetY,
        GAME_WIDTH * pxPerBlk,
        GAME_HEIGHT * pxPerBlk,
        0x11111B
    );

    EFI_TIME time;
    ST->RuntimeServices->GetTime(&time, NULL);
    UINT64 seed = 0;
    seed = (seed << 0 | time.Second);
    seed = (seed << 8 | time.Minute);
    seed = (seed << 8 | time.Hour);
    seed = (seed << 8 | time.Day);
    seed = (seed << 8 | time.Month);
    seed = (seed << 16 | time.Day);
    seed ^= time.Nanosecond;
    srand(seed);

    spawnFruit();
    snakeFrontAppend((struct vec2){GAME_WIDTH / 2, GAME_HEIGHT / 2});
    directionNext = directionCurrent = rand() % 4;

    for (UINT8 i = 0; i < 8; ++i) {
        EFI_KEY_DATA keyConf = {{0, notifyChars[i]}, {0, 0}};
        inputEx->RegisterKeyNotify(
            inputEx, &keyConf, &snakeKeyNotifyHandler, &(notifyHandles[i])
        );
    }
}

void snakeDeinit(EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* inputEx) {
    for (UINTN i = 0; i < 8; ++i) {
        inputEx->UnregisterKeyNotify(inputEx, notifyHandles[i]);
    }
}

BOOLEAN snakeDoTick(void) {
    BOOLEAN newFruit = FALSE;

    switch (directionNext) {
    case snakeUp:
        if (!(directionCurrent == snakeDown)) {
            directionCurrent = directionNext;
        }
        break;
    case snakeDown:
        if (!(directionCurrent == snakeUp)) {
            directionCurrent = directionNext;
        }
        break;
    case snakeRight:
        if (!(directionCurrent == snakeLeft)) {
            directionCurrent = directionNext;
        }
        break;
    case snakeLeft:
        if (!(directionCurrent == snakeRight)) {
            directionCurrent = directionNext;
        }
        break;
    }

    struct vec2 old_head = snakeFrontPeak();
    struct vec2 new_head;
    switch (directionCurrent) {
    case snakeUp:
        if (old_head.y == 0) goto DEAD;
        new_head = (struct vec2){old_head.x, old_head.y - 1};
        break;
    case snakeDown:
        if (old_head.y == GAME_HEIGHT - 1) goto DEAD;
        new_head = (struct vec2){old_head.x, old_head.y + 1};
        break;
    case snakeRight:
        if (old_head.x == GAME_WIDTH - 1) goto DEAD;
        new_head = (struct vec2){old_head.x + 1, old_head.y};
        break;
    case snakeLeft:
        if (old_head.x == 0) goto DEAD;
        new_head = (struct vec2){old_head.x - 1, old_head.y};
        break;
    }

    if (new_head.x == fruit.x && new_head.y == fruit.y) newFruit = TRUE;
    else snakeEndPop();

    struct snakeNode* node = snakeFront;
    while (TRUE) {
        if (node == NULL) break;
        if (new_head.x == node->value.x && new_head.y == node->value.y) {
            goto DEAD;
        }
        if (node->next == NULL) break;
        node = node->next;
    }

    snakeFrontAppend(new_head);

    if (snakeLen == (GAME_WIDTH * GAME_HEIGHT)) {
        snakeWon = TRUE;
        goto DEAD;
    }

    if (newFruit) spawnFruit();

    return TRUE;
DEAD:
    return FALSE;
}
