; context_switch.asm
section .text
global context_switch

extern current_task

context_switch:
    ; Save current task's registers
    pusha                      ; Push all general-purpose registers
    mov eax, current_task      ; current_task points to the current task struct
    mov eax, [eax]             ; Load the pointer to the current task struct

    ; Store the register values into the current task's register structure
    mov [eax + 0], edi
    mov [eax + 4], esi
    mov [eax + 8], ebp
    mov dword [eax + 12], esp  ; Specify DWORD for 32-bit size
    mov [eax + 16], ebx
    mov [eax + 20], edx
    mov [eax + 24], ecx
    mov [eax + 28], eax

    ; Save the IP, CS, and EFLAGS
    mov eax, [esp + 4]         ; IP (return address is on the stack after call to context_switch)
    mov dword [eax + 32], eax  ; Specify DWORD for 32-bit size

    mov ax, cs
    mov word [eax + 36], ax    ; Specify WORD for 16-bit size

    pushf
    pop eax
    mov dword [eax + 40], eax  ; Specify DWORD for 32-bit size

    ; Load the new task (the parameter to context_switch is in `ebx`)
    mov eax, ebx                ; Load the pointer to the new task struct

    ; Restore the new task's registers
    mov edi, [eax + 0]
    mov esi, [eax + 4]
    mov ebp, [eax + 8]
    mov esp, [eax + 12]
    mov ebx, [eax + 16]
    mov edx, [eax + 20]
    mov ecx, [eax + 24]
    mov eax, [eax + 28]

    ; Restore IP, CS, and EFLAGS
    push dword [eax + 40]      ; EFLAGS (32-bit)
    push word [eax + 36]       ; CS (16-bit)
    push dword [eax + 32]      ; IP (32-bit)

    ; Jump to the new task
    iret                       ; Interrupt return to load CS, IP, and FLAGS of the new task
              ; Interrupt return to load CS, IP, and FLAGS of the new task
