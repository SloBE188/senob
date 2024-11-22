#ifndef GDT_H
#define GDT_H

#include <stdint-gcc.h>

extern struct tss tss;

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
    uint32_t previous_task;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt_selector;
    uint16_t trap;
    uint16_t io_map;
} __attribute__ ((packed));


void init_gdt();
void setGdtEntry(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint32_t flags);
void setTSS(uint32_t num, uint16_t ss0, uint32_t esp0);
void update_tss_esp0(uint32_t esp0);
#endif
