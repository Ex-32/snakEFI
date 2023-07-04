
#include <efi.h>

#include "snake.h"
#include "gop.h"
#include "utils.h"

static struct snake_node* snake_front = NULL;
static struct snake_node* snake_end = NULL;
static enum snake_direction direction;

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

struct vec2 snake_end_pop(VOID) {
    struct vec2 value = snake_end->value;
    
    struct snake_node* prev = snake_end->prev;
    prev->next = NULL;
    bfree(snake_end);
    snake_end = prev;

    return value;
}

struct vec2 snake_font_peak(VOID) {
    return snake_front->value;
}

VOID snake_read_input(VOID) {

}

VOID snake_update_state(VOID) {

}

VOID snake_draw_frame(VOID) {
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = getGop();

}
