#include "syscalls.h"
#include "../interrupts/idt.h"
#include "../sys/process.h"
#include "../../../../drivers/video/vbe/vbe.h"


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

void syscall_1_load_process(struct Interrupt_registers* regs)
{
    char* user_program_name = regs->ebx;
    struct process* new_program = create_process(user_program_name);
    switch_to_thread(new_program->head_thread);
}

void syscall_2_clear_screen(struct Interrupt_registers* regs)
{
    uint32_t color = regs->ebx;
    clear_screen_sys_2(color);

}

void register_syscalls()
{
    printf("registering syscalls\n");
    add_syscalls(PRINT_SYSCALL, syscall_0_print);
    add_syscalls(LOAD_PROCESS_SYSCALL, syscall_1_load_process);
    add_syscalls(CLEAR_SCREEM_SYSCALL, syscall_2_clear_screen);
}