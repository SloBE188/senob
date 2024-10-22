#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "thread.h"


struct pcb
{
    uint32_t pid;
    uint32_t* page_directory;
    struct pcb* next;
    struct pcb* prev;
    struct thread* thread_head;  // head from the threads of the process
    struct thread* thread_tail;  // tail from the threads of the process
    uint32_t thread_count;

};

void init_processes(struct pcb* process);
void add_process(struct pcb* new_process);
void add_thread_to_process(struct pcb* process, struct thread* new_thread);
void schedule();
void idle_thread();
void thread_exit();


#endif