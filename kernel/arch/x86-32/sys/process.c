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

void schedule() {
    if (current_thread == NULL || current_thread->state == TERMINATED) {
        // Suche den nächsten READY Thread
        struct pcb* pcb = pcb_head;
        struct thread* thread = NULL;

        while (pcb) {
            thread = pcb->thread_head;
            while (thread) {
                if (thread->state == READY) {
                    break;
                }
                thread = thread->next;
            }
            if (thread && thread->state == READY) {
                break;
            }
            pcb = pcb->next;
        }

        // Fallback zum Idle Thread, falls kein anderer READY Thread gefunden wurde
        if (thread == NULL) {
            current_pcb = pcb_head;  // Idle Process sollte immer an erster Stelle sein
            current_thread = current_pcb->thread_head;
        } else {
            current_thread = thread;
            current_pcb = current_thread->owner;
        }
    }

    if (current_thread != NULL) {
        current_thread->state = RUNNING;
        printf("Switching to Thread ID: %u of Process %u\n", current_thread->id, current_thread->owner->pid);
        context_switch(current_thread);
    }
}



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
    printf("hey, hier ist der idle thread");
    while (1) {
        asm volatile("hlt"); // CPU in den Ruhezustand versetzen
    }
}

void thread_exit() {
    printf("Thread %u is exiting\n", current_thread->id);
    current_thread->state = TERMINATED;
    schedule();
}
