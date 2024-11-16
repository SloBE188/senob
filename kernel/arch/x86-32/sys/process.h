#ifndef PROCESS_H
#define PROCESS_H

#include <stdint-gcc.h>

#define USER_STACK_TOP 0xB0000000
#define PROGRAMM_VIRTUAL_ADDRESS_START 0x00400000

struct process
{
    uint32_t pid;
    char filename[20];

    uint32_t* page_directory;
    struct thread* thread;

} __attribute__((packed));

struct registers
{
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;

    uint32_t eip;
    uint32_t eflags;

    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;

    uint32_t cr3;
};

struct thread
{
    uint32_t thread_id;
    struct process* owner;

    struct registers* regs;

    struct
    {
        uint32_t esp0;
        uint16_t ss0;
        uint32_t stack_start;
    } kstack __attribute__ ((packed))

};

struct process* create_process(const char* filename);
extern void switch_task(struct thread* regs);
void switch_to_thread(struct thread* thread);
void copy_program_to_address(const char* filename, uint32_t pages_needed, uint32_t program_address);
uint32_t map_program_to_address(const char* filename, uint32_t program_address);

#endif