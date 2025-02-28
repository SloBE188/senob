#include "gdt.h"
#include "../../../libk/memory.h"
#include "../mm/heap/heap.h"
#include "../kernel.h"

extern void tss_flush();
extern void gdt_flush(uint32_t);


#define KERNEL_CODE_SEG 0x9A
#define KERNEL_DATA_SEG 0x92
#define USER_CODE_SEG 0xFA
#define USER_DATA_SEG 0xF2
#define TSS_SEG 0x89

#define BAST_GDT_ENTRIES 5
#define TOTAL_GDT_ENTRIES (BAST_GDT_ENTRIES + MAX_CPUS)

struct tss tss[MAX_CPUS];
struct gdt_entry_struct gdt_entries[TOTAL_GDT_ENTRIES];
struct gdt_ptr_struct gdt_ptr;

void init_gdt()
{

    setGdtEntry(0, 0, 0, 0, 0);                // Null segment
    setGdtEntry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel code segment
    setGdtEntry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Kernel data segment
    setGdtEntry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User code segment
    setGdtEntry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User data segment

    // BSP
    memset(&tss[0], 0, sizeof(struct tss));
    tss[0].ss0 = 0x10;
    tss[0].esp0 = 0;
    tss[0].io_map = sizeof(struct tss);
    uint32_t tss_base = (uint32_t)&tss[0];
    uint32_t tss_limit = sizeof(struct tss) - 1;
    setGdtEntry(5, tss_base, tss_limit, 0xE9, 0x00);

    // AP TSS
    for (uint32_t i = 1; i < MAX_CPUS; i++)
    {
        memset(&tss[i], 0, sizeof(struct tss));
        // These don't need to be set when we aren't using hardware
        // task switching, which we aren't using.
        // tss.cs   = 0x0B; //from ring 3 - 0x08 | 3 = 0x0B
        // tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13; //from ring 3 = 0x10 | 3 = 0x13
        tss[i].ss0 = 0x10;
        tss[i].esp0 = 0;
        tss[i].io_map = sizeof(struct tss);

        uint32_t tss_base_ap = (uint32_t)&tss[i];
        uint32_t tss_limit_ap = sizeof(struct tss) - 1;
        setGdtEntry(5 + i, tss_base_ap, tss_limit_ap, 0xE9, 0x00);
    }

    gdt_ptr.limit = (sizeof(struct gdt_entry_struct) * TOTAL_GDT_ENTRIES) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();    //only loads tss for the bsp
}

void setGdtEntry(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint32_t flags)
{
    // Base address
    gdt_entries[num].base_low = (base & 0xFFFF);        // The low 16 bits
    gdt_entries[num].base_middle = (base >> 16) & 0xFF; // The middle 8 bits
    gdt_entries[num].base_high = (base >> 24) & 0xFF;   // The high 8 bits

    gdt_entries[num].limit = (limit & 0xFFFF);
    gdt_entries[num].flags = (limit >> 16) & 0x0F; // High 4 bits of the limit
    gdt_entries[num].flags |= (flags & 0xF0);      // Place flags on top of the limit

    gdt_entries[num].access = access;
}

void update_tss_esp0(uint32_t esp0, uint32_t AP_NR)
{
    tss[AP_NR].esp0 = esp0;
}
