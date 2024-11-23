#include "syscalls.h"
#include "../interrupts/idt.h"

syscalls_fun_ptr syscall_functions[1024] = {NULL};

void init_syscalls()
{
    register_syscalls();
}

void add_syscalls(uint32_t syscal_number, syscalls_fun_ptr sys_function)
{
    printf("adding syscall %u at address %p\n", syscal_number, sys_function);
    syscall_functions[syscal_number] = sys_function;
}


void syscall_0_print(struct Interrupt_registers* regs)
{
    
    char* user_string = regs->ebx;
    printf("%s", user_string);
    
}

void register_syscalls()
{
    printf("registering syscalls\n");
    add_syscalls(PRINT_SYSCALL, syscall_0_print);
}