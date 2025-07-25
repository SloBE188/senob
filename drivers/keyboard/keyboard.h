#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../../kernel/arch/x86-32/interrupts/idt.h"

void irq1_handler(struct Interrupt_registers *regs);
void init_keyboard();
uint32_t get_key_from_buffer();

#endif // KEYBOARD_H
