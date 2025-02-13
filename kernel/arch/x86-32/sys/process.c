/*
 * Copyright (C) 2024 Nils Burkard
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
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
#include "../../../libk/string.h"
#include "../gdt/gdt.h"
#include "../mm/paging/paging.h"
#include "../mm/heap/heap.h"
#include "../mm/PMM/pmm.h"
#include "../fatfs/ff.h"
#include "smp.h"
#include "startup.h"
#include "spinlock.h"
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "../../../libk/libk.h"
#include <stdlib.h>


#define NPROC 100

uint32_t current_processid = 0;
uint32_t current_threadid = 0;

struct process processes[NPROC];
extern struct cpu cpus[MAX_CPUS];

struct process *root = NULL;
struct process *NIL = NULL;

struct spinlock rb_tree_lock;
struct spinlock scheduler_lock;

uint32_t get_process_id()
{
    return current_processid++;
}

uint32_t get_thread_id()
{
    return current_threadid++;
}

// left rotation
void left_rotate(struct process *x)
{
    struct process *y = x->right;
    x->right = y->left;
    if (y->left != NIL)
    {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == NULL)
    {
        root = y;
    }
    else if (x == x->parent->left)
    {
        x->parent->left = y;
    }
    else
    {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

// right rotation
void right_rotate(struct process *y)
{
    struct process *x = y->left;
    y->left = x->right;
    if (x->right != NIL)
    {
        x->right->parent = y;
    }
    x->parent = y->parent;
    if (y->parent == NULL)
    {
        root = x;
    }
    else if (y == y->parent->left)
    {
        y->parent->left = x;
    }
    else
    {
        y->parent->right = x;
    }
    x->right = y;
    y->parent = x;
}

void inOrderTraversal(struct process *x)
{
    if (x != NIL)
    {
        inOrderTraversal(x->left);
        kernel_write("%d %s ", x->pid, (x->color == 0) ? "Black" : "Red");
        inOrderTraversal(x->right);
    }
}

void fixup_insert(struct process *new_node)
{
    // parent yes & RED
    while (new_node->parent != NULL && new_node->parent->color == RED)
    {

        if (new_node->parent == NULL || new_node->parent->parent == NULL)
            break; // no fix needed

        // is the parent node a left or right node (grandnodes)?
        if (new_node->parent == new_node->parent->parent->left)
        {
            struct process *y = new_node->parent->parent->right; // uncle of node
            if (y != NIL && y->color == RED)
            {
                // nodes uncle is red
                new_node->parent->color = BLACK;       // set parent black (else it would violate the properties of a rb)
                y->color = BLACK;                      // uncle black
                new_node->parent->parent->color = RED; // grandparents red
                new_node = new_node->parent->parent;   // move to grandparent
            }
            else // uncle is black
            {    // new_node is a right child
                if (new_node == new_node->parent->right)
                {
                    // case 2
                    new_node = new_node->parent;
                    left_rotate(new_node);
                }
                // case 3, new_node is a left child
                new_node->parent->color = BLACK;
                new_node->parent->parent->color = RED;
                right_rotate(new_node->parent->parent);
            }
            // parent node is a right node from its parents
        }
        else
        {
            struct process *y = new_node->parent->parent->left;
            if (y->color == RED)
            {
                new_node->parent->color = BLACK;
                y->color = BLACK;
                new_node->parent->parent->color = RED;
                new_node = new_node->parent->parent;
            }
            else
            {
                if (new_node == new_node->parent->left)
                {
                    new_node = new_node->parent;
                    right_rotate(new_node);
                }
                new_node->parent->color = BLACK;
                new_node->parent->parent->color = RED;
                left_rotate(new_node->parent->parent);
            }
        }
    }
    root->color = BLACK; // has to be black ALWAYS
}

void rb_insert(uint32_t pid)
{

    struct process *new_process = &processes[pid];
    kernel_write("inserting process with PID: %d\n", pid);

    struct process *y = NULL; // follows the parent node
    struct process *x = root;

    // normal bst insertion (as long as x ist null(end of the path), y to x(root))
    while (x != NIL)
    {
        y = x;
        if (new_process->pid < x->pid)
        {
            x = x->left;
        }
        else
        {
            x = x->right;
        }
    }

    new_process->parent = y;
    if (y == NULL)
    {
        root = new_process;
    }
    else if (new_process->pid < y->pid)
    {
        y->left = new_process;
    }
    else
    {
        y->right = new_process;
    }

    new_process->left = NIL;
    new_process->right = NIL;
    new_process->color = RED;

    // fix possible violations for rb tree
    fixup_insert(new_process);
}

struct process *rb_search(struct process *root, uint32_t pid)
{
    struct process *current = root;
    while (current != NIL && current->pid != pid)
    {
        if (pid < current->pid)
            current = current->left;
        else
            current = current->right;
    }
    return current;
}

struct process *rb_search_runnable(struct process *root)
{
    if (root == NIL)
        return NULL;

    // Suche im linken Teilbaum
    struct process *left_node = rb_search_runnable(root->left);
    if (left_node != NULL)
        return left_node;

    // Prüfe aktuellen Knoten
    if (root->state == RUNNABLE)
        return root;

    // Suche im rechten Teilbaum
    return rb_search_runnable(root->right);
}

struct rb_node *tree_minimum(struct process *x)
{
    while (x->left != NIL)
    {
        x = x->left;
    }
    return x;
}

void rb_transplant(struct process *u, struct process *v)
{
    // u gets replaced by  v as a parent
    if (u->parent == NULL)
    {
        root = v;
    }
    else if (u == u->parent->left)
    {
        u->parent->left = v;
    }
    else
    {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

void rb_delete_fixup(struct process *x)
{
    while (x != root && x->color == BLACK)
    {
        if (x == x->parent->left)
        {
            struct process *w = x->parent->right; // siblings from x
            // case 1: w is red
            if (w->color == RED)
            {
                w->color = BLACK;
                x->parent->color = RED;
                left_rotate(x->parent);
                w = x->parent->right;
            }
            // case 2: w is black and both kids are black
            if (w->left->color == BLACK && w->right->color == BLACK)
            {
                w->color = RED;
                x = x->parent;
            }
            else
            {
                // case 3: w is black, w->left is red, w->right is black
                if (w->right->color == BLACK)
                {
                    w->left->color = BLACK;
                    w->color = RED;
                    right_rotate(w);
                    w = x->parent->right;
                }
                // case 4: w->right is red
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                left_rotate(x->parent);
                x = root;
            }
        }
        else
        {
            // switch left and right up
            struct process *w = x->parent->left;
            if (w->color == RED)
            {
                w->color = BLACK;
                x->parent->color = RED;
                right_rotate(x->parent);
                w = x->parent->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK)
            {
                w->color = RED;
                x = x->parent;
            }
            else
            {
                if (w->left->color == BLACK)
                {
                    w->right->color = BLACK;
                    w->color = RED;
                    left_rotate(w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                right_rotate(x->parent);
                x = root;
            }
        }
    }
    x->color = BLACK;
}

void rb_delete(uint32_t pid)
{
    // find node to delete
    struct process *z = rb_search(root, pid);
    if (z == NIL)
    {
        return;
    }

    struct process *y = z;   // node to del or move
    struct process *x = NIL; // x is the child which will bereplacing y
    int y_original_color = y->color;

    // normal bst delete
    if (z->left == NIL)
    {
        // no left child
        x = z->right;
        rb_transplant(z, z->right);
    }
    else if (z->right == NIL)
    {
        // no right child
        x = z->left;
        rb_transplant(z, z->left);
    }
    else
    {
        // 2 children ->  successor(most left node in right subtree)
        y = tree_minimum(z->right);
        y_original_color = y->color;
        x = y->right; // x will replace y

        if (y->parent == z)
        {
            x->parent = y;
        }
        else
        {
            // attach y->right to y->parent
            rb_transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }

        // z gets replaced by y
        rb_transplant(z, y);
        y->left = z->left;
        y->left->parent = y;

        // y which just replaced z has to take over zs color
        y->color = z->color;
    }

    // kfree(z->proc);  if allocated
    // kfree(z);

    // if y was a black node,fixuop
    if (y_original_color == BLACK)
    {
        rb_delete_fixup(x);
    }
}

// returns needed pages
uint32_t map_program_to_address(const char *filename, uint32_t program_address)
{
    FRESULT res;
    FILINFO filestat;

    res = f_stat(filename, &filestat);
    if (res != FR_OK)
    {
        kernel_write("Error looking for the stats from the file: %d\n", res);
        return 0;
    }

    uint32_t file_size = filestat.fsize;
    uint32_t pages_needed = CEIL_DIV(file_size, PAGE_SIZE);

    for (uint32_t i = 0; i < pages_needed; i++)
    {
        void *vaddr = (void *)(program_address + (i * PAGE_SIZE));
        mem_map_page((uint32_t)vaddr, pmm_alloc_pageframe(), PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);
    }

    return pages_needed;
}

void copy_program_to_address(const char *filename, uint32_t pages_needed, uint32_t program_address)
{
    FIL program;
    FRESULT res;
    BYTE buffer[PAGE_SIZE];
    UINT bytes_read;

    res = f_open(&program, filename, FA_READ);
    if (res != FR_OK)
    {
        kernel_write("Error opening the file: %d\n", res);
        return;
    }

    // copy the program into the mapped adressspace for it
    for (uint32_t i = 0; i < pages_needed; i++)
    {
        res = f_read(&program, buffer, PAGE_SIZE, &bytes_read);
        if (res != FR_OK)
        {
            kernel_write("Error reading the file: %d\n", res);
            f_close(&program);
            return;
        }

        void *dest_address = (void *)(program_address + (i * PAGE_SIZE));
        memcpy(dest_address, buffer, bytes_read);
    }

    res = f_close(&program);
    if (res != FR_OK)
    {
        kernel_write("Error closing the file: %d\n", res);
    }
}

struct thread *create_kernel_thread(struct process *process, void (*start_function)());
struct thread *create_user_thread(struct process *process);

struct process *get_unused_process()
{
    for (uint32_t i = 0; i < 100; i++)
    {
        if (processes[i].state == UNUSED)
        {
            return &processes[i];
        }
    }
    kernel_write("No unused process found\n");
    return NULL;
}

struct process *get_runnable_process()
{
    for (uint32_t i = 0; i < 100; i++)
    {
        if (processes[i].state == RUNNABLE)
        {
            return &processes[i];
        }
    }
    kernel_write("No runnable process found\n");
    return NULL;
}

struct process *create_kernel_process(void (*start_function)())
{

    struct process *new_process = get_unused_process();
    if (!new_process)
    {
        kernel_write("No more unused process slots\n");
        return;
    }

    // struct process *new_process = (struct process *)kmalloc(sizeof(struct process));
    memset(new_process, 0x00, sizeof(struct process));
    kernel_write("Allocated process structure at: %p\n", new_process);

    new_process->pid = get_process_id();
    new_process->state = EMBRYO;
    new_process->assigned_cpu = -1;
    new_process->isuserproc = 0;

    new_process->page_directory = kernel_directory;


    new_process->head_thread = NULL;
    new_process->tail_thread = NULL;

    struct thread *main_thread = create_kernel_thread(new_process, start_function);

    new_process->head_thread = main_thread;

    //update_tss_esp0(main_thread->kstack.esp0, get_local_apic_id_cpuid() + 5);

    new_process->state = RUNNABLE;

    acquire(&rb_tree_lock);
    rb_insert(new_process->pid);
    release(&rb_tree_lock);

    return new_process;
}

struct process *create_process(const char *filename)
{
    struct process *new_process = get_unused_process();
    // struct process *new_process = (struct process *)kmalloc(sizeof(struct process));
    memset(new_process, 0x00, sizeof(struct process));
    kernel_write("Allocated process structure at: %p\n", new_process);


    new_process->pid = get_process_id();
    new_process->state = EMBRYO;
    new_process->assigned_cpu = -1;
    new_process->isuserproc = 1;

    new_process->page_directory = mem_alloc_page_dir();
    /* uint32_t *pd = new_process->page_directory;
     for (int i = 0; i < 1024; i++)
     {
         kernel_write("PDE[%d]: %x\n", i, pd[i]);
     }*/

    mem_change_page_directory(new_process->page_directory);


    uint32_t pages_needed = map_program_to_address(filename, 0x00400000);
    copy_program_to_address(filename, pages_needed, 0x00400000);

    new_process->head_thread = NULL;
    new_process->tail_thread = NULL;

    struct thread *main_thread = create_user_thread(new_process);

    new_process->head_thread = main_thread;

    update_tss_esp0(main_thread->kstack.esp0, get_local_apic_id_cpuid());       //if i switch to a process directly (test or debugging reasons) i have to uncomment the tss update

    new_process->state = RUNNABLE;

    mem_change_page_directory(kernel_directory);

    acquire(&rb_tree_lock);
    rb_insert(new_process->pid);
    release(&rb_tree_lock);

    return new_process;
}

struct thread *create_kernel_thread(struct process *process, void (*start_function)())
{
    struct thread *new_kthread = (struct thread *)kmalloc(sizeof(struct thread));
    kernel_write("kernel thread allocated at: %p\n", new_kthread);
    memset(new_kthread, 0x00, sizeof(struct thread));

    new_kthread->thread_id = get_thread_id();
    new_kthread->owner = process;

    // kernel stack
    new_kthread->kstack.ss0 = 0x10;
    uint8_t *kernel_stack = (uint8_t *)kmalloc(4096);
    new_kthread->kstack.esp0 = (uint32_t)kernel_stack + 4096 - 4;
    new_kthread->kstack.stack_start = (uint32_t)kernel_stack;

    uint32_t segment_selector = 0x10;
    new_kthread->regs.ss = segment_selector;
    new_kthread->regs.eflags = 0x202;
    new_kthread->regs.cs = 0x08;
    new_kthread->regs.eip = (uint32_t)start_function;
    new_kthread->regs.ds = segment_selector;
    new_kthread->regs.es = segment_selector;
    new_kthread->regs.fs = segment_selector;
    new_kthread->regs.gs = segment_selector;
    new_kthread->regs.esp = new_kthread->kstack.esp0;

    if (process->head_thread == NULL)
    {
        process->head_thread = new_kthread;
        process->tail_thread = new_kthread;

        new_kthread->prev = NULL;
        new_kthread->next = NULL;
    }
    else
    {
        new_kthread->prev = process->tail_thread;
        new_kthread->next = NULL;

        process->tail_thread->next = new_kthread;
        process->tail_thread = new_kthread;
    }

    return new_kthread;
}

struct thread *create_user_thread(struct process *process)
{
    struct thread *new_thread = (struct thread *)kmalloc(sizeof(struct thread));
    memset(new_thread, 0x00, sizeof(struct thread));
    kernel_write("Allocated thread structure at: %p\n", new_thread);

    new_thread->thread_id = get_thread_id();
    new_thread->owner = process;

    uint32_t count_stack_pages = 40;

    // untere grenze vom stack
    void *end_of_stack = (void *)(USER_STACK_TOP - (PAGE_SIZE * count_stack_pages));

    uint32_t physical_frames[count_stack_pages];
    for (uint32_t i = 0; i < count_stack_pages; i++)
    {
        void *vaddr = (void *)((uint32_t)end_of_stack + (i * PAGE_SIZE));
        physical_frames[i] = pmm_alloc_pageframe();
        mem_map_page((uint32_t)vaddr, physical_frames[i], PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);
    }

    /*for (uint32_t addr = (USER_STACK_TOP - (PAGE_SIZE * count_stack_pages)); addr < USER_STACK_TOP; addr += PAGE_SIZE)
    {
        uint32_t phys = mem_get_phys_from_virt(addr);
        if (phys == (uint32_t)-1)
        {
            kernel_write("address 0x%x is not mapped\n", addr);
        }
        else
        {
            kernel_write("address 0x%x is mapped to physical address 0x%x\n", addr, phys);
        }
    }*/

    mem_map_page(0xb0000000, pmm_alloc_pageframe(), PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);

    if (!mem_is_valid_vaddr(0xb0000000))
    {
        kernel_write("address 0xb0000000 is not mapped\n");
    }

    uint32_t segment_selector = 0x23;
    new_thread->regs.ss = segment_selector;
    new_thread->regs.eflags = 0x202;
    new_thread->regs.cs = 0x1B;
    new_thread->regs.eip = PROGRAMM_VIRTUAL_ADDRESS_START;
    new_thread->regs.ds = segment_selector;
    new_thread->regs.es = segment_selector;
    new_thread->regs.fs = segment_selector;
    new_thread->regs.gs = segment_selector;
    uint32_t user_stack_pointer = USER_STACK_TOP - 4;
    new_thread->regs.esp = user_stack_pointer;

    // kernel stack
    new_thread->kstack.ss0 = 0x10;
    uint8_t *kernel_stack = (uint8_t *)kmalloc(4096);
    new_thread->kstack.esp0 = (uint32_t)kernel_stack + 4096 - 4;
    new_thread->kstack.stack_start = (uint32_t)kernel_stack;

    if (process->head_thread == NULL)
    {
        process->head_thread = new_thread;
        process->tail_thread = new_thread;

        new_thread->prev = NULL;
        new_thread->next = NULL;
    }
    else
    {
        new_thread->prev = process->tail_thread;
        new_thread->next = NULL;

        process->tail_thread->next = new_thread;
        process->tail_thread = new_thread;
    }

    return new_thread;
}

struct registers_save *save_thread_state(struct thread *thread)
{
    struct registers_save *registers = (struct registers_save *)kmalloc(sizeof(struct registers_save));

    registers->eax = thread->regs.eax;
    registers->ebx = thread->regs.ebx;
    registers->ecx = thread->regs.ecx;
    registers->edx = thread->regs.edx;

    registers->esp = thread->regs.esp;
    registers->ebp = thread->regs.ebp;

    registers->edi = thread->regs.edi;
    registers->eip = thread->regs.eip;
    registers->eflags = thread->regs.eflags;

    registers->cs = thread->regs.cs;
    registers->ss = thread->regs.ss;
    registers->ds = thread->regs.ds;
    registers->es = thread->regs.es;
    registers->fs = thread->regs.fs;
    registers->gs = thread->regs.gs;

    registers->cr3 = thread->regs.cr3;

    return registers;
}

void switch_to_thread(struct thread *thread)
{
    struct registers_save *registers = save_thread_state(thread);
    switch_task(registers);
}

uint32_t get_curr_pid()
{
    uint32_t cpu_nr = get_local_apic_id_cpuid();
    struct cpu* curr_cpu = &cpus[cpu_nr];
    return curr_cpu->proc->pid;
}

void process_exit(uint32_t pid)
{
    struct process* proc = rb_search(root, pid);
    kfree(proc->head_thread);
    rb_delete(pid);
    scheduler();
}

void node_preparation()
{
    // Initialize NIL node
    NIL = (struct process *)kmalloc(sizeof(struct process));
    NIL->color = BLACK; // NIL has to be black
    NIL->left = NIL;
    NIL->right = NIL;
    NIL->parent = NULL;
    root = NIL;

    for (uint32_t i = 0; i < NPROC; i++)
    {
        processes[i].color = BLACK;
        processes[i].left = NIL;
        processes[i].right = NIL;
        processes[i].parent = NULL;
        processes[i].state = UNUSED;
        processes[i].pid = i;
    }
}



static void context_switch(struct thread *new_thread);

void scheduler(void) 
{
    struct cpu *cpu = &cpus[get_local_apic_id_cpuid()];
    cpu->proc = NULL;

    while (1) 
    {
        acquire(&scheduler_lock);

        if(cpu->proc != NULL)
        {
        struct process *current_process = cpu->proc;
            if (current_process && current_process->state == RUNNING) 
            {
                current_process->state = SLEEPING;
            }
        }

        struct process *next_process = rb_search_runnable(root);
        if (!next_process) 
        {
            release(&scheduler_lock);
            continue;
        }

        struct thread *next_thread = next_process->head_thread;
        if (!next_thread) 
        {
            kernel_write("No threads in runnable process PID %d.\n", next_process->pid);
            release(&scheduler_lock);
            continue;
        }

        next_process->state = RUNNING;
        cpu->proc = next_process;

        release(&scheduler_lock);

        context_switch(next_thread);
    }
}

static void context_switch(struct thread *new_thread) 
{
    struct process* new_process = new_thread->owner;
    struct cpu* cpu = &cpus[get_local_apic_id_cpuid()];
    struct thread* old_thread = cpu->proc ? cpu->proc->head_thread : NULL;


    if (new_process->assigned_cpu == -1)
    {
        new_process->assigned_cpu = cpu->id;
    }

    update_tss_esp0(new_thread->kstack.esp0, cpu->id);
    if (new_process->isuserproc == 1)
    {
        mem_change_page_directory(new_process->page_directory);
    }
    
    

    kernel_write("switching context from old thread to TID %d\n", new_thread->thread_id);

    if(old_thread) 
    {
        save_thread_state(old_thread);
    }

    switch_to_thread(new_thread);
}



void init_locks()
{
    init_lock(&rb_tree_lock, "rb_tree_lock");
    init_lock(&scheduler_lock, "scheduler_lock");
}

void thethirdone()
{
    clear_screen_sys_2(COLOR_RED);
    kernel_write("Hey, here is thethirdone :D\n");
    PitWait(5000);
    process_exit(get_curr_pid());
}

void anotherone()
{
    clear_screen_sys_2(COLOR_BLUE);
    kernel_write("Hey, here is anotherone :D\n");
    while (1)
    {
    }
}

void test_process()
{

    clear_screen_sys_2(COLOR_GREEN);
    kernel_write("HEYY, ICH BIN EIN KERNEL PROZESS!!!\n");
    // struct process *p1 = create_process("0:/test.bin");
    //  inOrderTraversal(root);
    //  PitWait(8000);
    //  switch_to_thread(p1->head_thread);
    /*
   FILE *fp;
   const char *str = "Hello, World!";

   fp = fopen("test.txt", "w");
   if (fp == NULL) {
      perror("cant open fire for writing");
      return 1;
   }

   size_t written = fwrite(str, sizeof(char), strlen(str), fp);
   printf("nr written elems: %zu\n", written);

   fclose(fp);


    fp = fopen("test.txt", "r");
    if (fp == NULL) {
        perror("cant open file for reading");
        return 1;
    }

    char buffer[256];
    size_t read = fread(buffer, sizeof(char), sizeof(buffer) - 1, fp);
    buffer[read] = '\0';

    printf("content of test.txt: %s\n", buffer);

    fclose(fp);

    */
    struct stat stats;

    int fd = open("0:/qemu_log.txt", O_RDWR);
    printf("fd: %d\n", fd);

    fstat(fd, &stats);
    
    printf("file size: 0x%x\n", stats.st_size);

    close(fd);
   
    

    int* p = (int*)malloc(4000);
    kernel_write("Address: %p\n", p);
    if (p != NULL)
    {
        kernel_write("malloc seems to work\n");
    }

    
    int* p1 = (int*)malloc(300);
    kernel_write("Address: %p\n", p1);
    

    
    
    
    while(1){}
}

uint32_t init_proc()
{

    init_locks();
    node_preparation();
    
    kernel_write("Creating kernel process 1\n");
    struct process *k1 = create_kernel_process(&test_process);
    kernel_write("Creating kernel process 2\n");
    struct process *k2 = create_kernel_process(&anotherone);
    kernel_write("Creating kernel process 3\n");
    struct process *k3 = create_kernel_process(&thethirdone);
    kernel_write("Creating a uuser process (1)\n");
    struct process *u1 = create_process("0:/test.bin");

    
    kernel_write("In-order traversal of RB Tree:\n");
    inOrderTraversal(root);

    kernel_write("\n");

    //mem_change_page_directory(u1->page_directory);
    //switch_to_thread(u1->head_thread);

    switch_to_thread(k1->head_thread);


    return 0;
}
