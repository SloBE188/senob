#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>


#define USER_STACK_TOP 0xB0000000
#define PROGRAMM_VIRTUAL_ADDRESS_START 0x00400000
#define BLACK 0
#define RED 1


enum threadState {UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, WAITING};

extern struct process* root;

struct process
{
    uint32_t pid;
    char* filename;
    uint32_t* pageDirectory;
    uint32_t isUserProc;

    uint32_t color;
    struct process* parent;
    struct process* left;
    struct process* right;

    struct thread* head_thread;
    struct thread* tail_thread;
}__attribute__((packed));


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
    uint32_t threadID;
    struct process* owner;

    struct thread* next;
    struct thread* prev;
    
    struct regs* regs;
    enum threadState state;

    uint32_t basePriority;
    uint32_t dynamicPriority;
    uint32_t waitTicks;

    struct
    {
        uint32_t esp0;
        uint16_t ss0;
        uint32_t stackStart;
    } kstack __attribute__ ((packed));
};

struct process* createKernelProcess(void *(function)());
struct thread* createKernelThread(struct process* process, void *(function)());
struct process* createUserProcess(const char* filename);
struct thread* createUserThread(struct process* process);
extern void switchTask(struct regs* regs);
void initProc();
void inOrderTraversal(struct process *x, void (*callback)(struct process*));
uint32_t get_local_apic_id_cpuid();
struct process *rbSearch(uint32_t pid);
uint32_t initIdle();



#endif