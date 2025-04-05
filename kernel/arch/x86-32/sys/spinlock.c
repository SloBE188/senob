#include "spinlock.h"


//Initializes a lock.
//@lock = the lock structure.
//@name = the name of the lock (ASCII string).
void initLock(struct spinlock* lock, char* name)
{
    lock->cpu = 0;
    lock->locked = 0;
    lock->name = name;
}

//Function which operate with the atomic operation from the cpu (xchgl).
//@addr = address.
//@newval = value of the lock (0 or 1).
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

uint32_t holding(struct spinlock* lock);

//Asks if the lock is free and if it is it locks it.
//@lock = structure of the lock.
void acquire(struct spinlock* lock)
{
    if(holding(lock))
        kernel_write("cpu is already holding the lock, smth has gone wrong");

    asm volatile("cli");        //needed for preventing from deadlocks (sti in release)

    while(xchg(&lock->locked, 1) != 0)
        ;;

    __sync_synchronize();

    lock->cpu = get_local_apic_id_cpuid();

}

//Releases a lock.
//@lock = lock structure.
void release(struct spinlock* lock)
{
    if(!holding(lock))
        kernel_write("cpu isnt even holding the lock, why are you trying to release it?");

    __sync_synchronize();

    asm volatile("movl $0, %0" : "+m" (lock->locked) : );

    lock->cpu = 0;

    asm volatile("sti");
}

//Holds the lock.
//@lock = lock structure.
uint32_t holding(struct spinlock* lock)
{
    asm volatile("cli");

    uint32_t res;
    res = lock->locked && lock->cpu == get_local_apic_id_cpuid();
    
    return res;
}