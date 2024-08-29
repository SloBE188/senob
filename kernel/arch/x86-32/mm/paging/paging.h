#ifndef PAGING_H
#define PAGING_H

#include <stdint-gcc.h>

#define PAGE_SIZE 4096

 struct page_table_entry {
    uint32_t present    : 1;   // P
    uint32_t rw         : 1;   // R/W
    uint32_t user       : 1;   // U/S
    uint32_t pwt        : 1;   // PWT
    uint32_t pcd        : 1;   // PCD
    uint32_t accessed   : 1;   // A
    uint32_t dirty      : 1;   // D (only page table entry)
    uint32_t pat        : 1;   // PAT
    uint32_t global     : 1;   // G
    uint32_t available  : 3;   // AVL
    uint32_t frame      : 20;  // frame-adress (for 4KB pages)
};


struct page_table
{
    struct page_table_entry entries[1024];
};

 struct page_directory_entry {
    uint32_t present    : 1;   // P
    uint32_t rw         : 1;   // R/W
    uint32_t user       : 1;   // U/S
    uint32_t pwt        : 1;   // PWT
    uint32_t pcd        : 1;   // PCD
    uint32_t accessed   : 1;   // A
    uint32_t ignored    : 1;   // Ignored
    uint32_t size       : 1;   // PS (Page Size, 0 = 4KB, 1 = 4MB)
    uint32_t global     : 1;   // G (only relevant, if PS = 0)
    uint32_t available  : 3;   // AVL
    uint32_t table_addr : 20;  // Page table adress (for 4KB pages)
};

struct page_directory
{
    struct page_directory_entry entries[1024];
};

struct page_directory* create_page_directory();
void load_page_directory(struct page_directory* dir);
uint32_t get_physical_addr(uint32_t virtual_addr);

#endif