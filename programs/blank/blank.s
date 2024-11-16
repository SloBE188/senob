[BITS 32]

section .text

global _userstart

_userstart:
    ; Initialisieren der Register mit Werten
    mov eax, 5      ; eax = 5
    mov ebx, 10     ; ebx = 10
    mov ecx, 2      ; ecx = 2
    mov edx, 1      ; edx = 1

    ; Eine einfache Berechnung durchführen
    add eax, ebx    ; eax = eax + ebx (5 + 10 = 15)
    sub ebx, ecx    ; ebx = ebx - ecx (10 - 2 = 8)
    imul ecx, eax   ; ecx = ecx * eax (2 * 15 = 30)
    xor edx, edx    ; edx = edx ^ edx (edx = 0)

    ; Stack-Operationen
    push eax        ; Wert von eax auf den Stack legen
    push ebx        ; Wert von ebx auf den Stack legen
    pop eax         ; Wert von ebx zurück nach eax laden (eax = 8)
    pop ebx         ; Wert von eax zurück nach ebx laden (ebx = 15)

    ; Vergleichsoperation
    cmp eax, ebx    ; Vergleiche eax mit ebx
    jg .greater     ; Springe zu .greater, wenn eax > ebx

.less_or_equal:
    ; Dies wird ausgeführt, wenn eax <= ebx
    mov edx, 0xDEAD ; Setze edx auf 0xDEAD
    jmp .end_compare

.greater:
    ; Dies wird ausgeführt, wenn eax > ebx
    mov edx, 0xBEEF ; Setze edx auf 0xBEEF

.end_compare:
    ; Endlosschleife
.loop:
    jmp .loop       ; Springe zurück zu .loop, bleibt hier für immer
