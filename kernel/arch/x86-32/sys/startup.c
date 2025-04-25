#include "startup.h"
#include "../gdt/gdt.h"
#include "../interrupts/idt.h"
#include <stdint.h>
#include "smp.h"
#include "../mm/paging/paging.h"
#include "lapic.h"
#include "sched.h"
#include "process.h"
#include "../kernel.h"

extern void idt_flush(uint32_t);
extern void gdt_flush(uint32_t);
extern struct idtr_t idtr;
extern struct gdt_ptr_struct gdt_ptr;


//Loads TSS.
//@tssSelector = selector of the TSS which should get loader.
static inline void loadTSS(uint16_t tssSelector)
{
    asm volatile("ltr %0" : : "r"(tssSelector));
}

//This function gets called from the trampoline code of every cpu and fully initializes the cpu.
void initializeAP()
{
    //mem_change_page_directory(kernel_directory);    //Wouldnt be necessary, just to make sure

    //sync_page_dirs();
    uint32_t lapicID = get_local_apic_id_cpuid();
    uint32_t apTSSSelector = (5 + lapicID) << 3;

    gdt_flush((uint32_t)&gdt_ptr);  //Load global GDT
    loadTSS(apTSSSelector); //Load TSS
    idt_flush((uint32_t)&idtr);

    lapicInit();        //Didnt know that one, a APIC can recieve IPIs even if shes not activated yet 

    lapicTimerInit();


    while(1){}
}