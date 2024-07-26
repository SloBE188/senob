#ifndef VBE_H
#define VBE_H

#include <stdint-gcc.h>


//Farben
#define COLOR_BLACK         0x000000
#define COLOR_WHITE         0xFFFFFF
#define COLOR_RED           0xFF0000
#define COLOR_GREEN         0x00FF00
#define COLOR_BLUE          0x0000FF
#define COLOR_LIGHT_GREY    0xD3D3D3

struct vbe_info{
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
};


void init_vbe(struct vbe_info* vbeinfo);
void draw_pixel(uint32_t x, uint32_t y, uint32_t color, struct vbe_info* vbeinfo);
void draw_rectangle(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color, struct vbe_info* vbeinfo);
void draw_string(uint32_t x, uint32_t y, const char* str, uint32_t color, struct vbe_info* vbeinfo);


#endif