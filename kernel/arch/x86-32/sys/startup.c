#include "startup.h"
#include "../gdt/gdt.h"
#include "../interrupts/idt.h"
#include "../mm/paging/paging.h"

extern struct idtr_t idtr;
extern struct gdt_ptr_struct gdt_ptr;
extern uint32_t kernel_directory[1024];

extern void idt_flush(uint32_t);

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

void initialize_ap()
{
    
    idt_flush((uint32_t)&idtr);
    enable_paging();
    mem_change_page_directory(&kernel_directory);
}