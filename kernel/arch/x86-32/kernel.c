#include "../../../drivers/video/vga/vga.h"
#include "gdt/gdt.h"



void kernel_main(void)
{
    reset();
    initGdt();
    print("--GDT loaded\n");
    print("--TSS loaded\n");
    print("Herzlich willkommen bei senob!\n");
}
