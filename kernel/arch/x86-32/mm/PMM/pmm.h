#ifndef PMM_H
#define PMM_H

#include <stdint-gcc.h>


#define CEIL_DIV(a, b) (((a) + (b) - 1) / (b));

void pmm_init(uint32_t mem_low, uint32_t mem_high);
uint32_t pmm_alloc_pageframe();
void pmm_free_pageframe(uint32_t addr);
uint32_t pmm_get_total_allocated_pages();

#endif