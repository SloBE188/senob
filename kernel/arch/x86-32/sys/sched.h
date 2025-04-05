#ifndef SCHED_H
#define SCHED_H

#include "mp.h"
#include "process.h"

#define MAX_PRIORITY 5
#define MAX_RUNQUEUE_SIZE 64

extern uint64_t ticksSinceLastSwitch[MAX_CPUS];



struct runqueue 
{
    struct thread* queues[MAX_PRIORITY][MAX_RUNQUEUE_SIZE];
    int head[MAX_PRIORITY];
    int tail[MAX_PRIORITY];
};


void fillRunqueuesFromBST();
void enqueueThread(struct thread* t, uint32_t cpu); 
void enqueueRunnableThread(struct process* proc);
void schedule(uint32_t cpu);
void ageThreads(uint32_t cpu);

extern struct runqueue cpuRunqueues[MAX_CPUS];

#endif