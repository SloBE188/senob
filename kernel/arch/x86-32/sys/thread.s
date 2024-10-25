
global user_register
global userland


user_register:
    mov eax, 0x23
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    ret

; void userland(uint32_t* esp, uint32_t* eip)
userland:
    cli
    mov ebp, esp

    mov eax, [ebp+4]
    push 0x23
    push eax

    push 0x202
    push 0x1B

    mov eax, [ebp+8]
    push eax

    call user_register
    
    iret
    