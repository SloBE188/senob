#include "gdt.h"


extern void gdt_flush(addr_t);

struct gdt_entry_struct gdt_entries[5];
struct gdt_ptr_struct gdt_ptr;

void initGdt()
{
    gdt_ptr.limit = (sizeof(struct gdt_entry_struct) * 5) -1;
    gdt_ptr.base = &gdt_entries;

    setGdtGate(0, 0, 0, 0, 0);                  //Null segment
    setGdtGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);   //Kernel code segment       -> 0x9A = 1001 1010 (these are the privileges etc.)
    setGdtGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);   //Kernel data segment
    setGdtGate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);   //User code segment         -> 0xFA = 1111 1010
    setGdtGate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);   //User data segment

    gdt_flush(&gdt_ptr);
}

void setGdtGate(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint32_t flags)
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