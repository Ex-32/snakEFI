#pragma once
#include <efi.h>

struct vec2 {
    UINTN x;
    UINTN y;
};

struct snakeNode {
    struct snakeNode* prev;
    struct snakeNode* next;
    struct vec2 value;
};

enum snakeDirection { snakeUp, snakeDown, snakeLeft, snakeRight };

extern BOOLEAN snakeRunning;
extern BOOLEAN snakeWon;
extern UINTN snakeLen;

VOID snakeInit(EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* inputEx);
VOID snakeDeinit(EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* inputEx);
VOID snakeDoTick(VOID);

