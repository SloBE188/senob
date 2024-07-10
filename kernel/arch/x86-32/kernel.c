#include "../../../drivers/video/vga/vga.h"
#include "gdt/gdt.h"
#include "interrupts/idt.h"


void trigger_breakpoint() {
    asm volatile("int $3"); // Breakpoint Interrupt
}

void trigger_general_protection_fault() {
    asm volatile (
        "mov $0x10, %ax\n\t"
        "ltr %ax" // Läd ungültiges Segment in TR
    );
}

void trigger_division_by_zero() {
    asm volatile (
        "movl $1, %eax\n\t"  // Setze eax auf 1
        "movl $0, %ebx\n\t"  // Setze ebx auf 0
        "divl %ebx"          // Dividiere eax durch ebx (Division durch Null)
    );
}


void trigger_bound_range_exceeded() {
    int array[2] = {1, 2};
    asm volatile (
        "movl %0, %%eax\n\t"
        "movl $2, %%ecx\n\t"
        "bound %%ecx, (%%eax)"
        :
        : "r"(array)
    );
}

void trigger_overflow() {
    asm volatile (
        "add $0x7FFFFFFF, %eax\n\t"
        "add $1, %eax\n\t"
        "into"
    );
}



void kernel_main(void)
{
    reset();
    initGdt();
    print("--GDT loaded\n");
    print("--TSS loaded\n");
    idt_init();
    print("--IDT loaded\n");
    //trigger_breakpoint();
    //trigger_division_by_zero();
    print("Herzlich willkommen bei senob!\n");
    while (1){}
    
}
