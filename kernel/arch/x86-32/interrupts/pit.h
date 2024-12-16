#ifndef PIT_H
#define PIT_H
#include <stdint.h>


void init_pit();
void pit_wait(uint64_t ms);

#endif