[BITS 16]
global _startcpu

section .trampoline


_startcpu:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    lgdt [0x5000]

    ;mov eax, cr0
    ;or eax, 0x1
    ;mov cr0, eax
    jmp $