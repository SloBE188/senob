
%define REBASE16(ADDR) (ADDR - trampoline + 0x7000)
%define REBASE32(ADDR) (ADDR - pmmode + 0x8000)

KERNEL_VIRTUAL_START EQU 0xC0000000

[BITS 16]
section .trampoline


global trampoline
global trampoline_end

extern initialize_ap
extern kernel_directory

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

    lgdt[REBASE16(gdt_descriptor)]
    


    jmp 0x08:REBASE32(pmmode)
    jmp $


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

    mov ebp, 0x00200000
    mov esp, ebp


    ; Load the address of kernel_directory into ecx
    mov ecx, kernel_directory
    sub ecx, KERNEL_VIRTUAL_START
    mov cr3, ecx

    ; Enable PSE with 4 MiB pages
    mov ecx, cr4
    or ecx, 0x00000010
    mov cr4, ecx

    ; Enable paging
    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx
    
    jmp $
    call initialize_ap


    jmp $

trampoline_end: