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

#include "../../../drivers/video/vga/vga.h"
#include "gdt/gdt.h"
#include "interrupts/idt.h"
#include "interrupts/pit.h"
#include "../../../drivers/keyboard/keyboard.h"
#include "multiboot.h"
#include "../../libk/stdiok.h"
#include "mm/memory.h"



#define magic 0x1BADB002

void trigger_breakpoint() {
    asm volatile("int $3");
}

void trigger_general_protection_fault() {
    asm volatile (
        "mov $0x10, %ax\n\t"
        "ltr %ax"
    );
}

void trigger_division_by_zero() {
    asm volatile (
        "movl $1, %eax\n\t"
        "movl $0, %ebx\n\t"
        "divl %ebx"
    );
}


void trigger_bound_range_exceeded() {
    int array[2] = {1, 2};
    asm volatile (
        "movl %0, %%eax\n\t"
        "movl $2, %%ecx\n\t"
        "bound %%ecx, (%%eax)"
        :
        : "r"(array)
    );
}

void trigger_overflow() {
    asm volatile (
        "add $0x7FFFFFFF, %eax\n\t"
        "add $1, %eax\n\t"
        "into"
    );
}

void panic()
{
    print("Kernel Panic");
    while (1){}
}



void kernel_main(uint32_t magic_value, struct multiboot_info* multibootinfo)
{
    reset();
    init_gdt();
    print("--GDT loaded\n");
    print("--TSS loaded\n");
    idt_init();
    print("--IDT loaded\n");
    init_keyboard();
    print("--KEYBOARD DRIVER LOADED\n");
    //init_pit(50);
    //trigger_breakpoint();
    //trigger_division_by_zero();
    /*if (magic_value != magic)
    {
        printf("magic value isnt right");
    }*/
    
    print("Herzlich willkommen bei senob!\n");

    init_memory(multibootinfo);

    //uint64_t addr = multibootinfo->framebuffer_addr;
    printf("%d\n", multibootinfo->framebuffer_addr);
    while (1){}
    
}
