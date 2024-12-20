#include "gdt.h"
#include "../../../libk/memory.h"
#include "../mm/heap/heap.h"

extern void tss_flush();
extern void gdt_flush(uint32_t);

struct gdt_entry_struct gdt_entries[6];
struct gdt_ptr_struct gdt_ptr;

#define KERNEL_CODE_SEG 0x9A
#define KERNEL_DATA_SEG 0x92
#define USER_CODE_SEG 0xFA
#define USER_DATA_SEG 0xF2
#define TSS_SEG 0x89



struct tss tss[NUM_CPUS];

void init_tss(uint32_t AP_NR)
{
    tss[AP_NR].esp0 = 0;
    //setGdtEntry(AP_NR,);
}

void init_gdt()
{
    gdt_ptr.limit = (sizeof(struct gdt_entry_struct) * 6) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    memset((uint8_t)&tss[TSS_BSP], 0, sizeof(struct tss));

    tss[TSS_BSP].esp0 = 0;   //0x1FFF0;
    tss[TSS_BSP].ss0 = 0x10; //0x18;

    // These don't need to be set when we aren't using hardware
    // task switching, which we aren't using.
    //tss.cs   = 0x0B; //from ring 3 - 0x08 | 3 = 0x0B
    //tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13; //from ring 3 = 0x10 | 3 = 0x13

    uint32_t tss_base = (uint32_t) &tss[TSS_BSP];
    // TSS limit is one less than the actual size
    uint32_t tss_limit = sizeof(struct tss)-1;
    // Set io_map to higher than limit to disable io bitmap.
    tss[TSS_BSP].io_map = sizeof(struct tss);

    setGdtEntry(0, 0, 0, 0, 0);                  // Null segment
    setGdtEntry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);   // Kernel code segment
    setGdtEntry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);   // Kernel data segment
    setGdtEntry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);   // User code segment
    setGdtEntry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);   // User data segment
    setGdtEntry(5, tss_base, tss_limit, 0xE9, 0x00);

    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();
}

void setGdtEntry(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint32_t flags)
{
    // Base address
    gdt_entries[num].base_low = (base & 0xFFFF);            // The low 16 bits
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;     // The middle 8 bits
    gdt_entries[num].base_high = (base >> 24) & 0xFF;       // The high 8 bits

    gdt_entries[num].limit = (limit & 0xFFFF);
    gdt_entries[num].flags = (limit >> 16) & 0x0F;          // High 4 bits of the limit
    gdt_entries[num].flags |= (flags & 0xF0);               // Place flags on top of the limit

    gdt_entries[num].access = access;
}

void update_tss_esp0(uint32_t esp0, uint32_t AP_NR) 
{
    tss[AP_NR].esp0 = esp0;
}
