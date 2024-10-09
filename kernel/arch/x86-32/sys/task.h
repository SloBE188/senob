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
#include "../gdt/gdt.h"



struct task
{
    uint32_t pid;
    uint32_t esp0;
    void* start_address;
    uint32_t* page_dir;
    struct task* next;
    struct task* prev;

};

void init_task(struct task* new_task);
struct task* create_task(void*(start_function), int pid, uint32_t* page_dir, bool iskerneltaskornot);
void switch_task(struct task* next_task);
void schedule();


#endif