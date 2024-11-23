
global syscall_0_print


syscall_0_print:
    mov eax, 0
    mov ebx, [esp+4]
    int 0x80
    ret
