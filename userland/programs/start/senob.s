
global syscall_0_print
global syscall_1_load_proc

syscall_0_print:
    mov eax, 0
    mov ebx, [esp+4]
    int 0x80
    ret

syscall_1_load_proc:
    mov eax, 1
    mov ebx, [esp+4]
    int 0x80
    ret


