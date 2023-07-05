#pragma once

#include <efi.h>

struct vec2 {
    UINTN x;
    UINTN y;
};

struct snake_node {
    struct snake_node* prev;
    struct snake_node* next;
    struct vec2 value;
};

enum snake_direction { snakeUp, snakeDown, snakeLeft, snakeRight };

extern BOOLEAN snakeRunning;
extern BOOLEAN snakeWon;

VOID snakeInit(EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* inputEx);
VOID snakeDeinit(EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* inputEx); 
VOID snakeDoTick(VOID);

