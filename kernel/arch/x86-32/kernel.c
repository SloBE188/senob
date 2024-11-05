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
#include "sys/process.h"
#include "sys/thread.h"
#include "../../libk/memory.h"


extern void rust_testfunction();

#define magic 0x1BADB002

void kernel_panic(const char* message) 
{
    printf("Kernel Panic: %s\n", message);
    while (1);
}

void dummyfunction1()
{
    
    while(1){}
    //thread_exit();
    
}


void dummyfunction2()
{
    printf("Hallo, ich bin Kernel Thread Nr.1\n");
    while(1){}
    //thread_exit();
}

struct vbe_info vbeinfo;
void kernel_main(uint32_t magic_value, struct multiboot_info* multibootinfo)
{
    init_gdt();
    idt_init();
    init_keyboard();
    if (magic_value != 0x2BADB002)
    {
        printf("Invalid magic value: %x\n", magic_value);
        kernel_panic("Invalid magic value");
    }
    

    vbeinfo.framebuffer_addr = multibootinfo->framebuffer_addr;
    vbeinfo.framebuffer_pitch = multibootinfo->framebuffer_pitch;
    vbeinfo.framebuffer_width = multibootinfo->framebuffer_width;
    vbeinfo.framebuffer_height = multibootinfo->framebuffer_height;
    vbeinfo.framebuffer_bpp = multibootinfo->framebuffer_bpp;

    init_vbe(&vbeinfo);
    set_vbe_info(&vbeinfo);
    //draw_rectangle(212, 300, 400, 100, COLOR_BLUE, &vbeinfo);
    //draw_string(450, 300, "Herzlich willkommen bei senob ;)", COLOR_GREEN, &vbeinfo);


    uint32_t physicalAllocStart = 0x100000 * 16;

    init_memory(multibootinfo->mem_upper * 1024, physicalAllocStart);       //mem_upper comes in KiB (hex 0x1fb80), so it gets multiplied by 1024 so i got it in bytes
    heap_init();
    test_heap_shrink_and_reuse();
    //printf("Its gonna be alright.\n");

    //rust_testfunction();

    //uint32_t* new_dir = mem_alloc_page_dir();
    //mem_change_page_directory(new_dir);
    

    /*struct pcb* idle_process = (struct pcb*) kmalloc(sizeof(struct pcb));
    printf("created idle process successfully\n");
    init_processes(idle_process);
    printf("Idle process initialization successfull\n");
    create_kernel_thread(idle_process, idle_thread);
    idle_process->thread_head->state = IDLET;
    printf("kernel thread for idle process created\n");*/


    //context_switch(idle_process->thread_head);
    /*struct pcb* user_process = (struct pcb*) kmalloc(sizeof(struct pcb));
    printf("Creating user process\n");
    init_processes(user_process);
    create_user_thread(user_process, dummyfunction1);
    //proc_enter_usermode();
    struct pcb* kernelprocess = (struct pcb*) kmalloc(sizeof(struct pcb));
    create_kernel_thread(kernelprocess, dummyfunction2);
    printf("Ich bin der Thread Nr: %d vom Prozess %d", kernelprocess->thread_count, kernelprocess->pid);
    context_switch(kernelprocess->thread_head);*/

    
   

    //context_switch(user_process->thread_head);

    //init_pit(50);

    //schedule();
    
    


    while (1){}
    
}
