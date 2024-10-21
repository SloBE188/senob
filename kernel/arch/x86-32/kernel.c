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

//#include "../../../drivers/video/vga/vga.h"
#include "kernel.h"
#include "gdt/gdt.h"
#include "interrupts/idt.h"
#include "interrupts/pit.h"
#include "../../../drivers/keyboard/keyboard.h"
#include "multiboot.h"
#include "../../libk/stdiok.h"
#include "../../../drivers/video/vbe/vbe.h"
#include "../../../drivers/video/vbe/wm/window.h"
#include "../../../drivers/video/vbe/font.h"
#include "mm/heap/heap.h"
#include "mm/paging/paging.h"
#include "mm/PMM/pmm.h"
#include "sys/thread.h"
#include "sys/process.h"


extern void rust_testfunction();

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

int testfunction()
{
    printf("hallo ich bin ein kernel task xd");
    return 0;
}



struct vbe_info vbeinfo;
void kernel_main(uint32_t magic_value, struct multiboot_info* multibootinfo)
{
    init_gdt();
    heap_init();
    idt_init();
    init_keyboard();
    init_pit(50);
    if (magic_value != 0x2BADB002)
    {
        printf("Invalid magic value: %x\n", magic_value);
        panic();
    }
    

    vbeinfo.framebuffer_addr = multibootinfo->framebuffer_addr;
    vbeinfo.framebuffer_pitch = multibootinfo->framebuffer_pitch;
    vbeinfo.framebuffer_width = multibootinfo->framebuffer_width;
    vbeinfo.framebuffer_height = multibootinfo->framebuffer_height;
    vbeinfo.framebuffer_bpp = multibootinfo->framebuffer_bpp;

    init_vbe(&vbeinfo);
    set_vbe_info(&vbeinfo);
    draw_rectangle(212, 300, 400, 100, COLOR_BLUE, &vbeinfo);
    draw_string(450, 300, "Herzlich willkommen bei senob ;)", COLOR_GREEN, &vbeinfo);


    uint32_t mod1 = *(uint32_t*)(multibootinfo->mods_addr + 4);
    uint32_t physicalAllocStart = (mod1 + 0xFFF) & ~0xFFF;

    init_memory(multibootinfo->mem_upper * 1024, physicalAllocStart);

    rust_testfunction();

    uint32_t* new_dir = mem_alloc_page_dir();
    mem_change_page_directory(new_dir);

    struct pcb* first_process = (struct pcb*) malloc(sizeof(struct pcb));
    init_processes(first_process);
    printf("Process initialization successful\n");

    create_kernel_thread(first_process, (void(*)())0x12345678);
    printf("Kernel thread created successfully\n");

    /*struct pcb* second_process = (struct pcb*) malloc(sizeof(struct pcb));
    add_process(second_process);
    create_user_thread(second_process, (void(*)())0x87654321);
    printf("User thread created successfully\n");*/



    while (1){}
    
}
