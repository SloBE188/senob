
global task_switch
; void task_switch(struct task* old, struct task* new);
task_switch:
    pusha                   ; pushes general purpose regs
    pushfd                  ; pushes EFLAGS regs

    ; load adresses from the task structs in eax und edx
    mov eax, [esp + 36]     ; old task
    mov edx, [esp + 40]     ; new task

    ; save old esp
    mov [eax + 8], esp      ; old->esp = current esp (Offset 0x08)

    ; load esp from new task
    mov esp, [edx + 8]      ; esp = new->esp (Offset 0x08)

    ; switch page directory
    mov eax, [edx + 4]
    sub eax, 0xC0000000
    mov cr3, eax

    popfd                   ;restores the regs from the new task because i switched the esp (points to the new task now)
    popa                    ;restores the regs from the new task because i switched the esp (points to the new task now)
    ret