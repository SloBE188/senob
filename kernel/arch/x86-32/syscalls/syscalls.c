#include "syscalls.h"
#include "../interrupts/idt.h"
#include "../sys/process.h"
#include "../../../../drivers/video/vbe/vbe.h"
#include "../../../libk/libk.h"


syscalls_fun_ptr syscall_functions[1024] = {NULL};

void init_syscalls()
{
    register_syscalls();
}

void add_syscalls(uint32_t syscal_number, syscalls_fun_ptr sys_function)
{
    kernel_write("adding syscall %u at address %p\n", syscal_number, sys_function);
    syscall_functions[syscal_number] = sys_function;
}


void syscall_0_print(struct Interrupt_registers* regs)
{
    
    char* user_string = regs->ebx;
    kernel_write("%s", user_string);
    
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

void syscall_3_write(struct Interrupt_registers* regs)
{
    int fd = (int)regs->ebx;
    const void* buf = (const void*)regs->ecx;
    size_t len = (size_t)regs->edx;
    regs->eax = write(fd, buf, len);
}


void register_syscalls()
{
    kernel_write("registering syscalls\n");
    add_syscalls(PRINT_SYSCALL, syscall_0_print);
    add_syscalls(LOAD_PROCESS_SYSCALL, syscall_1_load_process);
    add_syscalls(CLEAR_SCREEN_SYSCALL, syscall_2_clear_screen);
    add_syscalls(WRITE_SYSCALL, syscall_3_write);
}