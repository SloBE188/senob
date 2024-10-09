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
//#include "../gdt/gdt.h"

#define KERNEL_STACK_SIZE 0x4000
#define KERNEL_DATA_SEGMENT 0x10
#define KERNEL_CODE_SEGMENT 0x08
#define USER_DATA_SEGMENT 0x20
#define USER_CODE_SEGMENT 0x18
#define RPL_USER 3
#define USER_STACK_SIZE 0x4000
#define USER_STACK_TOP 0xB0000000
#define USER_STACK_PAGES 16

struct task *task_head;
struct task *task_tail;
struct task *task_current;

extern void tss_flush();

uint8_t kernel_stacks[10][KERNEL_STACK_SIZE];
struct task tasks[10];

void init_task(struct task *new_task)
{
    memset(task_head, 0x00, sizeof(struct task));
    memset(task_tail, 0x00, sizeof(struct task));
    memset(task_current, 0x00, sizeof(struct task));
    task_head = new_task;
    task_tail = new_task;
    task_current = new_task;
}

struct task* create_task(void*(start_function), int pid, uint32_t* page_dir, bool iskerneltaskornot)
{
    struct task *new_task = &tasks[pid];        //(struct task *)kmalloc(sizeof(struct task));
    new_task->pid = pid;
    new_task->esp0 = (uint32_t)&kernel_stacks[pid][KERNEL_STACK_SIZE];            //(uint32_t *)kmalloc(KERNEL_STACK_SIZE);
    new_task->page_dir = page_dir;
    new_task->start_address = start_function;

    /*if (!iskerneltaskornot)
    {
        new_task->tss.esp = USER_STACK_TOP;
        new_task->tss.ss = USER_DATA_SEGMENT;
        for (int i = 0; i < USER_STACK_PAGES; i++)
        {
            mem_map_page(USER_STACK_TOP - USER_STACK_PAGES * 0x1000 + i * 0x1000, pmm_alloc_pageframe(), PAGE_FLAG_OWNER | PAGE_FLAG_USER | PAGE_FLAG_WRITE);
        }
    }*/

    uint32_t code_selector = iskerneltaskornot ? KERNEL_CODE_SEGMENT : (USER_CODE_SEGMENT | RPL_USER);
    uint32_t data_selector = iskerneltaskornot ? KERNEL_DATA_SEGMENT : (USER_DATA_SEGMENT | RPL_USER);

    tss.cs = code_selector;
    tss.ds = data_selector;
    tss.es = data_selector;
    tss.fs = data_selector;
    tss.gs = data_selector;

    tss.ss0 = KERNEL_DATA_SEGMENT;
    tss.esp0 = new_task->esp0;              //new_task->esp0 + KERNEL_STACK_SIZE;

    tss.eflags = 0x200;        //interrupts enabled
    tss.eip = (uint32_t)new_task->start_address;

    task_tail->next = new_task;
    task_head->prev = task_tail;
    task_current = new_task;

    return new_task;
    
}

void switch_task(struct task* next_task)
{
    /*if(next_task == task_current)
        return 0;
    
    task_current = next_task;*/

    tss_flush();
}



void schedule() {
    if (task_current && task_current->next) {
        switch_task(task_current->next); // switch to next task
    } else {
        //if no next tasks, switch to the first one
        switch_task(task_head);
    }
}

