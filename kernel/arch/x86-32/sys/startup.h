#ifndef STARTUP_H
#define STARTUP_H

#include <stdint.h>


void initialize_ap();
uint32_t get_local_apic_id_cpuid(void);

#endif