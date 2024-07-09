#include "idt.h"
#include "../../../libk/memset.h"
#include"io/io.h"

struct idt_entry_t idt_descriptors[256];
struct idtr_t idtr;


void idt_set()
{

}

//Unitializes the Master & Slave PIC
void init_pic()
{
    //0x20 = Master PIC, 0xA0 = Slave PIC
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

void idt_init()
{
    memset(&idt_descriptors, 0, sizeof(struct idt_entry_t) * 256);
    idtr.limit = sizeof(struct idt_entry_t) * 256 - 1;
    idtr.base = (uint32_t)&idt_descriptors[0];

    init_pic();

}