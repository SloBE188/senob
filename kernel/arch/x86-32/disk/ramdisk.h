#ifndef RAMDISK_H
#define RAMDISK_H

#include <stdint-gcc.h>
#include <stdbool.h>

#define RAMDISKVIRTUALADRESS 0xE0000000

struct ramdisk
{
    uint32_t size;
    uint32_t phys_addr;
};

void create_ramdisk();

extern bool using_ramdisk;
extern struct ramdisk ramdisk;


#endif