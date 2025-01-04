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

#include "idt.h"
#include "../../../libk/memory.h"
#include "../io/io.h"
#include "../../../../drivers/video/vga/vga.h"
#include "../../../libk/stdiok.h"
#include "../mm/paging/paging.h"
#include "../syscalls/syscalls.h"
#include "../sys/apic.h"
#include "pit.h"
#include "../kernel.h"

struct idt_entry_t idt_descriptors[256];
struct idtr_t idtr;

extern void idt_flush(uint32_t);

void idt_set_descriptor(uint8_t interrupt_number, uint32_t isr, uint8_t dpl)
{
    struct idt_entry_t *descriptor = &idt_descriptors[interrupt_number];
    descriptor->isr_low = isr & 0xFFFF;
    descriptor->kernel_cs = 0x08;
    descriptor->reserved = 0x00;
    descriptor->type = 0x8E | (dpl << 5); // Set DPL in bits 5-6, Present, DPL=0, Type=0xE (32-bit interrupt gate)
    descriptor->isr_high = (isr >> 16) & 0xFFFF;
}

// Unitializes the Master & Slave PIC
void init_pic()
{
    // 0x20 = Master PIC, 0xA0 = Slave PIC
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, 0x00);
    outb(0xA1, 0x00);

    //outb(0x21, 0xFD);
    //outb(0xA1, 0xFF);
}

void spurious_interrupt_handler(struct Interrupt_registers *regs);
void apic_timer_handler(struct Interrupt_registers *regs);
void apic_error_handler(struct Interrupt_registers *regs);

void idt_init()
{

    memset(&idt_descriptors, 0, sizeof(struct idt_entry_t) * 256);
    idtr.limit = sizeof(struct idt_entry_t) * 256 - 1;
    idtr.base = (uint32_t)&idt_descriptors;

    init_pic();

    // Exceptions
    idt_set_descriptor(0, (uint32_t)isr0, 1);
    idt_set_descriptor(1, (uint32_t)isr1, 1);
    idt_set_descriptor(2, (uint32_t)isr2, 1);
    idt_set_descriptor(3, (uint32_t)isr3, 1);
    idt_set_descriptor(4, (uint32_t)isr4, 1);
    idt_set_descriptor(5, (uint32_t)isr5, 1);
    idt_set_descriptor(6, (uint32_t)isr6, 1);
    idt_set_descriptor(7, (uint32_t)isr7, 1);
    idt_set_descriptor(8, (uint32_t)isr8, 1);
    idt_set_descriptor(9, (uint32_t)isr9, 1);
    idt_set_descriptor(10, (uint32_t)isr10, 1);
    idt_set_descriptor(11, (uint32_t)isr11, 1);
    idt_set_descriptor(12, (uint32_t)isr12, 1);
    idt_set_descriptor(13, (uint32_t)isr13, 1);
    idt_set_descriptor(14, (uint32_t)isr14, 1);
    idt_set_descriptor(15, (uint32_t)isr15, 1);
    idt_set_descriptor(16, (uint32_t)isr16, 1);
    idt_set_descriptor(17, (uint32_t)isr17, 1);
    idt_set_descriptor(18, (uint32_t)isr18, 1);
    idt_set_descriptor(19, (uint32_t)isr19, 1);
    idt_set_descriptor(20, (uint32_t)isr20, 1);
    idt_set_descriptor(21, (uint32_t)isr21, 1);
    idt_set_descriptor(22, (uint32_t)isr22, 1);
    idt_set_descriptor(23, (uint32_t)isr23, 1);
    idt_set_descriptor(24, (uint32_t)isr24, 1);
    idt_set_descriptor(25, (uint32_t)isr25, 1);
    idt_set_descriptor(26, (uint32_t)isr26, 1);
    idt_set_descriptor(27, (uint32_t)isr27, 1);
    idt_set_descriptor(28, (uint32_t)isr28, 1);
    idt_set_descriptor(29, (uint32_t)isr29, 1);
    idt_set_descriptor(30, (uint32_t)isr30, 1);
    idt_set_descriptor(31, (uint32_t)isr31, 1);

    // irqs;
    idt_set_descriptor(0x20, (uint32_t)irq0, 1);
    idt_set_descriptor(0x21, (uint32_t)irq1, 1);
    idt_set_descriptor(0x32, (uint32_t)apic_timer_handler, 1);
    idt_set_descriptor(0xFE, (uint32_t)apic_error_handler, 1);
    idt_set_descriptor(0xFF, (uint32_t)spurious_interrupt_handler, 1);

    // Syscalls
    idt_set_descriptor(128, (uint32_t)isr128, 3);

    idt_flush((uint32_t)&idtr);
}

void spurious_interrupt_handler(struct Interrupt_registers *regs)
{
    kernel_write("Spurious interrupt received: 0x%x\n", regs->interrupt_number);
}

void apic_timer_handler(struct Interrupt_registers *regs)
{
    kernel_write("APIC TIMER Interrupt occured\n");

}

void apic_error_handler(struct Interrupt_registers *regs)
{
    kernel_write("APIC Error Interrupt Triggered\n");
}

static const char *exceptions[] = {
    "[0x00] Divide by Zero Exception",
    "[0x01] Debug Exception",
    "[0x02] Unhandled Non-maskable Interrupt",
    "[0x03] Breakpoint Exception",
    "[0x04] Overflow Exception",
    "[0x05] Bound Range Exceeded Exception",
    "[0x06] Invalid Opcode/Operand Exception",
    "[0x07] Device Unavailable Exception",
    "[0x08] Double Fault",
    "[0x09] Coprocessor Segment Overrun",
    "[0x0A] Invalid TSS Exception",
    "[0x0B] Absent Segment Exception",
    "[0x0C] Stack-segment Fault",
    "[0x0D] General Protection Fault",
    "[0x0E] Page Fault",
    "[0x0F] Inexplicable Error",
    "[0x10] x87 Floating Exception",
    "[0x11] Alignment Check",
    "[0x12] Machine Check",
    "[0x13] SIMD Floating Exception",
    "[0x14] Virtualized Exception",
    "[0x15] Control Protection Exception",
    "[0x16] Inexplicable Error",
    "[0x17] Inexplicable Error",
    "[0x18] Inexplicable Error",
    "[0x19] Inexplicable Error",
    "[0x1A] Inexplicable Error",
    "[0x1B] Inexplicable Error",
    "[0x1C] Hypervisor Intrusion Exception",
    "[0x1D] VMM Communications Exception",
    "[0x1E] Security Exception",
    "[0x1F] Inexplicable Error"};

void page_fault_handler(struct Interrupt_registers *r)
{
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r"(faulting_address));

    kernel_write("Page fault at address: 0x%x, error code: 0x%x\n", faulting_address, r->err_code);

    if (!(r->err_code & 0x1))
    {
        kernel_write("Page not present.\n");
    }
    else
    {
        kernel_write("Page protection violation.\n");
    }

    while (1)
    {
    }
}

void syscall_handler(struct Interrupt_registers *regs)
{
    uint32_t syscall_number = regs->eax;

    if (syscall_number >= 1024)
    {
        kernel_write("not a valid syscall nr: %u\n", syscall_number);
        return;
    }

    syscall_functions[syscall_number](regs);
}

void isr_handler(struct Interrupt_registers *regs)
{
    if (regs->interrupt_number < 32)
    {
        if (regs->interrupt_number == 14)
        {
            page_fault_handler(regs);
        }

        kernel_write(exceptions[regs->interrupt_number]);
        kernel_write("\n");
        kernel_write("Exception occured!\n");
        for (;;)
            ;
    }

    if (regs->interrupt_number == 128)
    {
        syscall_handler(regs);
    }
}

void *vector_handlers[256] = {0}; // all vectors

void vector_add_handler(int vector, void (*handler)(struct Interrupt_registers *regs))
{
    vector_handlers[vector] = handler;
}

void vector_remove_handler(int vector)
{
    vector_handlers[vector] = 0;
}

void setup_vectors()
{
    vector_add_handler(0x20, &pit_handler);
    vector_add_handler(0x32, &apic_error_handler);
    vector_add_handler(0xFE, apic_error_handler);
    vector_add_handler(0xFF, spurious_interrupt_handler);
}

void irq_handler(struct Interrupt_registers *regs)
{
    int vector = regs->interrupt_number - 32; // find vector

    //kernel_write("Interrupt vector %d triggered\n", vector);

    void (*handler)(struct Interrupt_registers *regs);
    handler = vector_handlers[vector];
    if (handler)
    {
        handler(regs);
    }
    if(regs->interrupt_number >=15)
    {
        outb(0x20,0x20);
        outb(0xA0, 0x20);
    }else
    {
        lapicw(EOI, 0);
    }
    
    // send EOI to the local apic
    //lapicw(EOI, 0);
    /*lapicw(EOI, 0);
    
    uint32_t esr = lapic_read(ESR);
    kernel_write("ESR: 0x%x\n", esr);*/
}
