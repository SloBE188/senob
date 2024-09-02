#ifndef PAGING_H
#define PAGING_H

#include <stdint-gcc.h>

#define PAGE_SIZE 4096

#define PAGE_DIRECTORY_ENTRIES 1024
#define PAGE_TABLE_ENTRIES 1024
#define KERNEL_START_PAGE_INDEX 768
#define KERNEL_DIRECTORY_TOTAL_ENTRIES 1024


#define PAGING_ACCESS_FROM_ALL 0b00000100
#define PAGING_IS_PRESENT      0b00000001

struct paging_4gb_area
{
    uint32_t* directory_entry;
};

struct paging_4gb_area* create_paging_4gb_area(uint8_t flags);
extern void load_page_directory(uint32_t* dir);
uint32_t* get_directory_from_4gb_area(struct paging_4gb_area* area);
void switch_to_kernel_directory();
struct paging_4gb_area* create_minimal_paging_area();

#endif
