global switch_task

section .text
switch_task:

    ; esi points to the register struct
    mov esi, [esp + 4]      ; address from the register struct

    mov eax, [esi + 60]
    sub eax, 0xC0000000
    mov cr3, eax

    ; load segment register
    mov eax, [esi + 44]      ; ds
    mov ds, eax
    mov eax, [esi + 48]      ; es
    mov es, eax
    mov eax, [esi + 52]      ; fs
    mov fs, eax
    mov eax, [esi + 56]      ; gs
    mov gs, eax

    ; load general purpose regs
    mov eax, [esi + 0]      ; eax
    mov ebx, [esi + 4]     ; ebx
    mov ecx, [esi + 8]      ; ecx
    mov edx, [esi + 12]      ; edx
    mov edi, [esi + 24]     ; edi
    mov ebp, [esi + 20]     ; ebp

    ; push stack pointer, eip etc
    mov esp, [esi + 16]     ; esp
    push dword [esi + 40]   ; ss (Stack Segment)
    push dword [esi + 16]   ; esp
    push dword [esi + 32]   ; eflags
    push dword [esi + 36]   ; cs
    push dword [esi + 28]   ; eip

    iret
