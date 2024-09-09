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
#include "mm/memory.h"
#include "../../../drivers/video/vbe/vbe.h"
#include "../../../drivers/video/vbe/wm/window.h"
#include "../../../drivers/video/vbe/font.h"
#include "mm/heap/heap.h"
#include "mm/paging/paging.h"



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

void check_page_directory(struct page_directory* dir) {
    for (int i = 768; i < 1024; i++) {
        printf("Kernel Mapping Entry %d: 0x%x\n", i, dir->entries[i]);
    }

    // Optional: Überprüfe auch Heap- und User-Space-Einträge
    for (int i = 832; i <= 855; i++) {
        printf("Heap Mapping Entry %d: 0x%x\n", i, dir->entries[i]);
    }
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

    int agrad = kmalloc(26214400);
    agrad = 213456;
    printf("agrad: %d", agrad);
    kfree(agrad);

    
    int process_id = 1;
    struct page_directory* process_dir = create_page_directory(process_id);

    // mapping from a process(virtual 0x00400000 to physical 0x00002000)
    map_process_space(process_dir, 0x00400000, 0x00002000);

    load_process_directory(process_id);
    //load_page_directory((uint32_t*)((uint32_t)directory - 0xC0000000));

    //switch_to_kernel_directory();


    //struct window* window1 = window_create(50, 50, 200, 150, COLOR_WHITE, "Window 1", &vbeinfo);
    //struct window* window2 = window_create(300, 100, 200, 150, COLOR_BLUE, "Window 2", &vbeinfo);
    
    //init_memory(multibootinfo);
    while (1){}
    
}
