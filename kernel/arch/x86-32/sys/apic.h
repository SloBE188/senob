#ifndef APIC_H
#define APIC_H

#define EOI (0x00B0 / 4)   // EOI Reg
#define ESR (0x0280 / 4)

void lapic_init(void);
void ap_startup(uint32_t APIC_ID, uint32_t trampoline_addr);
void check_lapic_status();
void lapicw(int offset, int value);
uint32_t lapic_read(int offset);
void check_apic_base_address();
void lapic_delay(uint32_t ticks);

#endif