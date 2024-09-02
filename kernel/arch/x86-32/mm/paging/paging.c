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
void get_physical_address_from_kernel_directory();

struct paging_4gb_area* create_minimal_paging_area() {
    uint32_t* directory = kzalloc(sizeof(uint32_t) * PAGE_DIRECTORY_ENTRIES);
    if (directory == NULL) {
        return NULL;
    }

    directory[768] = kernel_directory[768];

    struct paging_4gb_area* new_area = kzalloc(sizeof(struct paging_4gb_area));
    if (new_area == NULL) {
        return NULL;
    }

    new_area->directory_entry = directory;
    return new_area;
}

struct paging_4gb_area* create_paging_4gb_area(uint8_t flags)
{
    uint32_t* directory = kzalloc(sizeof(uint32_t) * PAGE_DIRECTORY_ENTRIES);
    if (directory == NULL)
    {
        return NULL;
    }
    
    int offset = 0;

    /*for (int i = KERNEL_START_PAGE_INDEX; i < KERNEL_DIRECTORY_TOTAL_ENTRIES; i++)
    {
        directory[i] = kernel_directory[i];
    }*/
   

    directory[768] = kernel_directory[768];
    directory[832] = kernel_directory[832];
    directory[896] = kernel_directory[896];

    for (int i = 0; i < PAGE_DIRECTORY_ENTRIES; i++)
    {
        if (i == 768 || i == 832 || i == 896)
        {
            continue;
        }
        
        uint32_t* page_table_entry = kzalloc(sizeof(uint32_t) * PAGE_TABLE_ENTRIES);
        if (page_table_entry == NULL)
        {
            return NULL;
        }
        
        for (int j = 0; j < PAGE_TABLE_ENTRIES; j++)
        {
            page_table_entry[j] = (offset + (j * PAGE_SIZE)) | flags;
        }
        offset += (PAGE_TABLE_ENTRIES * PAGE_SIZE);
        directory[i] = (uint32_t) page_table_entry | flags | 0x3;
        
    }

    struct paging_4gb_area* new_area = kzalloc(sizeof(struct paging_4gb_area));
    if (new_area == NULL)
    {
        return NULL;
    }
    
    new_area->directory_entry = directory;
    return new_area;

}

uint32_t* get_directory_from_4gb_area(struct paging_4gb_area* area)
{
    uint32_t physical_address = (uint32_t)area->directory_entry - 0xC0000000;
    return (uint32_t*)physical_address;
}

uint32_t* get_current_page_directory() {
    uint32_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r" (cr3));
    return (uint32_t*)cr3;
}

void switch_to_kernel_directory()
{
    uint32_t physical_address = (uint32_t)kernel_directory - 0xC0000000;
    load_page_directory(physical_address);
}

void get_physical_address_from_kernel_directory()
{
    uint32_t physical_address = (uint32_t)kernel_directory - 0xC0000000;
    return physical_address;
}




