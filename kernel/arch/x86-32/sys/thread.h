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

#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include "../mm/paging/paging.h"
#include "../mm/PMM/pmm.h"
#include <stdbool.h>
#include "../gdt/gdt.h"


#include <stdint.h>
#include <stdbool.h>

#define PAGE_SIZE 4096
#define PAGE_FLAG_PRESENT (1 << 0)
#define PAGE_FLAG_WRITE   (1 << 1)
#define PAGE_FLAG_USER    (1 << 2)
#define PAGE_FLAG_4MB     (1 << 7)
#define PAGE_FLAG_OWNER   (1 << 9) // means im in charge of the physical page

enum state{
    RUNNING,
    READY,
    BLOCKED,
    TERMINATED
};


struct registers
{
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;       // stack pointer
    uint32_t eip;       //instruction pointer
    uint32_t e_flags;
    uint32_t cs;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    uint32_t ss;
};

struct thread
{
    uint32_t id;
    uint32_t* kernel_stack;
    uint32_t* user_stack;
    struct registers regs;
    struct thread* next;        //next thread in the same process
    struct thread* prev;        //prev thread in the same process
    struct pcb* owner;
    enum state state;

};


void mem_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void create_thread(struct pcb* process, void (*start_function)(), bool iskernelthreadornot);
void create_kernel_thread(struct pcb* process, void(*start_function)());
void create_user_thread(struct pcb* process, void(*start_function)());


#endif