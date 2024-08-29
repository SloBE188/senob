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

struct page_directory* create_page_directory()
{
    struct page_directory* dir = (struct page_directory*)kmalloc(sizeof(struct page_directory));
    memset(dir, 0, sizeof(struct page_directory));

    //kernel mappings
    dir->entries[768] = *(struct page_directory_entry*)&kernel_directory[768];
    dir->entries[832] = *(struct page_directory_entry*)&kernel_directory[832];
    dir->entries[896] = *(struct page_directory_entry*)&kernel_directory[896];


    for (int i = 0; i < 1024; i++)
    {
        if (i == 768 || i == 832 || i ==896)
        {
            continue;
        }

        struct page_table* table = (struct page_table*)kmalloc(sizeof(struct page_table));
        memset(table, 0, sizeof(struct page_table));

        // fill the new page table in the page directory
        dir->entries[i].table_addr = ((uint32_t)table) >> 12;  // physical addr from the new page table
        dir->entries[i].present = 1;       // present true
        dir->entries[i].rw = 1;            // read & write
        dir->entries[i].user = 1;          // user access yes

        /*for (int j = 0; j < 1024; j++)
        {
            void* virtual_addr = kmalloc(PAGE_SIZE);
            if (virtual_addr == NULL)
            {
                panic();
            }

            uint32_t physical_addr = get_physical_addr(virtual_addr);
            table->entries[j].frame = (uint32_t) physical_addr >> 12; // need the physical address
            table->entries[j].present = 1;                 // present true
            table->entries[j].rw = 1;                      // read & write
            table->entries[j].user = 1;                    // user access yes 
            
        }*/
        
    }
    

    return dir;
}


uint32_t* get_current_page_directory() {
    uint32_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r" (cr3));  // reads the Cr3 reg (the cr3 register has the address from the page directory)
    return (uint32_t*)cr3;
}


uint32_t get_physical_addr(uint32_t virtual_addr) {
    // get current page directory
    uint32_t* page_directory = (uint32_t*)get_current_page_directory();

    // extract the different parts of the virtual address
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF;  // high 10 bits
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;  // high 10 bits
    uint32_t offset = virtual_addr & 0xFFF;            // low 12 bits

    // get the address of the page table from the page directory
    uint32_t* page_table = (uint32_t*)(page_directory[pd_index] & ~0xFFF); //mask flags
    if (!(page_directory[pd_index] & 1)) 
    {
        return 0;
    }

    // get the physical frame address from the page table
    uint32_t physical_frame = page_table[pt_index] & ~0xFFF; // maks flags
    if (physical_frame == 0) 
    {
        return 0;
    }
    return physical_frame + offset;
}

void load_page_directory(struct page_directory* dir) {
    asm volatile("mov %0, %%cr3" :: "r"(dir));
}