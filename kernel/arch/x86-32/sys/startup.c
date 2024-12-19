#include "startup.h"
#include "../gdt/gdt.h"
#include "../interrupts/idt.h"

extern struct idtr_t idtr;
extern struct gdt_ptr_struct gdt_ptr;
extern uint32_t kernel_directory[1024];

void initialize_ap()
{
    
}