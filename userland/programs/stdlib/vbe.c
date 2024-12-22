#include "vbe.h"

void clear_screen(uint32_t color)
{
    syscall_2_clear_screen(color);
}