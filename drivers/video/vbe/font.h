#ifndef FONT_H
#define FONT_H

#include <stdint.h>

#define FONT_WIDTH  8
#define FONT_HEIGHT 8

uint8_t* font_get_glyph(char c);

#endif
