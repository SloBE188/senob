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
#include "../../../../drivers/video/vga/vga.h"
#include "../io/io.h"
#include <stdint-gcc.h>
#include "../../../libk/stdiok.h"
#include "../sys/process.h"

#define PIT_IRQ 0

#define PIT_CHANNEL_1_PORT 0x40
#define PIT_CHANNEL_2_PORT 0x41
#define PIT_CHANNEL_3_PORT 0x42
#define PIT_CONTROL 0x43

#define PIT_MASK 0xFF
#define PIT_SCALE 1193180


static volatile uint64_t pit_ticks = 0;

void pit_wait(uint64_t ms)
{
    uint64_t target_ticks = pit_ticks + ms;

    // Warten bis die gewünschte Anzahl von Ticks erreicht ist
    while (pit_ticks < target_ticks)
        asm volatile("hlt");
}

void irq0_handler(struct Interrupt_registers *regs)
{
    pit_ticks += 1;
    //schedule();
    //printf("testing irq0");

}


void init_pit(uint64_t hz)
{
    //adding interrupt
    //irq_add_handler(PIT_IRQ, &irq0_handler);

    uint64_t divisor = PIT_SCALE / hz;

    //0x36 os 0011 0110 from the bits perspective
    outb(PIT_CONTROL, 0x36);
    outb(PIT_CHANNEL_1_PORT, divisor & PIT_MASK);
    outb(PIT_CHANNEL_1_PORT, (divisor >> 8) & PIT_MASK);

}