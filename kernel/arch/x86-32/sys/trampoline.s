%define REBASE16(ADDR) (ADDR - trampoline + 0x7000)
%define REBASE32(ADDR) (ADDR - pMode + 0x8000)

KERNEL_VIRTUAL_START EQU 0xC0000000

extern kernel_directory
extern initializeAP

[BITS 16]

section .trampoline

global trampoline

; Initializes a simple stack and jumps to protected mode.
trampoline:
    cli
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov sp, 0x7C00

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    lgdt[REBASE16(gdtDescriptor)]

    jmp 0x08:REBASE32(pMode)
    jmp $



;GDT
gdtStart:
gdtNull:
    dd 0x0
    dd 0x0

;offset 0x8
gdtCode:       
    dw 0xffff   
    dw 0       
    db 0       
    db 0x9a     
    db 11001111b
    db 0   

;offset 0x10
gdtData:       
    dw 0xffff   
    dw 0        
    db 0        
    db 0x92     
    db 11001111b
    db 0        

gdtEnd:

gdtDescriptor:
    dw gdtEnd - gdtStart -1
    dd gdtStart


section .pMode

[BITS 32]

; Initializes a stack, activates paging (and loads kernel directory).
; Then it jumps to higher half (0xC0000000)
pMode:
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

    ; Jump to higher half
    lea ecx, [higherHalfAPStart]
    jmp ecx

; Initializes stack and calls the C function initializeAP
higherHalfAPStart:
    ; Setup stack
    mov esp, stack_top
    xor ebp, ebp
    cli

    call initializeAP

    jmp $


section .bss
stack_bottom:
    RESB 16384 * 8              ; 16 KiB stack
stack_top: