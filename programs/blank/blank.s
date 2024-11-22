[BITS 32]

section .text
global _userstart

_userstart:
    mov eax, 0
    mov ebx, 30
    int 0x80

loop:
    jmp loop


