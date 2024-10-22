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

#define KERNEL_CODE_SEGMENT 0x08
#define KERNEL_DATA_SEGMENT 0x10
#define USER_CODE_SEGMENT 0x18
#define USER_DATA_SEGMENT 0x20
#define INTERRUPTS_ENABLED 0x200
#define RPL_USER 3

#define USER_STACK_TOP 0xB0000000
#define USER_STACK_BOTTOM 0xAFFFE000
#define USER_STACK_PAGES 16

#define KERNEL_STACK_SIZE 0x4000
#define USER_STACK_SIZE 0x4000

uint32_t thread_id = 0;

extern uint32_t kernel_directory[1024];


void create_kernel_thread(struct pcb* process, void(*start_function)())
{
    create_thread(process, start_function, true);
}

void create_user_thread(struct pcb* process, void(*start_function)())
{
    create_thread(process, start_function, false);
}

void create_thread(struct pcb* process, void (*start_function)(), bool iskernelthreadornot)
{
    struct thread* new_thread = (struct thread*) kmalloc(sizeof(struct thread));
    if (new_thread == NULL) {
        printf("Failed to allocate memory for new thread\n");
        return;
    }
    memset(new_thread, 0x00, sizeof(struct thread));
    new_thread->owner = process;
    thread_id++;
    new_thread->id = thread_id;
    new_thread->state = READY;

    uint32_t code_selector = iskernelthreadornot ? KERNEL_CODE_SEGMENT : (USER_CODE_SEGMENT | RPL_USER);
    uint32_t data_selector = iskernelthreadornot ? KERNEL_DATA_SEGMENT : (USER_DATA_SEGMENT | RPL_USER);

    // Allocate kernel stack for the new thread
    /*new_thread->kernel_stack = (uint32_t*) kmalloc(KERNEL_STACK_SIZE);
    if (new_thread->kernel_stack == NULL) {
        printf("Failed to allocate memory for kernel stack\n");
        kfree(new_thread);
        return;
    }*/
    memset(new_thread->kernel_stack, 0x00, KERNEL_STACK_SIZE);

    if (iskernelthreadornot) {
        //new_thread->regs.esp = (uint32_t)(new_thread->kernel_stack + KERNEL_STACK_SIZE);
        new_thread->user_stack = NULL;
    } else {
        new_thread->user_stack = (uint32_t*) USER_STACK_TOP;
        new_thread->regs.esp = USER_STACK_TOP;
        for (int i = 0; i < USER_STACK_PAGES; i++) {
            mem_map_page(USER_STACK_TOP - (i * PAGE_SIZE), pmm_alloc_pageframe(), PAGE_FLAG_OWNER | PAGE_FLAG_USER | PAGE_FLAG_WRITE);
        }
    }

    new_thread->regs.cs = code_selector;
    new_thread->regs.ds = data_selector;
    new_thread->regs.es = data_selector;
    new_thread->regs.fs = data_selector;
    new_thread->regs.gs = data_selector;
    new_thread->regs.ss = data_selector;
    new_thread->regs.eip = (uint32_t)start_function;
    new_thread->regs.e_flags = INTERRUPTS_ENABLED;

    // Add thread to the specific process's thread list
    add_thread_to_process(process, new_thread);
}
