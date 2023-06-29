
#include <efi.h>

#include "snake.h"
#include "utils.h"

struct snake_node* snake_front = NULL;
struct snake_node* snake_end = NULL;
enum snake_direction direction;

VOID snake_front_append(struct vec2 value) {
    if (snake_front == NULL) {
        struct snake_node* node = bmalloc(sizeof(struct snake_node));
        
        node->prev = NULL;
        node->next = NULL;
        node->value = value;
        
        snake_front = node;
        snake_end = node;
    } else {
        struct snake_node* node = bmalloc(sizeof(struct snake_node));

        node->prev = NULL;
        node->next = snake_front;
        node->value = value;
        
        snake_front = node;
    }
}

struct vec2 snake_end_pop() {
    struct vec2 value = snake_end->value;
    
    struct snake_node* prev = snake_end->prev;
    prev->next = NULL;
    bfree(snake_end);
    snake_end = prev;

    return value;
}

struct vec2 snake_font_peak() {
    return snake_front->value;
}

VOID snake_read_input() {

}

VOID snake_update_state() {

}

VOID snake_draw_frame() {

}
