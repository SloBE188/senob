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
#include "../../multiboot.h"
#include "../../../../libk/stdiok.h"
#include "../PMM/pmm.h"

#define NUM_PAGE_DIRS 256
#define KERNEL_START 0xC0000000

static uint32_t pageDirs[NUM_PAGE_DIRS][1024] __attribute__((aligned(4096)));
static uint8_t pageDirsUsed[NUM_PAGE_DIRS];



// A virtual address 'la' has a three-part structure as follows:
//
// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |      Index     |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(va) --/ \--- PTX(va) --/


void invalidate(int vaddr);

int mem_num_vpages;

void init_memory(uint32_t memHigh, uint32_t physicalAllocstart)
{
    mem_num_vpages = 0;
    
    //unmap the first 4 entry in the kernel directory (DD 0x00000083 ; First Page Table Entry (0x00000000 - 0x003FFFFF)) and so on, the kernel is also mapped to this physical address so there where 2 mappings on it before
    /*uint32_t pt_addr = pmm_alloc_pageframe();
    kernel_directory[0] = pt_addr | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
    invalidate(0);
    kernel_directory[1] = 0;
    kernel_directory[2] = 0;
    kernel_directory[3] = 0;
    invalidate(1);
    invalidate(2);
    invalidate(3);*/

    //recursive mapping
    kernel_directory[1023] = ((uint32_t)kernel_directory - KERNEL_START) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
    invalidate(0xFFFFF000);

    pmm_init(physicalAllocstart, memHigh);


    memset(pageDirs, 0, PAGE_SIZE * NUM_PAGE_DIRS);
    memset(pageDirsUsed, 0, NUM_PAGE_DIRS);
}


void invalidate(int vaddr) {
    asm volatile("invlpg %0" :: "m"(vaddr));
}

void mem_change_page_directory(uint32_t* pd) {
    pd = (uint32_t*) (((uint32_t) pd) - KERNEL_START); // calc the physical address
    asm volatile(
        "mov %0, %%eax \n"
        "mov %%eax, %%cr3 \n"
        :: "m"(pd)
    );
}

uint32_t* mem_get_current_page_directory() {
    uint32_t pd;
    asm volatile("mov %%cr3, %0" : "=r"(pd));
    pd += KERNEL_START; // calc the virtual address
    return (uint32_t*) pd;
}

// addresses need to be page aligned (4096)
void mem_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {

    // if the virtual address is in the kernel memory space, change to the kernel directory
    uint32_t* prev_page_dir = 0;
    if (virt_addr >= KERNEL_START) 
    {
        prev_page_dir = mem_get_current_page_directory();

        if (prev_page_dir != kernel_directory)
            mem_change_page_directory(kernel_directory);
    }

    // extract indices from the vaddr with bit shifting
    uint32_t pd_index = virt_addr >> 22;    
    uint32_t pt_index = virt_addr >> 12 & 0x3FF;

    uint32_t* page_dir = REC_PAGEDIR;

    // page tables can only be directly accessed/modified using the recursive strat because their physical page is not mapped into memory
    uint32_t* pt = REC_PAGETABLE(pd_index);

    if (!(page_dir[pd_index] & PAGE_FLAG_PRESENT)) {
        // allocate a page table
        uint32_t pt_paddr = pmm_alloc_pageframe();

        page_dir[pd_index] = pt_paddr | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_OWNER | flags;
        invalidate(virt_addr);

        // access it directly using the recursive strategy
        for (uint32_t i = 0; i < 1024; i++) {
            pt[i] = 0;
        }
    }

    pt[pt_index] = phys_addr | PAGE_FLAG_PRESENT | flags;
    mem_num_vpages++;
    invalidate(virt_addr);

    if (prev_page_dir != 0) 
    {
        // ... then sync that across all others
        sync_page_dirs();
        // because i changed it to the kernel directory at the start of this function, i have to change back
        if (prev_page_dir != kernel_directory)
            mem_change_page_directory(prev_page_dir);
    }
}

// returns page table entry (physical address and flags)
uint32_t mem_unmap_page(uint32_t virt_addr) {

    // if the virtual address is in the kernel memory space, change to the kernel directory
    uint32_t* prev_page_dir = 0;
    if (virt_addr >= KERNEL_START) 
    {
        prev_page_dir = mem_get_current_page_directory();

        if (prev_page_dir != kernel_directory)
            mem_change_page_directory(kernel_directory);
    }

    uint32_t pd_index = virt_addr >> 22;
    uint32_t pt_index = virt_addr >> 12 & 0x3FF;

    uint32_t* page_dir = REC_PAGEDIR;   //get page directory from the recursive mapping

    uint32_t pd_entry = page_dir[pd_index];
    if (!(pd_entry & PAGE_FLAG_PRESENT))
    {
        printf("tried to free a page from a non present page table\n");
        return 0;
    }
    

    uint32_t* pt = REC_PAGETABLE(pd_index);

    uint32_t pte = pt[pt_index];
   if(!(pte & PAGE_FLAG_PRESENT))
   {
        printf("triet to free a non present page\n");
        return 0;
   }

    // virtual site gets set to 0
    pt[pt_index] = 0;

    mem_num_vpages--;

    // checks if all entrys in the page table are free, if yes, remove = true
    bool remove = true;
    for (uint32_t i = 0; i < 1024; i++) {
        if (pt[i] & PAGE_FLAG_PRESENT) 
        {
            remove = false;
            break;
        }
    }

    if (remove) 
    {
        // table is empty, destroy its physical frame if im the page owner.
        uint32_t pde = page_dir[pd_index];
        if (pde & PAGE_FLAG_OWNER) 
        {
            uint32_t pt_paddr = P_PHYS_ADDR(pde);
            pmm_free_pageframe(pt_paddr);
            page_dir[pd_index] = 0;
        }
    }

    invalidate(virt_addr);

    // free it here
    if (pte & PAGE_FLAG_OWNER) 
    {
        pmm_free_pageframe(P_PHYS_ADDR(pte));
    }

    if (prev_page_dir != 0) 
    {
        // ... then sync that across all others
        sync_page_dirs();
        // because i changed it to the kernel directory at the start of this function, i have to change back
        if (prev_page_dir != kernel_directory)
            mem_change_page_directory(prev_page_dir);
    }

    return pte;
}

uint32_t* mem_alloc_page_dir() 
{

    for (int i = 0; i < NUM_PAGE_DIRS; i++) {
        if (!pageDirsUsed[i]) {
            pageDirsUsed[i] = true;

            uint32_t* page_dir = pageDirs[i];
            memset(page_dir, 0, 0x1000);

            // first 768 entries are user page tables
            for (int i = 0; i < 768; i++) {
                page_dir[i] = 0;
            }

            // next 256 are kernel (except last)
            for (int i = 768; i < 1023; i++) {
                page_dir[i] = kernel_directory[i] & ~PAGE_FLAG_OWNER; // we don't own these though
            }

            // recursive mapping
            page_dir[1023] = (((uint32_t) page_dir) - KERNEL_START) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
            return page_dir;
        }
    }

    return 0;
}


// frees the user page tables from the give page dir
void mem_free_page_dir(uint32_t* page_dir) 
{

    uint32_t* prev_pagedir = mem_get_current_page_directory();
    mem_change_page_directory(page_dir);

    uint32_t pagedir_index = ((uint32_t)page_dir) - ((uint32_t) pageDirs);
    pagedir_index /= 4096;


    uint32_t* pd = REC_PAGEDIR;
    for (int i = 0; i < 768; i++) {
        int pde = pd[i];
        if (pde == 0)
            continue;
        
        uint32_t* ptable = REC_PAGETABLE(i);
        for (int j = 0; j < 1024; j++) {
            uint32_t pte = ptable[j];

            if (pte & PAGE_FLAG_OWNER) 
                pmm_free_pageframe(P_PHYS_ADDR(pte));
        }
        memset(ptable, 0, 4096);

        if (pde & PAGE_FLAG_OWNER) 
        {
            // created this pagetable, free it now
            pmm_free_pageframe(P_PHYS_ADDR(pde));
        }
        pd[i] = 0;
    }


    pageDirsUsed[pagedir_index] = 0;
    mem_change_page_directory(prev_pagedir);
}

// sync the kernel portions of all in-use pagedirs
void sync_page_dirs() 
{

    for (int i = 0; i < NUM_PAGE_DIRS; i++) {
        if (pageDirsUsed[i]) {
            uint32_t* page_dir = pageDirs[i];
            
            for (int i = 768; i < 1023; i++) {
                page_dir[i] = kernel_directory[i] & ~PAGE_FLAG_OWNER; // don't own these though
            }
        }
    }
}

uint32_t mem_get_phys_from_virt(uint32_t virt_addr) 
{

    uint32_t pd_index = virt_addr >> 22;
    uint32_t* page_dir = REC_PAGEDIR;
    if (!(page_dir[pd_index] & PAGE_FLAG_PRESENT)) {
        return -1;
    }

    uint32_t pt_index = virt_addr >> 12 & 0x3FF;
    uint32_t* pt = REC_PAGETABLE(pd_index);

    return P_PHYS_ADDR(pt[pt_index]);
}

bool mem_is_valid_vaddr(uint32_t vaddr) 
{
    
    uint32_t pd_index = vaddr >> 22;
    uint32_t pt_index = vaddr >> 12 & 0x3FF;

    uint32_t* page_dir = REC_PAGEDIR;

    uint32_t pd_entry = page_dir[pd_index];
    if (!(pd_entry & PAGE_FLAG_PRESENT))
        return false;
    
    uint32_t* pt = REC_PAGETABLE(pd_index);

    uint32_t pt_entry = pt[pt_index];
    return pt_entry & PAGE_FLAG_PRESENT;
}