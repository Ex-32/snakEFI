#pragma once
#include <efi.h>

// vector representing a location on the game grid (either a snake segment or a
// fruit segment)
struct vec2 {
    UINT16 x;
    UINT16 y;
};

// doubly linked list node containing a segment of the snake
struct snakeNode {
    struct snakeNode* prev;
    struct snakeNode* next;
    struct vec2 value;
};

enum snakeDirection { snakeUp, snakeDown, snakeLeft, snakeRight };

extern BOOLEAN snakeWon;
extern UINTN snakeLen;

void snakeInit(EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* inputEx);
void snakeDeinit(EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* inputEx);
BOOLEAN snakeDoTick(void);
