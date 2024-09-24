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

#ifndef TASK_H
#define TASK_H

#include <stdint-gcc.h>
#include "../mm/paging/paging.h"
#include "../mm/PMM/pmm.h"
#include <stdbool.h>


#define KERNEL_STACK_SIZE 0x4000    //16KB
#define USER_STACK_SIZE 0x4000      //16KB

enum task_state
{
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_TERMINATED,
    TASK_SLEEPING,
    TASK_WAITING
};


struct task
{
    uint32_t id;
    uint32_t* page_directory;       //tasks page directory, kernel tasks use the kernel_directory

    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;

    uint32_t kernel_stack;          //esp0 from tss

    uint32_t privilege_level;    // kernel task or user task
    uint32_t state;

    char name[32];

    //struct task* next;
    //struct task *prev;


};


struct task* create_task(uint32_t index, void* func, bool iskerneltaskornot, uint32_t* page_directory);


#endif