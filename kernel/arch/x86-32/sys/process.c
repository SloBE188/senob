/*
 * Copyright (C) 2024 Nils Burkard
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "process.h"
#include "../../../libk/stdiok.h"
#include "../kernel.h"
#include "../../../libk/memory.h"
#include "../gdt/gdt.h"


struct pcb* pcb_head = NULL;
struct pcb* pcb_tail = NULL;
struct pcb* current_pcb = NULL;
struct thread* current_thread = NULL;

uint32_t process_id = 0;


void init_processes(struct pcb* process)
{   
    memset(process, 0x00, sizeof(struct pcb));
    process->next = NULL;
    process->prev = NULL;
    process->thread_count = 0;
    process->pid = ++process_id;
    pcb_head = process;
    pcb_tail = process;
    current_pcb = process;

}

void add_process(struct pcb* new_process)
{
    memset(new_process, 0x00, sizeof(struct pcb));
    new_process->next = NULL;
    new_process->prev = pcb_tail;
    new_process->pid = ++process_id;
    if (pcb_tail) {
        pcb_tail->next = new_process;
    }
    pcb_tail = new_process;
}

void add_thread_to_process(struct pcb* process, struct thread* new_thread)
{
    if (process->thread_head == NULL) {
        // if there are no threads in the process it will be the first one
        process->thread_head = new_thread;
        process->thread_tail = new_thread;
        new_thread->next = NULL;
        new_thread->prev = NULL;
    } else {
        // insert the new thread to the thread list of the process
        struct thread* tail = process->thread_tail;
        tail->next = new_thread;
        new_thread->prev = tail;
        new_thread->next = NULL;
        process->thread_tail = new_thread;
    }
    process->thread_count++;
}

struct pcb* create_process(uint32_t* page_directory, struct thread* first_thread)
{
    struct pcb* new_process = (struct pcb*) kmalloc(sizeof(struct pcb));
    add_process(new_process);
    add_thread_to_process(new_process, first_thread);

    new_process->page_directory = page_directory;

    return new_process;

}

/*void schedule() {
    struct pcb* pcb = pcb_head;
    struct thread* next_thread = NULL;

    // searching for a thread which is READY
    while (pcb) {
        struct thread* thread = pcb->thread_head;
        while (thread) {
            if (thread->state == READY) {
                next_thread = thread;
                break;
            }
            thread = thread->next;
        }
        if (next_thread) {
            break;
        }
        pcb = pcb->next;
    }

    // if there is no READY thread in any process, use the idle thread
    if (next_thread == NULL) {
        pcb = pcb_head;  // idle process should always be the first one (head)
        next_thread = pcb->thread_head;  // choose idle thread
    }

    if (next_thread != NULL) {
        if (next_thread->state == IDLET) {
            // the idle thread keeps his special state IDLET
            printf("Switching to Idle Thread\n");
        } else {
            next_thread->state = RUNNING; // set the next thread which it will switch to to RUNNING
            printf("Switching to Thread ID: %u of Process %u\n", next_thread->id, next_thread->owner->pid);
        }
        context_switch(next_thread);
    }
}*/

void context_switch(struct thread* next_thread)
{
    if (next_thread == NULL || next_thread->state == TERMINATED) {
        printf("Next thread is NULL, unable to perform context switch\n");
        return;
    }

    // if i switch from the current thread, save his state
    if (current_thread != NULL) {
        asm volatile (
            "mov %%esp, %0\n"
            "mov %%ebp, %1\n"
            : "=m" (current_thread->regs.esp), "=m" (current_thread->regs.ebp)
        );
    }

    // switch to the next thread
    current_thread = next_thread;

    // load the stack from the new thread
    asm volatile (
        "mov %0, %%esp\n"
        "mov %1, %%ebp\n"
        : 
        : "m" (current_thread->regs.esp), "m" (current_thread->regs.ebp)
    );

    // jump to the next instruction (from the thread EIP)
    asm volatile (
        "jmp *%0\n"
        : 
        : "m" (current_thread->regs.eip)
    );
}










/*
    asm volatile (
        "assembly code"            // der eigentliche Assembly-Code
        : output operands           // Ausgabewerte -> werte werden vom asm code in eine c variable gespeichert
        : input operands            // Eingabewerte -> wert einer c variable wird in den asm code geladen
        : clobbered registers       // verwendete Register (optional)
    );
*/

void idle_thread() 
{
    printf("hey, hier ist der idle thread\n");
    while (1) {
        asm volatile("hlt");
    }
}

void thread_exit() 
{
    printf("Thread %u is exiting\n", current_thread->id);
    current_thread->state = TERMINATED;
    //schedule();
}


void proc_enter_usermode()
{
    if (!current_pcb)
    {
        return;
    }

    update_tss_esp0(current_pcb->thread_head->kernel_stack + KERNEL_STACK_SIZE);


    asm volatile (
        "cli\n"
        "mov $0x23, %%eax\n"
        "mov %%eax, %%ds\n"
        "mov %%eax, %%es\n"
        "mov %%eax, %%fs\n"
        "mov %%eax, %%gs\n"

        "push %%eax\n"       // %ss
        "mov %0, %%eax\n"
        "push %%eax\n"       // %esp
        "push $0x202\n"      // %eflags with IF set
        "push $0x1B\n"       // %cs
        "mov %1, %%eax\n"
        "push %%eax\n" // %eip
        "iret\n"
        :
        : "r" (current_pcb->thread_head->regs.esp), "r" (current_pcb->thread_head->regs.eip)
        : "%eax");
}
