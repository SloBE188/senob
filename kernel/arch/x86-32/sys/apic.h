#ifndef APIC_H
#define APIC_H

#define EOI (0x00B0 / 4)   // EOI Reg

void lapic_init(void);
void ap_startup(uint32_t APIC_ID, uint32_t trampoline_addr);
void check_lapic_status();
void lapicw(int offset, int value);
uint32_t lapic_read(int offset);


#endif