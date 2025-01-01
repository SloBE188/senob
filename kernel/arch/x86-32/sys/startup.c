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

#include "startup.h"
#include "../gdt/gdt.h"
#include "../interrupts/idt.h"
#include "../mm/paging/paging.h"
#include "smp.h"
#include "process.h"
#include "../kernel.h"

extern struct idtr_t idtr;
extern struct gdt_ptr_struct gdt_ptr;
extern uint32_t kernel_directory[1024];

extern void idt_flush(uint32_t);
extern void gdt_flush(uint32_t);

void enable_paging()
{
    asm volatile(
        "push %ebp \n"
        "mov %esp, %ebp \n"
        "mov %cr0, %eax \n"
        "or $0x80000000, %eax \n"
        "mov %eax, %cr0 \n"
        "mov %ebp, %esp \n"
        "pop %ebp \n"
        "ret \n"
    );
}

static inline void load_tr(uint16_t tss_selector)
{
    asm volatile("ltr %0" : : "r"(tss_selector));
}


void initialize_ap()
{

    uint32_t apic_id = get_local_apic_id_cpuid();
    uint32_t ap_tss_selector = (5 + apic_id) << 3;  //*8 for getting the right entry

    gdt_flush((uint32_t)&gdt_ptr);
    load_tr(ap_tss_selector);

    idt_flush((uint32_t)&idtr);
    mem_change_page_directory(&kernel_directory);
    scheduler();
    while(1){}
}