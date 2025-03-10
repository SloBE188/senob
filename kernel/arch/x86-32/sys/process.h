#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "../../../../drivers/video/vbe/vbe.h"

#define USER_STACK_TOP 0xB0000000
#define PROGRAMM_VIRTUAL_ADDRESS_START 0x00400000
#define KERNEL_PROCESS 20
#define USER_PROCESS 30
#define BLACK 0
#define RED 1

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct process
{
    uint32_t pid;
    char filename[20];

    uint32_t* page_directory;

    enum procstate state;
    uint32_t assigned_cpu;
    uint32_t isuserproc;
    
    struct process* parent;
    struct process* left;
    struct process* right;
    uint32_t color;

    struct thread* head_thread;
    struct thread* tail_thread;


} __attribute__((packed));




struct regs
{
    uint32_t eax;     //  0
    uint32_t ebx;     //  4
    uint32_t ecx;     //  8
    uint32_t edx;     //  12

    uint32_t esp;     //  16
    uint32_t ebp;     //  20

    uint32_t edi;     //  24
    uint32_t eip;     //  28
    uint32_t eflags;  //  32

    uint32_t cs;      //  36
    uint32_t ss;      //  40
    uint32_t ds;      //  44
    uint32_t es;      //  48
    uint32_t fs;      //  52
    uint32_t gs;      //  56

    uint32_t cr3;     //  60
} __attribute__ ((packed));

struct thread
{
    uint32_t thread_id;
    struct process* owner;
    struct thread* next;
    struct thread* prev;

    struct regs* regs;

    
    struct
    {
        uint32_t esp0;
        uint16_t ss0;
        uint32_t stack_start;
    } kstack __attribute__ ((packed));


};

struct process* create_process(const char* filename);
extern void switch_task(struct regs* regs);
void copy_program_to_address(const char* filename, uint32_t pages_needed, uint32_t program_address);
uint32_t map_program_to_address(const char* filename, uint32_t program_address);
struct registers_save* save_thread_state(struct thread* thread);
uint32_t init_proc();
void scheduler(void);
struct process *rb_search(uint32_t pid);
struct process *rb_search_runnable(struct process *root);
void inOrderTraversal(struct process *x);
void process_exit(uint32_t pid);
uint32_t get_curr_pid();
struct process *create_kernel_process(void (*start_function)());
void exec_proc(struct process* proc);

#endif
