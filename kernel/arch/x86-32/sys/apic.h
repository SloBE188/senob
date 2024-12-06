#ifndef APIC_H
#define APIC_H


void lapic_init(void);
void ap_startup(uint32_t APIC_ID, uint32_t trampoline_addr);
void check_lapic_status();


#endif