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

#include "gdt.h"
#include "../../../libk/memory.h"

extern void tss_flush();
extern void gdt_flush(uint32_t);

struct gdt_entry_struct gdt_entries[6];
struct gdt_ptr_struct gdt_ptr;
struct tss tss;

void init_gdt()
{
    gdt_ptr.limit = (sizeof(struct gdt_entry_struct) * 6) -1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    setGdtEntry(0, 0, 0, 0, 0);                  //Null segment
    setGdtEntry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);   //Kernel code segment       -> 0x9A = 1001 1010 (these are the privileges etc.)
    setGdtEntry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);   //Kernel data segment
    setGdtEntry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);   //User code segment         -> 0xFA = 1111 1010
    setGdtEntry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);   //User data segment
    setTSS(5, 0x10, 0x0);

    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();
}

void setTSS(uint32_t num, uint16_t ss0, uint32_t esp0)
{
    uint32_t base = (uint32_t) &tss;
    uint32_t limit = base + sizeof(tss);

    setGdtEntry(num, base, limit, 0xE9, 0x00);      //0xE9 = 1110 1001
    memset(&tss, 0, sizeof(tss));
    
    tss.ss0 = ss0;      //set kernel stack segment
    tss.esp0 = esp0;    //set kernel stack pointer

    //User mode segment registers
    tss.cs = 0x08 | 0x3;    //0x3 sets the privilege level to ring 3 (0x3 sets the two lowest bits from the segment selector(cs here))
    tss.ss = 0x10 | 0x3;
    tss.ds = 0x10 | 0x3;
    tss.es = 0x10 | 0x3;
    tss.fs = 0x10 | 0x3;
    tss.gs = 0x10 | 0x3;
}

void setGdtEntry(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint32_t flags)
{

    //base address
    gdt_entries[num].base_low = (base & 0xFFFF);            //The low 16 bits
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;     //The middle 8 bits
    gdt_entries[num].base_high = (base >> 24) & 0xFF;       //The high 8 bits

    gdt_entries[num].limit = (limit & 0xFFFF);
    gdt_entries[num].flags = (limit >> 16) & 0x0F;          //Hight 4 bits of the limit
    gdt_entries[num].flags |= (flags & 0xF0);               //Place flags on top of the limit

    gdt_entries[num].access = access;

}