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


uint32_t number_tasks = 0;
uint32_t current_task_index = 0;
struct task* current_task;
struct task tasks[32];

extern void task_switch(struct task* old, struct task* new);

void task1() {
    while (1) {
        printf("Task 1 is running\n");
        for (volatile int i = 0; i < 5; i++);
    }
}

void task2() {
    while (1) {
        printf("Task 2 is running\n");
        for (volatile int i = 0; i < 5; i++);
    }
}


void init_tasks() {
    create_task(0, task1, true, mem_alloc_page_dir()); // test task 1
    create_task(1, task2, true, mem_alloc_page_dir()); // test task 2

    current_task = &tasks[0]; 
    current_task_index = 0;
    number_tasks = 2;
}


struct task* create_task(uint32_t index, void* func, bool iskerneltaskornot, uint32_t* page_directory)
{
    memset(&tasks[index], 0x00, sizeof(struct task));

    tasks[index].id = 1;
    tasks[index].page_directory = page_directory;

    tasks[index].kernel_stack = (uint32_t) kmalloc(KERNEL_STACK_SIZE);

    //set the kernel stack for the task in the tss
    tss.esp0 = tasks[index].kernel_stack + KERNEL_STACK_SIZE;

    //user stack
    uint32_t user_stack_base = (uint32_t) kmalloc(USER_STACK_SIZE);
    uint32_t user_stack_top = user_stack_base + USER_STACK_SIZE;

    tasks[index].esp = user_stack_top;
    tasks[index].ebp = user_stack_top;
    tasks[index].eip = (uint32_t)func;
    tasks[index].state = TASK_READY;
    
    tasks[index].privilege_level = iskerneltaskornot;


    return &tasks[index];

}

void schedule() {

    struct task* old_task = current_task;
    struct task* new_task = &tasks[1];
    current_task = new_task;
    current_task_index = 1;
    task_switch(old_task, new_task);
}


