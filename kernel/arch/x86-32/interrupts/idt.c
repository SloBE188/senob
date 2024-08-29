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
struct idt_entry_t idt_descriptors[256];
struct idtr_t idtr;

extern void idt_flush(uint32_t);



void idt_set_descriptor(uint8_t interrupt_number, uint32_t isr) {
    struct idt_entry_t *descriptor = &idt_descriptors[interrupt_number];
    descriptor->isr_low = isr & 0xFFFF;
    descriptor->kernel_cs = 0x08;
    descriptor->reserved = 0x00;
    descriptor->type = 0x8E; // Present, DPL=0, Type=0xE (32-bit interrupt gate)
    descriptor->isr_high = (isr >> 16) & 0xFFFF;
}






//Unitializes the Master & Slave PIC
void init_pic()
{
    //0x20 = Master PIC, 0xA0 = Slave PIC
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

void idt_init()
{

    memset(&idt_descriptors, 0, sizeof(struct idt_entry_t) * 256);
    idtr.limit = sizeof(struct idt_entry_t) * 256 - 1;
    idtr.base = (uint32_t)&idt_descriptors;

    init_pic();

    //Exceptions
    idt_set_descriptor(0, (uint32_t)isr0);
    idt_set_descriptor(1, (uint32_t)isr1);
    idt_set_descriptor(2, (uint32_t)isr2);
    idt_set_descriptor(3, (uint32_t)isr3);
    idt_set_descriptor(4, (uint32_t)isr4);
    idt_set_descriptor(5, (uint32_t)isr5);
    idt_set_descriptor(6, (uint32_t)isr6);
    idt_set_descriptor(7, (uint32_t)isr7);
    idt_set_descriptor(8, (uint32_t)isr8);
    idt_set_descriptor(9, (uint32_t)isr9);
    idt_set_descriptor(10, (uint32_t)isr10);
    idt_set_descriptor(11, (uint32_t)isr11);
    idt_set_descriptor(12, (uint32_t)isr12);
    idt_set_descriptor(13, (uint32_t)isr13);
    idt_set_descriptor(14, (uint32_t)isr14);
    idt_set_descriptor(15, (uint32_t)isr15);
    idt_set_descriptor(16, (uint32_t)isr16);
    idt_set_descriptor(17, (uint32_t)isr17);
    idt_set_descriptor(18, (uint32_t)isr18);
    idt_set_descriptor(19, (uint32_t)isr19);
    idt_set_descriptor(20, (uint32_t)isr20);
    idt_set_descriptor(21, (uint32_t)isr21);
    idt_set_descriptor(22, (uint32_t)isr22);
    idt_set_descriptor(23, (uint32_t)isr23);
    idt_set_descriptor(24, (uint32_t)isr24);
    idt_set_descriptor(25, (uint32_t)isr25);
    idt_set_descriptor(26, (uint32_t)isr26);
    idt_set_descriptor(27, (uint32_t)isr27);
    idt_set_descriptor(28, (uint32_t)isr28);
    idt_set_descriptor(29, (uint32_t)isr29);
    idt_set_descriptor(30, (uint32_t)isr30);
    idt_set_descriptor(31, (uint32_t)isr31);

    //irqs
    idt_set_descriptor(32, (uint32_t)irq0);
    idt_set_descriptor(33, (uint32_t)irq1);
    idt_set_descriptor(34, (uint32_t)irq2);
    idt_set_descriptor(35, (uint32_t)irq3);
    idt_set_descriptor(36, (uint32_t)irq4);
    idt_set_descriptor(37, (uint32_t)irq5);
    idt_set_descriptor(38, (uint32_t)irq6);
    idt_set_descriptor(39, (uint32_t)irq7);
    idt_set_descriptor(40, (uint32_t)irq8);
    idt_set_descriptor(41, (uint32_t)irq9);
    idt_set_descriptor(42, (uint32_t)irq10);
    idt_set_descriptor(43, (uint32_t)irq11);
    idt_set_descriptor(44, (uint32_t)irq12);
    idt_set_descriptor(45, (uint32_t)irq13);
    idt_set_descriptor(46, (uint32_t)irq14);
    idt_set_descriptor(47, (uint32_t)irq15);

    //Syscalls
    idt_set_descriptor(128, (uint32_t)isr128);
    idt_set_descriptor(177, (uint32_t)isr177);

    idt_flush((uint32_t)&idtr);

}

static const char* exceptions[] = {
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
    "[0x1F] Inexplicable Error"
};


void isr_handler(struct Interrupt_registers *regs)
{
    if(regs->interrupt_number < 32)
    {
        printf(exceptions[regs->interrupt_number]);
        printf("\n");
        printf("Exception occured!\n");
        for(;;);
    }
}

void *irq_handlers[16] = {
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};

void irq_add_handler(int irq_number, void (*handler)(struct Interrupt_registers *regs))
{
    irq_handlers[irq_number] = handler;
}

void irq_delete_handler(int irq_number)
{   
    irq_handlers[irq_number] = 0x00;
}

void irq_handler(struct Interrupt_registers *regs)
{
    void (*handler)(struct Interrupt_registers *regs);

    handler = irq_handlers[regs->interrupt_number - 32];
    if (handler)
    {
        handler(regs);
    }

    if (regs->interrupt_number >= 40)
    {
        outb(0xA0, 0x20);
    }

    outb(0x20,0x20);
    
    
}