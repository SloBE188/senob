global gdt_flush
global tss_flush

gdt_flush:
    mov eax, [esp+4]
    lgdt[eax]

    mov eax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush

.flush:
    ret

tss_flush:
    mov ax, 0x2B    ;not 0x28 because 0x2B are the bottom 2 bits
    ltr ax
    ret   