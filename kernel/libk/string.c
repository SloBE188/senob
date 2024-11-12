#include "string.h"


char* strchr(const char* str, int c) 
{
    while (*str != '\0') {
        if (*str == (char)c) {
            return (char*)str;
        }
        str++;
    }

    if (c == '\0') {
        return (char*)str;
    }

    return 0;
}

int strlen(const char* ptr)
{
    int i = 0;
    while(*ptr != 0)
    {
        i++;
        ptr += 1;
    }

    return i;
}