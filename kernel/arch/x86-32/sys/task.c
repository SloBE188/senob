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

#include "task.h"
#include "../mm/heap/heap.h"
#include "../mm/paging/paging.h"
#include "../mm/PMM/pmm.h"
#include "../../../libk/memory.h"
#include "../interrupts/idt.h"
#include "../gdt/gdt.h"


uint32_t number_tasks;
uint32_t current_task_index;
struct task* current_task;
struct task tasks[MAX_TASKS];

extern void task_switch();



void init_tasks() 
{
    memset(tasks, 0x00, sizeof(struct task) * MAX_TASKS);
    for (int i = 0; i < MAX_TASKS; i++)
    {
        tasks[i].state = TASK_FREE;
    }
    tasks[0].state = TASK_RUNNING;
    tasks[0].page_directory = mem_get_current_page_directory();
    tasks[0].id = 100;
    current_task = &tasks[0];
    current_task_index = 0;
    number_tasks = 0;
    

}

int find_avaiable_task_slot()
{
    for (int i = 0; i < MAX_TASKS; i++)
    {
        if (tasks[i].state == TASK_FREE)
            return i;
    }
    return -1;
}



struct task* create_task(uint32_t index, void* func, bool iskerneltaskornot, uint32_t* page_directory)
{
    if (index >= MAX_TASKS) {
        return NULL;
    }
    
    memset(&tasks[index], 0x00, sizeof(struct task));
    tasks[index].id = index;
    tasks[index].page_directory = page_directory;
    tasks[index].state = TASK_READY;
    tasks[index].privilege_level = iskerneltaskornot;

    // Allocate kernel stack
    tasks[index].kernel_stack = (uint32_t) kmalloc(KERNEL_STACK_SIZE);

    if (!tasks[index].kernel_stack) {
        printf("failed to allocate memory for kernel stack");
        return NULL;
    }

    // Set the kernel stack for the task in the TSS
    tss.esp0 = tasks[index].kernel_stack + KERNEL_STACK_SIZE;
    tss.ss0 = 0x10; // Kernel Data Segment Selector


        // Kernel task initialization
        tasks[index].registers.ss = 0x10;        // Kernel Data Segment Selector
        tasks[index].registers.cs = 0x08;        // Kernel Code Segment Selector
        tasks[index].registers.esp = tasks[index].kernel_stack + KERNEL_STACK_SIZE;
        tasks[index].registers.ebp = tasks[index].registers.esp;
        tasks[index].registers.ip = (uint32_t)func;
        tasks[index].registers.flags = 0x200;    // Interrupt Flag (IF) enabled

        return &tasks[index];
}


void create_kernel_task(void* func)
{
    int slot = find_avaiable_task_slot();
    if (slot == -1)
    {
        printf("No slots are available anymore\n");
        return;
    }
    
    struct task* new_task = create_task(slot, func, true, kernel_directory);
    if (!new_task)
    {
        printf("Failed to create a kernel task\n");
        return;
    }

    number_tasks++;
    current_task = new_task;
    return new_task;
}


void switch_to_user_mode() {
			    // Set up a stack structure for switching to user mode.
			    asm volatile("  \
			        cli; \
			        mov $0x23, %ax; \
			        mov %ax, %ds; \
			        mov %ax, %es; \
			        mov %ax, %fs; \
			        mov %ax, %gs; \
			                \ 
			        mov %esp, %eax; \
			        pushl $0x23; \
			        pushl %eax; \
			        pushf; \
			        mov $0x200, %eax; \
			        push %eax; \
			        pushl $0x1B; \
			        push $1f; \
			        iret; \    1: \
			    "); 
			}

void switch_task()
{
    uint32_t next_task_index = (current_task_index + 1) % MAX_TASKS;  // Einfacher Round-Robin-Algorithmus
    struct task* next_task = &tasks[next_task_index];

    // Prüfe, ob der nächste Task bereit ist
    if (next_task->state == TASK_READY || next_task->state == TASK_RUNNING)
    {
        current_task->state = TASK_READY;  // Setze den aktuellen Task auf READY
        next_task->state = TASK_RUNNING;   // Setze den nächsten Task auf RUNNING
        current_task = next_task;
        current_task_index = next_task_index;

        // Führe den Kontextwechsel durch
        context_switch(next_task);
    }
}

