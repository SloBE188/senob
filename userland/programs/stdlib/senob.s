
global syscall_0_print
global syscall_1_load_proc
global syscall_2_clear_screen
global syscall_3_write
global syscall_4_sbrk

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

syscall_2_clear_screen:
    mov eax, 2
    mov ebx, [esp+4]
    int 0x80
    ret

syscall_3_write:
    mov eax, 3
    mov ebx, [esp + 4]      ;fd
    mov ecx, [esp + 8]      ;pointer to buff
    mov edx, [esp + 12]     ;count bytes
    int 0x80
    ret

syscall_4_sbrk:
    mov eax, 4
    mov ebx, [esp + 4]      ;increment
    int 0x80
    ret


