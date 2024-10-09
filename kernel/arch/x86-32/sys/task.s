global context_switch

section .text
context_switch:
    ; Lade den neuen TSS
    mov ax, 0x28
    ltr ax

    ; Führe den Kontextwechsel durch
    mov esp, [esp + 4]        ; ESP des neuen Tasks setzen (übergeben als Argument)
    push dword [esp + 8]      ; EFLAGS des neuen Tasks pushen (übergeben als Argument)
    push dword [esp + 12]     ; EIP des neuen Tasks pushen (übergeben als Argument)
    push dword [esp + 16]     ; CS des neuen Tasks pushen (übergeben als Argument)
    push dword [esp + 20]     ; Stack-Segment des neuen Tasks pushen (übergeben als Argument)
    iret                      ; Interrupt Return, um den Kontext zu wechseln
