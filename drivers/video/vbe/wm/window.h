#ifndef WINDOW_H
#define WINDOW_H

#include <stdint-gcc.h>

struct window
{
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t background_color;
    char title[256];
    uint32_t* buffer;
};



struct window* window_create(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t background_color, const char* title, struct vbe_info* vbeinfo);
void window_draw(struct window* window, struct vbe_info* vbeinfo);
void window_move(struct window* window, uint32_t new_x, uint32_t new_y, struct vbe_info* vbeinfo);

#endif