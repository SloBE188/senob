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

uint32_t current_processid = 0;
uint32_t current_threadid = 0;

struct process processes[100];
extern struct cpu cpus[MAX_CPUS];

struct rb_node *root = NULL;
struct rb_node *NIL = NULL;

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
void left_rotate(struct rb_node *x)
{
    struct rb_node *y = x->right;
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
void right_rotate(struct rb_node *y)
{
    struct rb_node *x = y->left;
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

void inOrderTraversal(struct rb_node *x)
{
    if (x != NIL)
    {
        inOrderTraversal(x->left);
        printf("%d %s ", x->proc->pid, (x->color == 0) ? "Black" : "Red");
        inOrderTraversal(x->right);
    }
}

void fixup_insert(struct rb_node *new_node)
{
    // parent yes & RED
    while (new_node->parent != NULL && new_node->parent->color == RED)
    {

        // is the parent node a left or right node (grandnodes)?
        if (new_node->parent == new_node->parent->parent->left)
        {
            struct rb_node *y = new_node->parent->parent->right; // uncle of node
            if (y->color == RED)
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
            struct rb_node *y = new_node->parent->parent->left;
            if (y->color = RED)
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
    struct rb_node *new_node = (struct rb_node *)kmalloc(sizeof(struct rb_node));
    printf("Node allocated at: %p\t", new_node);

    // evt. proc alloc
    struct process *new_process = &processes[pid];
    printf("Process allocated at: %p\n", new_process);
    new_node->proc = new_process;

    new_node->proc->pid = pid;

    struct rb_node *y = NULL; // follows the parent node
    struct rb_node *x = root;

    // normal bst insertion (as long as x ist null(end of the path), y to x(root))
    while (x != NIL)
    {
        y = x;
        if (new_node->proc->pid < x->proc->pid)
        {
            x = x->left;
        }
        else
        {
            x = x->right;
        }
    }

    new_node->parent = y;
    if (y == NULL)
    {
        root = new_node;
    }
    else if (new_node->proc->pid < y->proc->pid)
    {
        y->left = new_node;
    }
    else
    {
        y->right = new_node;
    }

    new_node->left = NIL;
    new_node->right = NIL;
    new_node->color = RED;

    // fix possible violations for rb tree
    fixup_insert(new_node);
}

struct rb_node *rb_search(struct rb_node *root, uint32_t pid)
{
    struct rb_node *current = root;
    while (current != NIL && current->proc->pid != pid)
    {
        if (pid < current->proc->pid)
            current = current->left;
        else
            current = current->right;
    }
    return current;
}

struct rb_node *rb_search_runnable(struct rb_node *root)
{
    if (root == NIL)
        return;

    // search left subtree
    struct rb_node *left_node = rb_search_runnable(root->left);
    if (left_node != NIL)
        return left_node;

    if (root->proc->state == RUNNABLE)
        return root;

    // search right
    return rb_search_runnable(root->right);
}

struct rb_node *tree_minimum(struct rb_node *x)
{
    while (x->left != NIL)
    {
        x = x->left;
    }
    return x;
}

void rb_transplant(struct rb_node *u, struct rb_node *v)
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

void rb_delete_fixup(struct rb_node *x)
{
    while (x != root && x->color == BLACK)
    {
        if (x == x->parent->left)
        {
            struct rb_node *w = x->parent->right; // siblings from x
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
            struct rb_node *w = x->parent->left;
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
    struct rb_node *z = rb_search(root, pid);
    if (z == NIL)
    {
        return;
    }

    struct rb_node *y = z;   // node to del or move
    struct rb_node *x = NIL; // x is the child which will bereplacing y
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
        printf("Error looking for the stats from the file: %d\n", res);
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
        printf("Error opening the file: %d\n", res);
        return;
    }

    // copy the program into the mapped adressspace for it
    for (uint32_t i = 0; i < pages_needed; i++)
    {
        res = f_read(&program, buffer, PAGE_SIZE, &bytes_read);
        if (res != FR_OK)
        {
            printf("Error reading the file: %d\n", res);
            f_close(&program);
            return;
        }

        void *dest_address = (void *)(program_address + (i * PAGE_SIZE));
        memcpy(dest_address, buffer, bytes_read);
    }

    res = f_close(&program);
    if (res != FR_OK)
    {
        printf("Error closing the file: %d\n", res);
    }
}

void rb_insert_process(struct process *p)
{
    // new node for rb tree
    struct rb_node *new_node = (struct rb_node *)kmalloc(sizeof(struct rb_node));
    memset(new_node, 0, sizeof(struct rb_node));
    printf("RB node allocated at: %p\n", new_node);

    new_node->proc = p;
    // p->pid already set in create_process()
    // new_node->proc->pid = p->pid;

    // normal bst insertion (as long as x ist null(end of the path), y to x(root))
    struct rb_node *y = NULL;
    struct rb_node *x = root;

    while (x != NIL)
    {
        y = x;
        if (p->pid < x->proc->pid)
            x = x->left;
        else
            x = x->right;
    }

    new_node->parent = y;
    if (y == NULL)
    {
        root = new_node;
    }
    else if (p->pid < y->proc->pid)
    {
        y->left = new_node;
    }
    else
    {
        y->right = new_node;
    }

    new_node->left = NIL;
    new_node->right = NIL;
    new_node->color = RED;

    fixup_insert(new_node);
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
    printf("No unused process found\n");
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
    printf("No runnable process found\n");
    return NULL;
}

struct process *create_kernel_process(void (*start_function)())
{

    struct process *new_process = get_unused_process();
    if (!new_process)
    {
        printf("No more unused process slots\n");
        return;
    }
    
    // struct process *new_process = (struct process *)kmalloc(sizeof(struct process));
    memset(new_process, 0x00, sizeof(struct process));
    printf("Allocated process structure at: %p\n", new_process);
    new_process->state = EMBRYO;

    new_process->page_directory = kernel_directory;

    //new_process->pid = get_process_id();

    new_process->head_thread = NULL;
    new_process->tail_thread = NULL;

    struct thread *main_thread = create_kernel_thread(new_process, start_function);

    new_process->head_thread = main_thread;

    update_tss_esp0(main_thread->kstack.esp0, 6);

    new_process->state = RUNNABLE;

    // acquire(&rb_tree_lock);
    // rb_insert_process(new_process);
    // release(&rb_tree_lock);

    return new_process;
}

struct process *create_process(const char *filename)
{
    struct process *new_process = get_unused_process();
    // struct process *new_process = (struct process *)kmalloc(sizeof(struct process));
    memset(new_process, 0x00, sizeof(struct process));
    printf("Allocated process structure at: %p\n", new_process);

    new_process->state = EMBRYO;

    new_process->page_directory = mem_alloc_page_dir();
    /* uint32_t *pd = new_process->page_directory;
     for (int i = 0; i < 1024; i++)
     {
         printf("PDE[%d]: %x\n", i, pd[i]);
     }*/

    mem_change_page_directory(new_process->page_directory);

    new_process->pid = get_process_id();

    uint32_t pages_needed = map_program_to_address(filename, 0x00400000);
    copy_program_to_address(filename, pages_needed, 0x00400000);

    new_process->head_thread = NULL;
    new_process->tail_thread = NULL;

    struct thread *main_thread = create_user_thread(new_process);

    new_process->head_thread = main_thread;

    update_tss_esp0(main_thread->kstack.esp0, 6);

    new_process->state = RUNNABLE;

    // acquire(&rb_tree_lock);
    // rb_insert_process(new_process);
    // release(&rb_tree_lock);

    return new_process;
}

struct thread *create_kernel_thread(struct process *process, void (*start_function)())
{
    struct thread *new_kthread = (struct thread *)kmalloc(sizeof(struct thread));
    printf("kernel thread allocated at: %p\n", new_kthread);
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
    printf("Allocated thread structure at: %p\n", new_thread);

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
            printf("address 0x%x is not mapped\n", addr);
        }
        else
        {
            printf("address 0x%x is mapped to physical address 0x%x\n", addr, phys);
        }
    }*/

    mem_map_page(0xb0000000, pmm_alloc_pageframe(), PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);

    if (!mem_is_valid_vaddr(0xb0000000))
    {
        printf("address 0xb0000000 is not mapped\n");
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

void node_preparation()
{
    // Initialize NIL node
    NIL = (struct rb_node *)kmalloc(sizeof(struct rb_node));
    NIL->color = BLACK; // NIL has to be black
    NIL->left = NIL;
    NIL->right = NIL;
    NIL->parent = NIL;
    root = NIL;
}

void test_process()
{

    clear_screen_sys_2(COLOR_GREEN);
    printf("HEYY, ICH BIN EIN KERNEL PROZESS!!!\n");
    //struct process *p1 = create_process("0:/test.bin");
    // inOrderTraversal(root);
    // PitWait(8000);
    // switch_to_thread(p1->head_thread);
    while (1)
    {
    }
}

struct process* get_process(uint32_t nr)
{
    struct process *proc = &processes[nr];
    return proc;
}

void scheduler(void)
{

    struct process *p;
    struct cpu *cpu = curr_cpu();
    cpu->proc = 0;

    for (;;)
    {
        // asm volatile("sti");

        acquire(&scheduler_lock);

        // struct rb_node* node = rb_search_runnable(root);        
        for (uint32_t i = 0; 0 < 100; i++)
        {
            if (processes[i].state != RUNNABLE)
                continue;

            p = &processes[i];
            cpu->proc = p;
            mem_change_page_directory(p->page_directory);
            p->state = RUNNABLE;
            switch_to_thread(p->head_thread);

            mem_change_page_directory(kernel_directory);

            cpu->proc = 0;
        }

        release(&scheduler_lock);
    }
}

void init_locks()
{
    init_lock(&rb_tree_lock, "rb_tree_lock");
    init_lock(&scheduler_lock, "scheduler_lock");
}



uint32_t init_proc()
{

    for (uint32_t i = 0; i < 100; i++)
    {
        processes[i].state = UNUSED;
        processes[i].pid = i;
    }

    init_locks();
    // node_preparation();

    // struct process* p1 = create_process("0:/test.bin");
    struct process *pk1 = create_kernel_process(&test_process);
    //struct process* pk2 = &processes[0];
    struct process* pk2 = get_process(0);
    // struct process* pk2 = create_kernel_process(&test_process);
    // struct process* pk3 = create_kernel_process(&test_process);
    //switch_to_thread(pk2->head_thread);
    // inOrderTraversal(root);

    return 0;
}
