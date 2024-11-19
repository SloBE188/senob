[BITS 32]

section .text

global _userstart

_userstart:
 
.loop:
    jmp .loop
