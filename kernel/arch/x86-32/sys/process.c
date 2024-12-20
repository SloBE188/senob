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

// returns needed pages
uint32_t map_program_to_address(const char *filename, uint32_t program_address)
{
    FRESULT res;
    FILINFO filestat;

    res = f_stat(filename, &filestat);
    if (res != FR_OK)
    {
        printf("Error looking for the stats from the file: %d\n", res);
        return 0;
    }

    uint32_t file_size = filestat.fsize;
    uint32_t pages_needed = CEIL_DIV(file_size, PAGE_SIZE);

    for (uint32_t i = 0; i < pages_needed; i++)
    {
        void *vaddr = (void *)(program_address + (i * PAGE_SIZE));
        mem_map_page((uint32_t)vaddr, pmm_alloc_pageframe(), PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);
    }

    return pages_needed;
}

void copy_program_to_address(const char *filename, uint32_t pages_needed, uint32_t program_address)
{
    FIL program;
    FRESULT res;
    BYTE buffer[PAGE_SIZE];
    UINT bytes_read;

    res = f_open(&program, filename, FA_READ);
    if (res != FR_OK)
    {
        printf("Error opening the file: %d\n", res);
        return;
    }

    // copy the program into the mapped adressspace for it
    for (uint32_t i = 0; i < pages_needed; i++)
    {
        res = f_read(&program, buffer, PAGE_SIZE, &bytes_read);
        if (res != FR_OK)
        {
            printf("Error reading the file: %d\n", res);
            f_close(&program);
            return;
        }

        void *dest_address = (void *)(program_address + (i * PAGE_SIZE));
        memcpy(dest_address, buffer, bytes_read);
    }

    res = f_close(&program);
    if (res != FR_OK)
    {
        printf("Error closing the file: %d\n", res);
    }
}

struct process *create_process(const char *filename)
{
    struct process *new_process = (struct process *)kmalloc(sizeof(struct process));
    memset(new_process, 0x00, sizeof(struct process));
    printf("Allocated process structure at: %p\n", new_process);

    new_process->page_directory = mem_alloc_page_dir();
   /* uint32_t *pd = new_process->page_directory;
    for (int i = 0; i < 1024; i++)
    {
        printf("PDE[%d]: %x\n", i, pd[i]);
    }*/

    mem_change_page_directory(new_process->page_directory);

    new_process->pid = get_process_id();

    struct thread *new_thread = (struct thread *)kmalloc(sizeof(struct thread));
    memset(new_thread, 0x00, sizeof(struct thread));
    printf("Allocated thread structure at: %p\n", new_thread);
    
    
    //new_thread->regs = (struct regss *)kmalloc(sizeof(struct regss));
    //memset(new_thread->regs, 0x00, sizeof(struct regss));
    
    
    new_thread->thread_id = get_thread_id();
    new_thread->owner = new_process;

    //new_thread->regs->cr3 = new_process->page_directory;

    uint32_t pages_needed = map_program_to_address(filename, 0x00400000);
    copy_program_to_address(filename, pages_needed, 0x00400000);

    uint32_t count_stack_pages = 40;

    // untere grenze vom stack
    void *end_of_stack = (void *)(USER_STACK_TOP - (PAGE_SIZE * count_stack_pages));

    uint32_t physical_frames[count_stack_pages];
    for (uint32_t i = 0; i < count_stack_pages; i++)
    {
        void *vaddr = (void *)((uint32_t)end_of_stack + (i * PAGE_SIZE));
        physical_frames[i] = pmm_alloc_pageframe();
        mem_map_page((uint32_t)vaddr, physical_frames[i], PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);
    }

    for (uint32_t addr = (USER_STACK_TOP - (PAGE_SIZE * count_stack_pages)); addr < USER_STACK_TOP; addr += PAGE_SIZE) 
    {
    uint32_t phys = mem_get_phys_from_virt(addr);
    if (phys == -1) 
    {
        printf("address 0x%x is not mapped\n", addr);
    } else {
        printf("address 0x%x is mapped to physical address 0x%x\n", addr, phys);
    }
    }   

    mem_map_page(0xb0000000, pmm_alloc_pageframe(), PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);

    if (!mem_is_valid_vaddr(0xb0000000)) 
    {
        printf("address 0xb0000000 is not mapped\n");
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

    // kernel stack
    new_thread->kstack.ss0 = 0x10;
    uint8_t *kernel_stack = (uint8_t *)kmalloc(4096);
    new_thread->kstack.esp0 = (uint32_t)kernel_stack + 4096 - 4;
    new_thread->kstack.stack_start = (uint32_t)kernel_stack;

    new_process->thread = new_thread;

    update_tss_esp0(new_thread->kstack.esp0, 6);

    return new_process;
}

/*uint32_t kernel_stack = ((uint32_t) kmalloc(4096)) + 4096;
uint8_t* kesp = (uint8_t*) (kernel_stack);
update_tss_esp0(kernel_stack);*/

/*    uint32_t eax;     //  0
    uint32_t ebx;     //  4
    uint32_t ecx;     //  8
    uint32_t edx;     //  12

    uint32_t esp;     //  16
    uint32_t ebp;     //  20

    uint32_t edi;     //  24
    uint32_t eip;     //  28
    uint32_t eflags;  //  32

    uint32_t cs;      //  36
    uint32_t ss;      //  40
    uint32_t ds;      //  44
    uint32_t es;      //  48
    uint32_t fs;      //  52
    uint32_t gs;      //  56

    uint32_t cr3;     //  60*/


struct registers_save* save_thread_state(struct thread* thread)
{

    struct registers_save* registers = (struct registers_save*)kmalloc(sizeof(struct registers_save));

    registers->eax = thread->regs.eax;
    registers->ebx = thread->regs.ebx;
    registers->ecx = thread->regs.ecx;
    registers->edx = thread->regs.edx;

    registers->esp = thread->regs.esp;
    registers->ebp = thread->regs.ebp;

    registers->edi = thread->regs.edi;
    registers->eip = thread->regs.eip;
    registers->eflags = thread->regs.eflags;

    registers->cs = thread->regs.cs;
    registers->ss = thread->regs.ss;
    registers->ds = thread->regs.ds;
    registers->es = thread->regs.es;
    registers->fs = thread->regs.fs;
    registers->gs = thread->regs.gs;

    registers->cr3 = thread->regs.cr3;

    return registers;
}

void switch_to_thread(struct thread *thread)
{

    struct registers_save* registers = save_thread_state(thread);
    switch_task(registers);
}