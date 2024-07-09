section .asm

global insb
global insw
global outb
global outw


;label reads 8 bits (1 byte) from a specified I/O port. The read data is transferred to the dx register and then loaded into the lowest bytes of the ax register
insb:
    push eb
    mov ebp, esp    

    xor eax, eax
    mov edx, [ebp+8]
    in al, dx

    pop ebp         ;pop ebp to restore ebp from the top of the stack
    ret

;label reads 16 bits (2 bytes) from a specified I/O port.
insw:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov edx, [ebp+8]
    in ax, dx

    pop ebp
    ret

;label writes 8 bits (1 byte) to a specified I/O port. The value for writing is transferred to the eax register and the destination port to the dx register 
;value is loaded into the lowest byte (al) of the eax register
outb:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov eax, [ebp+12]
    mov edx, [ebp+8]
    out dx, al

    pop ebp
    ret


;label writes 16 bits (2 bytes) to a specified I/O port.
outw:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov eax, [ebp+12]
    mov edx, [ebp+8]
    out dx, ax

    pop ebp
    ret