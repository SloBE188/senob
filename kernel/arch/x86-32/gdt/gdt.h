#ifndef GDT_H
#define GDT_H

#include <stdint.h>

struct tss tss;

struct gdt_entry_struct
{
    uint16_t limit;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t flags;
    uint8_t base_high;
}__attribute__((packed));

struct gdt_ptr_struct
{
    uint16_t limit;
    unsigned int base;
}__attribute__((packed));

struct tss
{
    uint32_t prev_tss;
    uint32_t esp0;      // Kernel stack pointer
    uint32_t ss0;       // Kernel stack segment
    uint32_t esp1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t sr3;
    uint32_t eip;       //Instruction pointer (Program counter) ist ein register in der CPU, dass die IMMER Adresse der nächsten auszuführenden Anweusung im proigrammspeicher enthält
    uint32_t eflags;    //spezielles register mit flags
    uint32_t eax;       //General Purpose Registers (AX,BX,CX,DX) 
    uint32_t ecx;       //*
    uint32_t edx;       //*
    uint32_t ebx;       //*
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;        //Segment selector
    uint32_t cs;        //*
    uint32_t ss;        //*
    uint32_t ds;        //*
    uint32_t fs;        //*
    uint32_t gs;        //*
    uint32_t ldt;
    uint32_t trap;
    uint32_t iomap_base;      //I/O permission bitmap.
} __attribute__((packed));


void init_gdt();
void setGdtEntry(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint32_t flags);
void setTSS(uint32_t num, uint16_t ss0, uint32_t esp0);

#endif