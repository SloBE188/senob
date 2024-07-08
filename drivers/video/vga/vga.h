#ifndef VGA_H
#define VGA_H
#include <stdint.h>


#define COLOR8_BLACK 0
#define COLOR8_LIGHT_GREY 7

#define width 80
#define height 25

void print(const char* s);
void scrollUp();
void newLine();
void reset();


#endif