#include "spinlock.h"
#include "startup.h"



void init_lock(struct spinlock* lock, char* name)
{
    lock->cpu = 0;
    lock->locked = lock;
    lock->name = name;
}

static inline uint32_t xchg(volatile uint32_t *addr, uint32_t newval)
{

  uint32_t result;

  // The + in "+m" denotes a read-modify-write operand.
  asm volatile("lock; xchgl %0, %1" :
               "+m" (*addr), "=a" (result) :
               "1" (newval) :
               "cc");
  return result;
}

void acquire(struct spinlock* lock)
{
    if(holding(lock))
        printf("cpu is already holding the lock, smth has gone wrong");

    asm volatile("cli");        //needed for preventing from deadlocks (sti in release)

    while(xchg(&lock->locked, 1) != 0)
        ;;

    lock->cpu = curr_cpu();


}

void release(struct spinlock* lock)
{
    if(!holding(lock))
        printf("cpu isnt even holding the lock, why are you trying to release it?");


    asm volatile("movl $0, %0" : "+m" (lock->locked) : );

    lock->cpu = 0;

    asm volatile("sti");
}


uint32_t holding(struct spinlock* lock)
{
    asm volatile("cli");

    uint32_t res;
    res = (lock->locked && lock->cpu == curr_cpu());
    
    return res;
}