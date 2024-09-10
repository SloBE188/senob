#ifndef PAGING_H
#define PAGING_H

#include <stdint-gcc.h>

#define PAGE_SIZE 4096
#define PAGE_FLAG_PRESENT (1 << 0)
#define PAGE_FLAG_WRITE   (1 << 1)
#define PAGE_FLAG_USER    (1 << 2)
#define PAGE_FLAG_4MB     (1 << 7)
#define PAGE_FLAG_OWNER   (1 << 9) // means we are in charge of the physical page

#define P_PHYS_ADDR(x) ((x) & ~0xFFF)
//virtual addresses
#define REC_PAGEDIR ((uint32_t*) 0xFFFFF000)
#define REC_PAGETABLE(i) ((uint32_t*) (0xFFC00000 + ((i) << 12)))

void init_memory(uint32_t memHigh, uint32_t physicalAllocstart);

#endif
