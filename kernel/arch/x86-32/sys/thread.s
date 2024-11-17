global switch_task

section .text
switch_task:
    ; ebx points to the register struct
    mov ebx, [esp + 4]      ; address from the register struct

    ; load pd
    ;mov eax, [esi + 64]
    ;mov cr3, eax

    ; load segment register
    mov eax, [ebx + 48]      ; ds
    mov ds, eax
    mov eax, [ebx + 52]      ; es
    mov es, eax
    mov eax, [ebx + 56]      ; fs
    mov fs, eax
    mov eax, [ebx + 60]      ; gs
    mov gs, eax

    ; load general purpose regs
    mov eax, [ebx + 0]      ; eax
    mov ebx, [ebx + 4]     ; ebx
    mov ecx, [ebx + 8]      ; ecx
    mov edx, [ebx + 12]      ; edx
    mov esi, [ebx + 24]     ; esi
    mov edi, [ebx + 28]     ; edi
    mov ebp, [ebx + 20]     ; ebp

    ; push stack pointer, eip etc
    mov esp, [ebx + 16]     ; esp
    push dword [ebx + 36]   ; eflags
    push dword [ebx + 40]   ; cs
    push dword [ebx + 32]   ; eip
    push dword [ebx + 44]   ; ss (Stack Segment)
    push dword [ebx + 16]   ; esp
    iret
