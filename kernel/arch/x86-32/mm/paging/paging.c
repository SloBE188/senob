/*
 * Copyright (C) 2024 Nils Burkard
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "paging.h"
#include "../heap/heap.h"
#include "../../../../libk/memory.h"
#include "../../kernel.h"

extern uint32_t kernel_directory[1024];
uint32_t get_physical_addr(uint32_t virtual_addr);

struct page_directory* create_page_directory() {
    struct page_directory* dir = (struct page_directory*)kmalloc(sizeof(struct page_directory));
    memset(dir, 0, sizeof(struct page_directory));

    // Copy kernel mappings
    dir->entries[768] = *(struct page_directory_entry*)&kernel_directory[768];
    dir->entries[832] = *(struct page_directory_entry*)&kernel_directory[832];
    dir->entries[896] = *(struct page_directory_entry*)&kernel_directory[896];

    return dir;
}

uint32_t* get_current_page_directory() {
    uint32_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r" (cr3));
    return (uint32_t*)cr3;
}

uint32_t get_physical_addr(uint32_t virtual_addr) {
    uint32_t* page_directory = get_current_page_directory();
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;
    uint32_t offset = virtual_addr & 0xFFF;

    if (!(page_directory[pd_index] & 1)) return 0;
    uint32_t* page_table = (uint32_t*)(page_directory[pd_index] & ~0xFFF);

    if (!(page_table[pt_index] & 1)) return 0;
    uint32_t physical_frame = page_table[pt_index] & ~0xFFF;

    return physical_frame + offset;
}

void load_page_directory(struct page_directory* dir) {
    asm volatile("mov %0, %%cr3" :: "r"(dir));
}

void switch_to_kernel_directory() {
    load_page_directory((struct page_directory*)kernel_directory);
}

void map_page(struct page_directory* dir, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

    struct page_table* table;
    if (!(dir->entries[pd_index].present)) {
        // Page Table does not exist, create a new one
        table = (struct page_table*)kmalloc(sizeof(struct page_table));
        memset(table, 0, sizeof(struct page_table));

        dir->entries[pd_index].table_addr = ((uint32_t)table) >> 12;
        dir->entries[pd_index].present = 1;
        dir->entries[pd_index].rw = (flags & 0x2) >> 1;  // Set read/write flag
        dir->entries[pd_index].user = (flags & 0x4) >> 2; // Set user flag
    } else {
        table = (struct page_table*)((dir->entries[pd_index].table_addr) << 12);
    }

    table->entries[pt_index].frame = (physical_addr >> 12);
    table->entries[pt_index].present = 1;
    table->entries[pt_index].rw = (flags & 0x2) >> 1;  // Set read/write flag
    table->entries[pt_index].user = (flags & 0x4) >> 2; // Set user flag
}


void test_paging() {
    struct page_directory* new_pd = create_page_directory();

    // Map new page and test
    map_page(new_pd, 0x40000000, 0x100000, 0x3);
    load_page_directory(new_pd);

    uint32_t* ptr = (uint32_t*)0x40000000;
    *ptr = 42;
    //assert(*ptr == 42);

    switch_to_kernel_directory();

    uint32_t kernel_value = *(uint32_t*)0xC0000000;
}
