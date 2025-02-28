#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>
#include "../../../libk/stdiok.h"
#include "../kernel.h"

typedef void (*syscalls_fun_ptr)(struct Interrupt_registers* regs);

extern syscalls_fun_ptr syscall_functions[1024];



enum syscall_numbers
{
    PRINT_SYSCALL,
    LOAD_PROCESS_SYSCALL,
    CLEAR_SCREEN_SYSCALL,
    WRITE_SYSCALL,
    SBRK_SYSCALL,
    OPEN_SYSCALL,
    READDIR_SYSCALL,
    CLOSE_SYSCALL,
    READ_SYSCALL,
    LSEEK_SYSCALL,
    STAT_SYSCALL,
    FSTAT_SYSCALL,
    MKDRI_SYSCALL,
    GKBUFFER_SYSCALL,
    DRAW_FRAME_DOOM_SYSCALL,
    GETTICKS_SYSCALL,
    EXECVE_SYSCALL,
    GETPID_SYSCALL
};

void add_syscalls(uint32_t syscall_number, syscalls_fun_ptr sys_function);

void register_syscalls();
void init_syscalls();

#endif