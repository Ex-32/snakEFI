#include <efi.h>
#include "snake.h"
#include "gop.h"
#include "utils.h"
#include "rng.h"

static const UINTN GAME_WIDTH = 10;
static const UINTN GAME_HEIGHT = 10;

static VOID* notifyHandles[8];
static CHAR16 notifyChars[8] = {L'w', L'W',
                                L'd', L'D',
                                L's', L'S',
                                L'a', L'A'};

BOOLEAN snakeRunning = TRUE;
BOOLEAN snakeWon = FALSE;
static struct vec2 fruit;
static struct snakeNode* snakeFront = NULL;
static struct snakeNode* snakeEnd = NULL;
UINTN snakeLen = 0;
static enum snakeDirection directionCurrent;
static volatile enum snakeDirection directionNext;

static UINTN pxPerBlk;

EFI_STATUS EFIAPI snakeKeyNotifyHandler(EFI_KEY_DATA* keyData) {
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

VOID snakeFrontAppend(struct vec2 value) {
    drawRect(
        pxPerBlk * value.x , pxPerBlk * value.y,
        pxPerBlk, pxPerBlk,
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

struct vec2 snakeEndPop(VOID) {
    struct vec2 value = snakeEnd->value;

    drawRect(
        pxPerBlk * value.x , pxPerBlk * value.y,
        pxPerBlk, pxPerBlk,
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

inline struct vec2 snakeFrontPeak(VOID) {
    return snakeFront->value;
}

VOID spawnFruit(VOID) {
    struct vec2 newFruit = (struct vec2){
        rand() % GAME_WIDTH,
        rand() % GAME_HEIGHT
    };

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
        pxPerBlk * fruit.x, pxPerBlk * fruit.y,
        pxPerBlk, pxPerBlk,
        0xF38BA8
    );
}

VOID snakeInit(EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* inputEx) {
    const EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* const info = gop->Mode->Info;
    const UINT32 hSize = info->HorizontalResolution / GAME_WIDTH;
    const UINT32 vSize = info->VerticalResolution / GAME_HEIGHT;
    pxPerBlk = hSize < vSize ? hSize : vSize;

    drawRect(
        0, 0,
        info->HorizontalResolution, info->VerticalResolution,
        0x1E1E2E
    );
    drawRect(
        0, 0,
        GAME_WIDTH * pxPerBlk, GAME_HEIGHT * pxPerBlk,
        0x11111B
    );

    EFI_TIME time;
    ST->RuntimeServices->GetTime(&time, NULL);
    srand(
        (time.Year + 1) * (time.Month + 1) * (time.Day + 1) *
        (time.Hour + 1) * (time.Minute + 1) * (time.Second) +
        time.Nanosecond
    );

    spawnFruit();
    snakeFrontAppend((struct vec2){GAME_WIDTH / 2, GAME_HEIGHT / 2});
    directionNext = directionCurrent = rand() % 4;

    for (UINT8 i = 0; i < 8; ++i) {
        EFI_KEY_DATA keyConf = {{0, notifyChars[i]}, {0, 0}};
        inputEx->RegisterKeyNotify(
            inputEx,
            &keyConf,
            &snakeKeyNotifyHandler,
            &(notifyHandles[i])
        );
    }
}

VOID snakeDeinit(EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* inputEx) {
    for (UINTN i = 0; i < 8; ++i) {
        inputEx->UnregisterKeyNotify(inputEx, notifyHandles[i]);
    }
}

VOID snakeDoTick(VOID) {
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
            new_head = (struct vec2){ old_head.x, old_head.y - 1 };
            break;
        case snakeDown:
            if (old_head.y == GAME_HEIGHT - 1) goto DEAD;
            new_head = (struct vec2){ old_head.x, old_head.y + 1 };
            break;
        case snakeRight:
            if (old_head.x == GAME_WIDTH - 1) goto DEAD;
            new_head = (struct vec2){ old_head.x + 1, old_head.y };
            break;
        case snakeLeft:
            if (old_head.x == 0) goto DEAD;
            new_head = (struct vec2){ old_head.x - 1, old_head.y };
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

    return;
    DEAD: snakeRunning = FALSE;
}

