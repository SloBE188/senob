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
    uint16_t previous_task;
    uint32_t esp0;
    uint16_t ss0;
    uint32_t esp1;
    uint16_t ss1;
    uint32_t esp2;
    uint16_t ss2;
    uint32_t cr3;
    uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint16_t es;
    uint16_t cs;
    uint16_t ss;
    uint16_t ds;
    uint16_t fs;
    uint16_t gs;
    uint16_t ldt_selector;
    uint16_t io_map;
} __attribute__ ((packed));


void init_gdt();
void setGdtEntry(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint32_t flags);
void setTSS(uint32_t num, uint16_t ss0, uint32_t esp0);
void update_tss_esp0(uint32_t esp0);
#endif