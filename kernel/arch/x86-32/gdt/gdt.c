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

struct tss tss;

void init_gdt()
{
    gdt_ptr.limit = (sizeof(struct gdt_entry_struct) * 6) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    memset(&tss, 0, sizeof(tss));
    tss.ss0 = 0x10;
    // User mode segment registers
    tss.cs = 0x1B;  // 0x1B = User Code Segment Selector | RPL 3
    tss.ss = 0x23;  // 0x23 = User Data Segment Selector | RPL 3
    tss.ds = 0x23;
    tss.es = 0x23;
    tss.fs = 0x23;
    tss.gs = 0x23;

    setGdtEntry(0, 0, 0, 0, 0);                  // Null segment
    setGdtEntry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);   // Kernel code segment
    setGdtEntry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);   // Kernel data segment
    setGdtEntry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);   // User code segment
    setGdtEntry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);   // User data segment
    setGdtEntry(5, (uint32_t) &tss, sizeof(tss), 0x89, 0x40);

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

void update_tss_esp0(uint32_t esp0) 
{
    tss.esp0 = esp0;
}