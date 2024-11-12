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
#include "process.h"


#include <stdint-gcc.h>
#include <stdbool.h>

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
    TERMINATED,
    IDLET
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


struct thread *create_user_thread(const char *path, struct pcb *process);
//void create_kernel_thread(struct pcb* process, void(*start_function)());
//void create_user_thread(struct pcb* process, void(*start_function)());
void userland(uint32_t* esp, uint32_t* eip);

#endif