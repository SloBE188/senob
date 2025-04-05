#ifndef LAPIC_H
#define LAPIC_H

#include <stdint.h>

void lapicInit();
void apStartup(uint32_t apicID, uint32_t trampolineAddr);
void lapicWrite(uint32_t offset, uint32_t value);
void lapicTimerInit();
void eoi();

#endif