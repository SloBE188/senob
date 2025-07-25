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
#include "../kernel.h"

#define PIT_IRQ 0

#define PIT_CHANNEL_1_PORT 0x40
#define PIT_CHANNEL_2_PORT 0x41
#define PIT_CHANNEL_3_PORT 0x42
#define PIT_CONTROL 0x43

#define PIT_MASK 0xFF
#define PIT_SCALE 1193180





/*void pit_handler(struct Interrupt_registers *regs)
{
    pit_ticks++;
    //schedule();
    asm volatile("sti");

}*/


void init_pit()
{
    //adding interrupt
    vector_add_handler(PIT_IRQ, &pit_handler);
    uint32_t hz = 1000;

    uint64_t divisor = PIT_SCALE / hz;

    //0x36 os 0011 0110 from the bits perspective
    outb(PIT_CONTROL, 0x36);
    outb(PIT_CHANNEL_1_PORT, divisor);
    outb(PIT_CHANNEL_1_PORT, divisor >> 8);

}