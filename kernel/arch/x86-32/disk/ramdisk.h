#ifndef RAMDISK_H
#define RAMDISK_H

#include <stdint-gcc.h>
#include <stdbool.h>
#include "../multiboot.h"

#define RAMDISKVIRTUALADRESS 0xE0400000
#define SECTOR_SIZE 512

struct ramdisk
{
    uint32_t size;
    uint32_t phys_addr;
};

void create_ramdisk();
void init_ramdisk_disk(struct multiboot_info* mbinfo);
void disk_read_sector(void* buffer, uint32_t offset, uint32_t size);

extern bool using_ramdisk;
extern struct ramdisk ramdisk;


#endif