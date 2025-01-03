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

// #include "../../../drivers/video/vga/vga.h"
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
#include "../../libk/memory.h"
#include "disk/ramdisk.h"
#include "fatfs/ff.h"
#include "fatfs/diskio.h"
#include "syscalls/syscalls.h"
#include "sys/smp.h"
#include "sys/apic.h"
#include "interrupts/pit.h"

extern void rust_testfunction();

#define magic 0x1BADB002

void kernel_panic(const char *message)
{
    printf("Kernel Panic: %s\n", message);
    while (1)
        ;
}

volatile uint64_t pit_ticks;


void pit_handler(struct Interrupt_registers *regs)
{
    pit_ticks++;
    //schedule();
    asm volatile("sti");

}

void PitWait(uint32_t ms)
{
    uint32_t now = pit_ticks;
    ++ms;

    while (pit_ticks - now < ms)
    {
        ;
    }
}


struct vbe_info vbeinfo;
void kernel_main(uint32_t magic_value, struct multiboot_info *multibootinfo)
{
    init_gdt();
    idt_init();
    init_pit();
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
    // draw_rectangle(212, 300, 400, 100, COLOR_BLUE, &vbeinfo);
    // draw_string(450, 300, "Herzlich willkommen bei senob ;)", COLOR_GREEN, &vbeinfo);

    init_ramdisk_disk(multibootinfo);

    uint32_t physicalAllocStart = 0x100000 * 16;

    if (using_ramdisk)
    {
        physicalAllocStart = (ramdisk.phys_addr + ramdisk.size + 0xFFF) & 0xFFFFF000;
    }

    init_memory(multibootinfo->mem_upper * 1024, physicalAllocStart); // mem_upper comes in KiB (hex 0x1fb80), so it gets multiplied by 1024 so i got it in bytes
    heap_init();
    printf("Module: %d\n", multibootinfo->mods_count);
    create_ramdisk();

    disk_initialize(0);
    disk_status(0);


    FATFS fs;
    FRESULT res;

    res = f_mount(&fs, "0:", 1);
    printf("result of mount: %d\n", res);

    if (res != FR_OK)
    {
        printf("Filesystem mount failed.\n");
    }
    else
    {
        printf("Filesystem mounted successfully!\n");
    }

    init_syscalls();
    init_proc();
    DIR dir;
    FILINFO fno;

    if (f_opendir(&dir, "0:/") == FR_OK) {
        while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != 0) {
            printf("Gefunden: %s\n", fno.fname);
        }
        f_closedir(&dir);
    }
    

    lapic_init();
    struct addr *addr = smp_addresses(multibootinfo);
    init_smp(addr->floating_ptr_addr, addr->mp_config_table_addr);
    printf("\n\n\nfloating_ptr_addr: 0x%x\nmp_table_addr: 0x%x\nlocal_apic_addr: 0x%x\n", addr->floating_ptr_addr, addr->mp_config_table_addr, addr->local_apic);

    setup_vectors();
    init_keyboard();


    scheduler();

    // test_heap_shrink_and_reuse();
    while (1)
    {
        asm volatile("hlt");
    }
}
