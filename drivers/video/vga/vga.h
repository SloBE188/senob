#ifndef VGA_H
#define VGA_H
#include <stdint-gcc.h>


#define COLOR8_BLACK 0
#define COLOR8_LIGHT_GREY 7

#define width_screen 80
#define height_screen 25

void print(const char* s);
void scrollUp();
void newLine();
void reset();


#endif