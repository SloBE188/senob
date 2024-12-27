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


static inline uint32_t get_local_apic_id_cpuid(void) 
{
    uint32_t eax, ebx, ecx, edx;
    __asm__ volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1)
    );
    // APIC-ID is in the bits 31:24 from ebx
    return (ebx >> 24) & 0xFF;
}

void initialize_ap()
{

    uint32_t apic_id = get_local_apic_id_cpuid();
    uint32_t ap_tss_selector = (5 + apic_id) << 3;  //*8 for getting the right entry

    gdt_flush((uint32_t)&gdt_ptr);
    load_tr(ap_tss_selector);

    idt_flush((uint32_t)&idtr);
    mem_change_page_directory(&kernel_directory);
    while(1){}
}