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

struct process* create_process(const char* filename)
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

    //new pagedir for process
    new_process->page_directory = mem_alloc_page_dir();
    mem_change_page_directory(new_process->page_directory);



    uint32_t file_size = filestat.fsize;
    uint32_t pages_needed = CEIL_DIV(file_size, PAGE_SIZE);

    for (uint32_t i = 0; i < pages_needed; i++)
    {
        void* vaddr = PROGRAMM_VIRTUAL_ADDRESS_START + (i * PAGE_SIZE);
        mem_map_page(vaddr, pmm_alloc_pageframe(), PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);

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

    res = f_close(&program);
    


    struct thread* new_thread = (struct thread*) kmalloc(sizeof(struct thread));
    memset(new_thread, 0x00, sizeof(struct thread));
    new_thread->thread_id = get_thread_id();
    new_thread->owner = new_process;
    new_thread->regs.cr3 = new_process->page_directory;



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
   for (uint32_t i = 0; i < count_stack_pages; i++)
   {
        physical_frames[i] = pmm_alloc_pageframe();
        mem_map_page(end_of_stack, physical_frames[i], PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);
   }

    uint32_t segment_selector = 0x23;
    new_thread->regs.ss = segment_selector;
    new_thread->regs.eflags = 0x202;
    new_thread->regs.cs = 0x1B;
    new_thread->regs.eip = PROGRAMM_VIRTUAL_ADDRESS_START;
    new_thread->regs.ds = segment_selector;
    new_thread->regs.es = segment_selector;
    new_thread->regs.fs = segment_selector;
    new_thread->regs.gs = segment_selector;

   
   
    uint32_t user_stack_pointer = USER_STACK_TOP - 4;
    new_thread->regs.esp = user_stack_pointer;


    //kernel stack
    new_thread->kstack.ss0 = 0x10;
    uint8_t* kernel_stack = (uint8_t*)kmalloc(4096);
    new_thread->kstack.esp0 = (uint32_t)kernel_stack + 4096 - 4;
    new_thread->kstack.stack_start = (uint32_t)kernel_stack;


    return new_process;

}


void switch_to_thread(struct thread* thread)
{
    update_tss_esp0(thread->kstack.esp0);
    //switch_task(thread->regs);
}