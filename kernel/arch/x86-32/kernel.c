#include "../../../drivers/video/vga/vga.h"
#include "gdt/gdt.h"
#include "interrupts/idt.h"



void kernel_main(void)
{
    reset();
    initGdt();
    print("--GDT loaded\n");
    print("--TSS loaded\n");
    idt_init();
    print("--IDT loaded\n");
    print(1/0);
    print("Herzlich willkommen bei senob!\n");
}
