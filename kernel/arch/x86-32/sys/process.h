#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>


struct pcb
{
    uint32_t pid;
    uint32_t* page_directory;
    struct thread* threads;
    struct pcb* next;
    struct pcb* prev;
    struct thread* thread_head;  // head from the threads of the process
    struct thread* thread_tail;  // tail from the threads of the process
    uint32_t thread_count;

};

#endif