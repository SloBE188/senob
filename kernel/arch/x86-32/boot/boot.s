; Copyright (C) 2024 Nils Burkard
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program. If not, see <http://www.gnu.org/licenses/>.

BITS 32

section .text
	ALIGN 4
	DD 0x1BADB002
	DD 0x00000007
	DD -(0x1BADB002 + 0x00000007)
	DD 0
	DD 0
	DD 0
	DD 0
	DD 0
	DD 1
	DD 1024
	DD 768
	DD 32
	
global start
extern kernel_main

start:
	CLI
	MOV esp, stack_space
	PUSH EBX	; Pointer to the Multiboot information structure
	PUSH EAX	; Magic value
	CALL kernel_main
	HLT

haltkernel:
	CLI
	HLT
	JMP haltkernel
	
	
section .bss
RESB 8192	
stack_space:
