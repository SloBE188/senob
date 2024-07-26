#include "vbe.h"
#include "font.h"


static struct vbe_info* globalvbeinfo;


void clear_screen(uint32_t color, struct vbe_info* vbeinfo) 
{
    draw_rectangle(0, 0, vbeinfo->framebuffer_width, vbeinfo->framebuffer_height, color, vbeinfo);
}

void draw_pixel(uint32_t x, uint32_t y, uint32_t color, struct vbe_info* vbeinfo) 
{
    if (x >= vbeinfo->framebuffer_width || y >= vbeinfo->framebuffer_height) return;
    uint32_t* framebuffer = (uint32_t*)vbeinfo->framebuffer_addr;
    framebuffer[y * (vbeinfo->framebuffer_pitch / 4) + x] = color;
}

void draw_rectangle(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color, struct vbe_info* vbeinfo) 
{
    for (uint32_t i = 0; i < height; i++) {
        for (uint32_t j = 0; j < width; j++) {
            draw_pixel(x + j, y + i, color, vbeinfo);
        }
    }
}

void draw_char(uint32_t x, uint32_t y, char c, uint32_t color, struct vbe_info* vbeinfo) {
    uint8_t* glyph = font_get_glyph(c);
    for (uint32_t i = 0; i < FONT_HEIGHT; i++) {
        for (uint32_t j = 0; j < FONT_WIDTH; j++) {
            if (glyph[i] & (1 << j)) {
                draw_pixel(x + j, y + i, color, vbeinfo);
            }
        }
    }
}

void draw_string(uint32_t x, uint32_t y, const char* str, uint32_t color, struct vbe_info* vbeinfo) 
{
    while (*str) {
        draw_char(x, y, *str, color, vbeinfo);
        x += FONT_WIDTH;
        str++;
    }
}

void init_vbe(struct vbe_info* vbeinfo)
{
    globalvbeinfo = vbeinfo;
    clear_screen(COLOR_LIGHT_GREY, vbeinfo);
}

