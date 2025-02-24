#include "window.h"

char* strncpy(char* dest, const char* src, int count)
{
    int i = 0;
    for (i = 0; i < count-1; i++)
    {
        if (src[i] == 0x00)
            break;

        dest[i] = src[i];
    }

    dest[i] = 0x00;
    return dest;
}



void window_draw(struct window* window, struct vbe_info* vbeinfo) 
{
    draw_rectangle(window->x, window->y, window->width, window->height, window->background_color, vbeinfo);
    draw_string(window->x + 5, window->y + 5, window->title, COLOR_BLACK, vbeinfo);
}

void window_move(struct window* window, uint32_t new_x, uint32_t new_y, struct vbe_info* vbeinfo) 
{
    clear_screen(COLOR_LIGHT_GREY, vbeinfo); // Clear the screen before redrawing windows
    window->x = new_x;
    window->y = new_y;
    window_draw(window, vbeinfo);
}

struct window* window_create(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t background_color, const char* title, struct vbe_info* vbeinfo) 
{
    struct window* window = (struct window*) malloc(sizeof(struct window));
    window->x = x;
    window->y = y;
    window->width = width;
    window->height = height;
    window->background_color = background_color;
    strncpy(window->title, title, 255);
    window->buffer = (uint32_t*) malloc(width * height * sizeof(uint32_t));
    for (uint32_t i = 0; i < width * height; i++) {
        window->buffer[i] = background_color;
    }
    window_draw(window, vbeinfo);
    return window;
}

