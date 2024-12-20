#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#define TSS_BSP 5
#define TSS_AP1 6
#define TSS_AP2 7
#define TSS_AP3 8
#define TSS_AP4 9


extern struct gdt_ptr_struct gdt_ptr;

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


struct tss {
    uint16_t previous_task, __previous_task_unused;
    uint32_t esp0;
    uint16_t ss0, __ss0_unused;
    uint32_t esp1;
    uint16_t ss1, __ss1_unused;
    uint32_t esp2;
    uint16_t ss2, __ss2_unused;
    uint32_t cr3;
    uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint16_t es, __es_unused;
    uint16_t cs, __cs_unused;
    uint16_t ss, __ss_unused;
    uint16_t ds, __ds_unused;
    uint16_t fs, __fs_unused;
    uint16_t gs, __gs_unused;
    uint16_t ldt_selector, __ldt_sel_unused;
    uint16_t debug_flag, io_map;
} __attribute__ ((packed));


void init_gdt();
void setGdtEntry(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint32_t flags);
void setTSS(uint32_t num, uint16_t ss0, uint32_t esp0);
void update_tss_esp0(uint32_t esp0, uint32_t AP_NR);
#endif
