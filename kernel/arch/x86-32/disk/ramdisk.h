#ifndef RAMDISK_H
#define RAMDISK_H

#include <stdint-gcc.h>

#define RAMDISKVIRTUALADRESS 0xE0000000

struct ramdisk
{
    uint32_t size;
    uint32_t phys_addr;
};

void create_ramdisk();

extern bool using_ramdisk;


#endif