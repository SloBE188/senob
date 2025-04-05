#ifndef KERNEL_H
#define KERNEL_H

#define NULL 0
#define MAX_CPUS 8

#include <stdint.h>

#include "interrupts/idt.h"

void dummyfunction1();
void dummyfunction2();

void pit_handler(struct Interrupt_registers *regs);
void PitWait(uint32_t ms);

extern uint64_t cpuTicks[MAX_CPUS];

void kernel_panic(const char* message);
#define assert(condition) \
    do { \
        if (!(condition)) { \
            kernel_panic("Assertion failed: " #condition); \
        } \
    } while (0)

#endif