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

enum snake_direction { snake_up, snake_down, snake_left, snake_right };

VOID snake_front_append(struct vec2 value);
struct vec2 snake_end_pop();
struct vec2 snake_front_peak();


