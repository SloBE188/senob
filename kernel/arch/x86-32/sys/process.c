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
#include "thread.h"
#include "../kernel.h"


struct pcb* pcb_head = NULL;
struct pcb* pcb_tail = NULL;
struct pcb* current_pcb = NULL;
uint32_t process_id = 0;


void init_processes(struct pcb* process)
{   
    memset(process, 0x00, sizeof(struct pcb));
    process->threads = NULL;
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
    new_process->threads = NULL;
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
