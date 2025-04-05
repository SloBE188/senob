#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>

struct spinlock
{
    uint32_t locked;
    struct cpu* cpu;
    char* name;
};

void initLock(struct spinlock* lock, char* name);
void acquire(struct spinlock* lock);
void release(struct spinlock* lock);


#endif