#ifndef PAGING_H
#define PAGING_H

#include <stdint-gcc.h>

#define PAGE_SIZE 4096

#define PAGE_DIRECTORY_ENTRIES 1024
#define PAGE_TABLE_ENTRIES 1024
#define KERNEL_START_PAGE_INDEX 768


#define PAGING_CACHE_DISABLED  0b00010000
#define PAGING_WRITE_THROUGH   0b00001000
#define PAGING_ACCESS_FROM_ALL 0b00000100
#define PAGING_IS_WRITEABLE    0b00000010
#define PAGING_IS_PRESENT      0b00000001


struct page_directory
{
    uint32_t entries[1024];
};

struct page_table
{
    uint32_t entries[1024];
};


extern void load_page_directory(uint32_t* dir);
void switch_to_kernel_directory();
uint32_t* get_current_page_directory();
struct page_directory* create_page_directory(int process_id);
void load_process_directory(int process_id);
uint32_t virtual_to_physical(uint32_t virt_addr);
void map_process_page(int process_id, uint32_t virt_addr, uint32_t phys_addr);
void map_process_space(struct page_directory* dir, uint32_t virt_addr, uint32_t phys_addr);
#endif
