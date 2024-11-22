#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct idt_entry_t{
	uint16_t    isr_low;      // The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t     reserved;     // Set to zero
	uint8_t     type;   // Type and attributes; see the IDT page
	uint16_t    isr_high;     // The higher 16 bits of the ISR's address
} __attribute__((packed));


struct idtr_t{
	uint16_t	limit;
	uint32_t	base;
} __attribute__((packed));

struct Interrupt_registers{
    uint32_t cr2;
    uint32_t ds;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t interrupt_number;
    uint32_t err_code;
    uint32_t eip;
    uint32_t csm;
    uint32_t eflags;
    uint32_t useresp;
    uint32_t ss;
};


//Exceptions
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

//irqs
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();


//Syscalls
extern void isr80();

void idt_set_descriptor(uint8_t interrupt_number, uint32_t isr, uint8_t dpl);
void idt_init();
void isr_handler(struct Interrupt_registers *regs);
void irq_add_handler(int irq_number, void (*handler)(struct Interrupt_registers *regs));
void irq_delete_handler(int irq_number);

#endif
