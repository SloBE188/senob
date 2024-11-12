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

#include "thread.h"
#include "../mm/heap/heap.h"
#include "../mm/paging/paging.h"
#include "../mm/PMM/pmm.h"
#include "../../../libk/memory.h"
#include "../interrupts/idt.h"
#include "../gdt/gdt.h"
#include "../fatfs/ff.h"

uint32_t thread_id = 0;

extern uint32_t kernel_directory[1024];

/*void create_kernel_thread(struct pcb* process, void(*start_function)())
{
    create_thread(process, start_function, true);
}

void create_user_thread(struct pcb* process, void(*start_function)())
{
    create_thread(process, start_function, false);
}*/

struct thread *create_user_thread(const char *path, struct pcb *process)
{
    struct thread *new_thread = (struct thread *)kmalloc(sizeof(struct thread));
    if (new_thread == NULL)
    {
        printf("Failed to allocate memory for new thread\n");
        return;
    }
    memset(new_thread, 0x00, sizeof(struct thread));
    new_thread->owner = process;
    thread_id++;
    new_thread->id = thread_id;
    new_thread->state = READY;

    // Allocate kernel stack for the new thread
    new_thread->kernel_stack = (uint32_t *)kmalloc(KERNEL_STACK_SIZE);
    if (new_thread->kernel_stack == NULL)
    {
        printf("Failed to allocate memory for kernel stack\n");
        kfree(new_thread);
        return;
    }
    memset(new_thread->kernel_stack, 0x00, KERNEL_STACK_SIZE);

    update_tss_esp0(new_thread->kernel_stack + KERNEL_STACK_SIZE / sizeof(uint32_t));

    new_thread->user_stack = (uint32_t *)USER_STACK_TOP;
    new_thread->regs.esp = USER_STACK_TOP;
    new_thread->regs.ebp = USER_STACK_TOP;
    for (int i = 0; i < USER_STACK_PAGES; i++)
    {
        mem_map_page(USER_STACK_TOP - (i * PAGE_SIZE), pmm_alloc_pageframe(), PAGE_FLAG_OWNER | PAGE_FLAG_USER | PAGE_FLAG_WRITE);
    }

    new_thread->regs.cs = USER_CODE_SEGMENT;
    new_thread->regs.ds = USER_DATA_SEGMENT;
    new_thread->regs.es = USER_DATA_SEGMENT;
    new_thread->regs.fs = USER_DATA_SEGMENT;
    new_thread->regs.gs = USER_DATA_SEGMENT;
    new_thread->regs.ss = USER_DATA_SEGMENT;
    new_thread->regs.eip = 0x00400000;
    new_thread->regs.e_flags = INTERRUPTS_ENABLED;

    FIL userprogramfile;
    FILINFO userprograminfo;

    FRESULT res = f_stat(path, &userprograminfo);
    uint32_t program_size = userprograminfo.fsize;


    //mapping the pages for the new userprogram at address 0x00400000
    uint32_t pages_needed = CEIL_DIV(program_size, PAGE_SIZE);
    for (uint32_t i = 0; i < pages_needed; i++)
    {
        uint32_t virt_addr = 0x00400000 + (i * PAGE_SIZE);
        uint32_t phys_addr = pmm_alloc_pageframe();
        mem_map_page(virt_addr, phys_addr, PAGE_FLAG_USER | PAGE_FLAG_WRITE | PAGE_FLAG_PRESENT);
    }

    //copy the userprogram from the ramdisk to the mapped addresses starting by 0x00400000
    uint8_t buffer[PAGE_SIZE];
    uint32_t bytes_read;
    for (uint32_t i = 0; i < pages_needed; i++)
    {
        uint32_t virt_addr = 0x00400000 + (i * PAGE_SIZE);
        f_read(&userprogramfile, buffer, PAGE_SIZE, &bytes_read);
        memcpy((void *)virt_addr, buffer, bytes_read); 
    }

    f_close(&userprogramfile);

    // Add thread to the specific process's thread list
    add_thread_to_process(process, new_thread);

    return new_thread;
}
