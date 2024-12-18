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

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    mov eax, gdt_descriptor
    lgdt[eax]

    jmp 0x8:0x8000


;GDT
gdt_start:
gdt_null:
    dd 0x0
    dd 0x0

;offset 0x8
gdt_code:       
    dw 0xffff   
    dw 0       
    db 0       
    db 0x9a     
    db 11001111b
    db 0   

;offset 0x10
gdt_data:       
    dw 0xffff   
    dw 0        
    db 0        
    db 0x92     
    db 11001111b
    db 0        

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start -1
    dd gdt_start
    

section .pmmc
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
