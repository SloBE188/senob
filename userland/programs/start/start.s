[BITS 32]

section .text
global _userstart

extern main

_userstart:
    mov eax, 0
    mov ebx, 30
    int 0x80
    call main

loop:
    jmp loop


