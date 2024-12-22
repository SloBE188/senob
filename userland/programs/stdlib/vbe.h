#ifndef VBE_H
#define VBE_H
#include <stdint.h>


extern void syscall_2_clear_screen(uint32_t color);
void clear_screen(uint32_t color);

#endif