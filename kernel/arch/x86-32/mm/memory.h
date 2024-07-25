#ifndef MEMORY_H
#define MEMORY_H

#include "../multiboot.h"
#include "../../../libk/stdiok.h"


void init_memory(struct multiboot_info* bootinfo);

#endif