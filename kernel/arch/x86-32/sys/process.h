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
        uint32_t eax, ecx, edx, ebx;
        uint32_t esp, ebp, esi, edi;
        uint32_t eip, eflags;
        uint32_t cs:16, ss:16, ds:16, es:16, fs:16, gs:16;
        uint32_t cr3;
};

struct thread
{
    uint32_t thread_id;
    struct process* owner;

    struct
    {
        uint32_t eax, ecx, edx, ebx;
        uint32_t esp, ebp, esi, edi;
        uint32_t eip, eflags;
        uint32_t cs:16, ss:16, ds:16, es:16, fs:16, gs:16;
        uint32_t cr3;
    } regs __attribute__ ((packed));

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