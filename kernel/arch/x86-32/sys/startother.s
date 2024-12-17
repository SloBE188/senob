[BITS 16]
section .trampoline


global trampoline
global trampoline_end


trampoline:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    
    jmp 0x08:pmmode


;GDT
gdt_start:
gdt_null:
    dd 0x0
    dd 0x0

;offset 0x8
gdt_code:       ;CS
    dw 0xffff   ;Segment limit first 0-15 bits
    dw 0        ;Base first 0-15 bits
    db 0        ;Base 16-23 bits
    db 0x9a     ;Access byte
    db 11001111b;High 4 bit flags and the low 4 bit flags
    db 0        ;Base 24-31 bits

;offset 0x10
gdt_data:       ;DS,SS,ES,FS,GS
    dw 0xffff   ;Segment limit first 0-15 bits
    dw 0        ;Base first 0-15 bits
    db 0        ;Base 16-23 bits
    db 0x92     ;Access byte
    db 11001111b;High 4 bit flags and the low 4 bit flags
    db 0        ;Base 24-31 bits

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start -1
    dd gdt_start

[BITS 32]
pmmode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x9000

    jmp $

trampoline_end:
