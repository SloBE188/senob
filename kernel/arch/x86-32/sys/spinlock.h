#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>
#include "smp.h"

struct spinlock
{
    uint32_t locked;
    struct cpu* cpu;
    char* name;
};

void init_lock(struct spinlock* lock, char* name);


#endif