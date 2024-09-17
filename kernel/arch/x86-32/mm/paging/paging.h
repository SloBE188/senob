#ifndef PAGING_H
#define PAGING_H

#include <stdint-gcc.h>

extern uint32_t kernel_directory[1024];

#define PAGE_SIZE 4096
#define PAGE_FLAG_PRESENT (1 << 0)
#define PAGE_FLAG_WRITE   (1 << 1)
#define PAGE_FLAG_USER    (1 << 2)
#define PAGE_FLAG_4MB     (1 << 7)
#define PAGE_FLAG_OWNER   (1 << 9) // means im in charge of the physical page

#define P_PHYS_ADDR(x) ((x) & ~0xFFF)   // gets the physical address from a page directory or page table entry
//virtual addresses
#define REC_PAGEDIR ((uint32_t*) 0xFFFFF000)    // refers to the last entry in the page directory (recursive mapping)
#define REC_PAGETABLE(i) ((uint32_t*) (0xFFC00000 + ((i) << 12)))   //gets a spezific page table in a given page directory (recursive mapping), the param is the index from the page table

void init_memory(uint32_t memHigh, uint32_t physicalAllocstart);
void mem_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void mem_change_page_directory(uint32_t* pd);
uint32_t* mem_get_current_page_directory();
uint32_t mem_unmap_page(uint32_t virt_addr);
uint32_t* mem_alloc_page_dir();
void mem_free_page_dir(uint32_t* page_dir);
void sync_page_dirs();

#endif
