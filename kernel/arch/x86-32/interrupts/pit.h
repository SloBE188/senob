#ifndef PIT_H
#define PIT_H
#include <stdint.h>


void irq0_pit(struct Interrupt_registers *regs);
void init_pit(uint64_t hz);


#endif