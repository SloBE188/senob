#include "startup.h"
#include "../gdt/gdt.h"
#include "../interrupts/idt.h"
#include "../mm/paging/paging.h"

extern struct idtr_t idtr;
extern struct gdt_ptr_struct gdt_ptr;
extern uint32_t kernel_directory[1024];

extern void idt_flush(uint32_t);
extern void gdt_flush(uint32_t);

void enable_paging()
{
    asm volatile(
        "push %ebp \n"
        "mov %esp, %ebp \n"
        "mov %cr0, %eax \n"
        "or $0x80000000, %eax \n"
        "mov %eax, %cr0 \n"
        "mov %ebp, %esp \n"
        "pop %ebp \n"
        "ret \n"
    );
}

static inline void load_tr(uint16_t tss_selector)
{
    asm volatile("ltr %0" : : "r"(tss_selector));
}

void initialize_ap()
{
    uint32_t apic_id = 2;   //will get that from cpuid, just as an example here
    uint32_t ap_tss_selector = (5 + apic_id) << 3;  //*8 for getting the right entry

    gdt_flush((uint32_t)&gdt_ptr);
    load_tr(ap_tss_selector);

    idt_flush((uint32_t)&idtr);
    enable_paging();
    mem_change_page_directory(&kernel_directory);
    while(1){}
}