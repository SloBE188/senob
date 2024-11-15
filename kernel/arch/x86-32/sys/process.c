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

#include "process.h"
#include "../../../libk/stdiok.h"
#include "../kernel.h"
#include "../../../libk/memory.h"
#include "../../../libk/string.h"
#include "../gdt/gdt.h"
#include "../mm/paging/paging.h"
#include "../mm/heap/heap.h"
#include "../mm/PMM/pmm.h"
#include "../fatfs/ff.h"

uint32_t current_processid = 0;
uint32_t current_threadid = 0;

uint32_t get_process_id()
{
    return current_processid++;
}

uint32_t get_thread_id()
{
    return current_threadid++;
}

struct process* create_process(char* filename)
{
    struct process* new_process = (struct process*) kmalloc(sizeof(struct process));
    memset(new_process, 0x00, sizeof(struct process));
    new_process->pid = get_process_id();

    FRESULT res;
    FILINFO filestat;

    res = f_stat(filename, &filestat);
    if (res != FR_OK)
    {
        printf("Error looking for the stats from the file\n");
        return;
    }

    uint32_t file_size = filestat.fsize;
    uint32_t pages_needed = CEIL_DIV(file_size, PAGE_SIZE);

    for (uint32_t i = 0; i < pages_needed; i++)
    {
        void* vaddr = PROGRAMM_VIRTUAL_ADDRESS_START + (i * PAGE_SIZE);
        mem_map_page(vaddr, pmm_alloc_pageframe(), PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);

    }

    FIL program;
    uint32_t buffer[PAGE_SIZE];
    uint32_t bytes_read;

    //copy the programm from the ramdisk into the mapped address space
    for (uint32_t i = 0; i < pages_needed; i++)
    {
        f_read(&program, buffer, PAGE_SIZE, &bytes_read);
        memcpy(PROGRAMM_VIRTUAL_ADDRESS_START, buffer, bytes_read);
    }
    
    //new pagedir for process
    new_process->page_directory = mem_alloc_page_dir();


    struct thread* new_thread = (struct thread*) kmalloc(sizeof(struct thread));
    memset(new_thread, 0x00, sizeof(struct thread));
    new_thread->thread_id = get_thread_id();
    new_thread->owner = new_process;
    new_thread->regs.cr3 = new_process->page_directory;

    mem_change_page_directory(new_process->page_directory);


    /*const uint32_t stack_page_count = 50;
    char* v_address_stack_page = (char *) (USER_STACK - PAGESIZE_4K * stack_page_count);
    uint32_t stack_frames[stack_page_count];
    for (uint32_t i = 0; i < stack_page_count; ++i)
    {
        stack_frames[i] = vmm_acquire_page_frame_4k();
    }
    void* stack_v_mem = vmm_map_memory(process, (uint32_t)v_address_stack_page, stack_frames, stack_page_count, TRUE);
    if (NULL == stack_v_mem)
    {
        for (uint32_t i = 0; i < stack_page_count; ++i)
        {
            vmm_release_page_frame_4k(stack_frames[i]);
        }
    }   
    */

   uint32_t count_stack_pages = 40;

   void* end_of_stack = USER_STACK_TOP - (PAGE_SIZE * count_stack_pages);
   uint32_t physical_frames[count_stack_pages];







    /*uint32_t selector = 0x23;

    thread->regs.ss = selector;
    thread->regs.eflags = 0x0;
    thread->regs.cs = 0x1B;
    thread->regs.eip = (uint32_t)func;
    thread->regs.ds = selector;
    thread->regs.es = selector;
    thread->regs.fs = selector;
    thread->regs.gs = selector; //48 | 3;

    uint32_t stack_pointer = USER_STACK - 4;

    thread->regs.esp = stack_pointer;



    thread->kstack.ss0 = 0x10;
    uint8_t* stack = (uint8_t*)kmalloc(KERN_STACK_SIZE);
    thread->kstack.esp0 = (uint32_t)(stack + KERN_STACK_SIZE - 4);
    thread->kstack.stack_start = (uint32_t)stack;*/

}