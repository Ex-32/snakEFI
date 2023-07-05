#include <efi.h>
#include "snake.h"
#include "gop.h"
#include "utils.h"
#include "rng.h"

static const UINTN game_width = 10;
static const UINTN game_height = 10;

static CHAR16 notifyChars[8] = { L'w', L'W', L'd', L'D', L's', L'S', L'a', L'A' };
static VOID* notifyHandles[8];

BOOLEAN snakeRunning = TRUE;
BOOLEAN snakeWon = FALSE;
static struct vec2 fruit;
static struct snake_node* snakeFront = NULL;
static struct snake_node* snakeEnd = NULL;
static UINTN snakeLen = 0;
static enum snake_direction directionCurrent;
static volatile enum snake_direction directionNext;

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
        gop, 
        pxPerBlk * value.x , pxPerBlk * value.y, 
        pxPerBlk, pxPerBlk, 
        0xFFFFFF
    ); 
    if (snakeFront == NULL) {
        struct snake_node* node = bmalloc(sizeof(struct snake_node));

        node->prev = NULL;
        node->next = NULL;
        node->value = value;
        
        snakeFront = node;
        snakeEnd = node;
    } else {
        struct snake_node* node = bmalloc(sizeof(struct snake_node));

        node->prev = NULL;
        node->next = snakeFront;
        node->value = value;
        
        snakeFront = node;
    }
    ++snakeLen;
}

struct vec2 snakeEndPop(VOID) {
    struct vec2 value = snakeEnd->value;

    drawRect(
        gop, 
        pxPerBlk * value.x , pxPerBlk * value.y, 
        pxPerBlk, pxPerBlk, 
        0x000000
    ); 

    if (snakeFront == snakeEnd) {
        bfree(snakeEnd);
        snakeEnd = NULL;
        snakeFront = NULL;
    } else {
        struct snake_node* prev = snakeEnd->prev;
        prev->next = NULL;
        bfree(snakeEnd);
        snakeEnd = prev;
    }
    --snakeLen;
    return value;
}

struct vec2 snakeFrontPeak(VOID) {
    return snakeFront->value;
}

VOID snakeInit(EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* inputEx) {
    UINTN hSize = gop->Mode->Info->HorizontalResolution / game_width;
    UINTN vSize = gop->Mode->Info->VerticalResolution / game_height;
    pxPerBlk = hSize < vSize ? hSize : vSize; 

    drawRect(
        gop, 
        0, 
        0, 
        gop->Mode->Info->HorizontalResolution, 
        gop->Mode->Info->VerticalResolution, 
        0x0F0F0F
    );
    drawRect(
        gop, 
        0, 0,
        game_width * pxPerBlk, game_height * pxPerBlk, 
        0x000000
    );

    EFI_TIME time;
    ST->RuntimeServices->GetTime(&time, NULL);
    srand(
        (time.Year + 1) * (time.Month + 1) * (time.Day + 1) *
        (time.Hour + 1) * (time.Minute + 1) * (time.Second) + 
        time.Nanosecond
    );

    fruit = (struct vec2){ rand() % game_width, rand() % game_height };
    snakeFrontAppend((struct vec2){ 
        game_width / 2, 
        game_height / 2,
    });
    directionNext = directionCurrent = rand() % 4;

    for (UINTN i = 0; i < 8; ++i) {
        EFI_KEY_DATA keyConf = { { 0, notifyChars[i] }, { 0, 0 } };
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
            if (old_head.y == game_height - 1) goto DEAD;
            new_head = (struct vec2){ old_head.x, old_head.y + 1 };
            break;
        case snakeRight:
            if (old_head.x == game_width - 1) goto DEAD;
            new_head = (struct vec2){ old_head.x + 1, old_head.y };
            break;
        case snakeLeft:
            if (old_head.x == 0) goto DEAD;
            new_head = (struct vec2){ old_head.x - 1, old_head.y };
            break;
    }

    snakeEndPop();

    struct snake_node* node = snakeFront;
    while (TRUE) {
        if (node == NULL) break;
        if (new_head.x == node->value.x && new_head.y == node->value.y) {
            goto DEAD;
        }
        if (node->next == NULL) break;
        node = node->next;
    }

    snakeFrontAppend(new_head);

    return;  
    DEAD: snakeRunning = FALSE;
}

