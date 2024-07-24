#ifndef STDIOK_H
#define STDIOK_H

#include <stdbool.h>

void putc(char c);
void puts(const char* str);
void printf(const char* fmt, ...);
int * printf_number(int*, int, bool, int);


#define PRINTF_STATE_START 0
#define PRINTF_STATE_LENGTH 1
#define PRINTF_STATE_SHORT 2
#define PRINTF_STATE_LONG 3
#define PRINTF_STATE_SPEC 4

#define PRINTF_LENGTH_START 0
#define PRINTF_LENGTH_SHORT_SHORT 1
#define PRINTF_LENGTH_SHORT 2
#define PRINTF_LENGTH_LONG 3
#define PRINTF_LENGTH_LONG_LONG 4



#endif