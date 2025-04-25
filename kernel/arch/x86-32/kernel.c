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
#include "sys/lapic.h"
#include "interrupts/pit.h"
#include <stdio.h>
#include "sys/sched.h"

extern void rust_testfunction();

#define magic 0x1BADB002

void kernel_panic(const char *message)
{
    kernel_write("Kernel Panic: %s\n", message);
    while (1)
        ;
}

uint64_t cpuTicks[MAX_CPUS];
volatile uint64_t pit_ticks;
void pit_handler(struct Interrupt_registers *regs)
{
    pit_ticks++;
    // schedule();
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
        kernel_write("Invalid magic value: %x\n", magic_value);
        kernel_panic("Invalid magic value");
    }

    vbeinfo.framebuffer_addr = multibootinfo->framebuffer_addr;
    vbeinfo.framebuffer_pitch = multibootinfo->framebuffer_pitch;
    vbeinfo.framebuffer_width = multibootinfo->framebuffer_width;
    vbeinfo.framebuffer_height = multibootinfo->framebuffer_height;
    vbeinfo.framebuffer_bpp = multibootinfo->framebuffer_bpp;

    init_vbe(&vbeinfo);
    set_vbe_info(&vbeinfo);

    init_ramdisk_disk(multibootinfo);

    uint32_t physicalAllocStart = 0x100000 * 16;

    if (using_ramdisk)
    {
        physicalAllocStart = (ramdisk.phys_addr + ramdisk.size + 0xFFF) & 0xFFFFF000;
    }

    init_memory(multibootinfo->mem_upper * 1024, physicalAllocStart); // mem_upper comes in KiB (hex 0x1fb80), so it gets multiplied by 1024 so i got it in bytes
    heap_init();
    kernel_write("Module: %d\n", multibootinfo->mods_count);
    create_ramdisk();

    disk_initialize(0);
    disk_status(0);

    FATFS fs;
    FRESULT res;

    res = f_mount(&fs, "0:", 1);
    kernel_write("result of mount: %d\n", res);

    if (res != FR_OK)
    {
        kernel_write("Filesystem mount failed.\n");
    }
    else
    {
        kernel_write("Filesystem mounted successfully!\n");
    }

    setup_vectors();
    init_keyboard();
    init_syscalls();
    DIR dir;
    FILINFO fno;

    if (f_opendir(&dir, "0:/") == FR_OK)
    {
        while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != 0)
        {
            kernel_write("Gefunden: %s\n", fno.fname);
        }
        f_closedir(&dir);
    }

    

    initProc();
    smpInit();
    initIdle();
    fillRunqueuesFromBST();

    struct process* doom = createUserProcess("0:/doom.bin");
    updateTssEsp0(doom->head_thread->kstack.esp0, get_local_apic_id_cpuid());
    switchTask(doom->head_thread->regs);

    while (1)
    {
        asm volatile("hlt");
    }
}