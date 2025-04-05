#include "sched.h"
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include "../../../../drivers/video/vbe/vbe.h"

#define AGE_THRESHOLD 50

struct runqueue cpuRunqueues[MAX_CPUS];

uint32_t threadsPerCPU[MAX_CPUS] = {0};

struct thread* currentThread[MAX_CPUS]= {0};
uint64_t ticksSinceLastSwitch[MAX_CPUS] = {0};


//Fills the runqueue of a cpu with a thread.
//@t = thread which should get added to the runqueue.
//@ cpu = cpu, in which runqueue the thread sould get in.
void enqueueThread(struct thread* t, uint32_t cpu) 
{
    uint32_t prio = t->dynamicPriority;
    if (prio >= MAX_PRIORITY) prio = MAX_PRIORITY - 1;

    struct runqueue* rq = &cpuRunqueues[cpu];

    if (rq->tail[prio] >= MAX_RUNQUEUE_SIZE) 
    {
        printf("Runqueue full at CPU %u, prio %u!\n", cpu, prio);
        return;
    }

    rq->queues[prio][rq->tail[prio]++] = t;
    threadsPerCPU[cpu]++;
}

//This function searches a thead in the runqueues for the cpu with the highest priority.
//@cpu = the APIC-ID from the cpu to get the runqueues from.
struct thread* dequeueThread(uint32_t cpu)
{
    struct runqueue* rq = &cpuRunqueues[cpu];
    for (int prio = 0; prio < MAX_PRIORITY; prio++) 
    {
        if (rq->head[prio] < rq->tail[prio]) 
        {
            struct thread* t = rq->queues[prio][rq->head[prio]++];
            threadsPerCPU[cpu]--;
            return t;
        }
    }
    return NULL;
}

//Increases the priority of a thread.
//@t = thread.
void demotePriority(struct thread* t)
{
    if (t->dynamicPriority < MAX_PRIORITY - 1)
        t->dynamicPriority++;
}


//Searches the cpu with the least amount of threads.
uint32_t leastLoadedCPU()
{
    uint32_t min = threadsPerCPU[0];
    uint32_t idx = 0;
    for (uint32_t i = 0; i < ncpus; i++)
    {
        if (threadsPerCPU[i] < min)
        {
            min = threadsPerCPU[i];
            idx = i;
        }
    }
    return idx;
    
}

//Iterates threw the double linked list of the given process, calls leastLoadedCPU and enqueues every found thread from the given process.
//@proc = process.
void enqueueRunnableThread(struct process* proc)
{
    struct thread* thread = proc->head_thread;
    while (thread)
    {
        if(thread->state == RUNNABLE)
        {
            uint32_t cpu = leastLoadedCPU();
            enqueueThread(thread, cpu);
            printf("Enqueue thread %p to CPU %u (prio %u)\n", thread, cpu, thread->dynamicPriority);

        }
        thread = thread->next;
    }
    
}

//Fills runqueues from the RB-BST.
void fillRunqueuesFromBST()
{
    inOrderTraversal(root, enqueueRunnableThread);
}

//The priority increases if the process is aging.
//@cpu = the APIC-ID from the cpu which calls this funtion.
void ageThreads(uint32_t cpu)
{
    struct runqueue* rq = &cpuRunqueues[cpu];

    for (int prio = MAX_PRIORITY - 1; prio > 0; prio--) 
    {
        for (int i = rq->head[prio]; i < rq->tail[prio]; i++) 
        {
            struct thread* t = rq->queues[prio][i];
            if (t->state == RUNNABLE && t->waitTicks > AGE_THRESHOLD) 
            {
                t->waitTicks = 0;
                t->dynamicPriority--; //priority increases
            } else {
                t->waitTicks++;
            }
        }
    }
}


//The main function of the scheduler.
//@cpu = the APIC-ID from the cpu which calls this funtion.
void schedule(uint32_t cpu)
{
    //idle process/thread
    struct process* idleProcess = rbSearch(initIdle());
    
    struct thread* current = currentThread[cpu];
    
    //handling for the old running thread
    if (current && current->state == RUNNING)
    {
        current->state = RUNNABLE;
        demotePriority(current);
        enqueueThread(current, cpu);
    }

    struct thread* nextThread = dequeueThread(cpu);

    if (!nextThread)
    {
        //printf("No runnable thread anymore, idle\n");
        nextThread = idleProcess->head_thread;
    }
    
    nextThread->waitTicks = 0;
    nextThread->state = RUNNING;
    currentThread[cpu] = nextThread;
    ticksSinceLastSwitch[cpu] = 0;

    updateTssEsp0(nextThread->kstack.esp0, cpu);
    switchTask(nextThread->regs);    
    
}