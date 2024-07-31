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

MULTIBOOT_MAGIC EQU 0x1BADB002
MULTIBOOT_CHECKSUM EQU -(MULTIBOOT_MAGIC + 0x00000007)


BITS 32

global start
extern kernel_main
global initial_page_dir

section .multiboot
	ALIGN 4
	DD MULTIBOOT_MAGIC
	DD 0x00000007
	DD -(0x1BADB002 + 0x00000007)
	DD 0
	DD 0
	DD 0
	DD 0
	DD 0
	DD 0
	DD 1024
	DD 768
	DD 32
	

section .bss
ALIGN 16
stack_bottom:
	RESB 16384 * 8
stack_top:

section .boot


start:
    MOV ecx, (initial_page_dir - 0xC0000000)
    MOV cr3, ecx

    MOV ecx, cr4
    OR ecx, 0x10
    MOV cr4, ecx

    MOV ecx, cr0
    OR ecx, 0x80000000
    MOV cr0, ecx

    JMP higher_half



section .text
higher_half:
	MOV esp, stack_top
	PUSH ebx
	PUSH eax
	XOR ebp, ebp
	CALL kernel_main

haltkernel:
	HLT
	JMP haltkernel
	
section .data
ALIGN 4096
initial_page_dir:
	DD 10000011b				; first page directory entry (entry 0) these are the bits set for the first page directory entry. BIT 0: Present, BIT 1: Read/Write, BIT 2: User/Supervisor, BIT 7: Page Size(4MB if true), BIT 12-31: Basisaddress (0 here for 0x00000000. In a second page directory it would be 0x00800000 so the whol DD command would be like that: 1000000010000000000000011b or hex: 0x00800083)
	TIMES 768-1 DD 0			; fill up the next 767 entrys with 0

    DD (0 << 22) | 10000011b	; page directory entry 768, pointer tp 4MB page because BIT 7 is set, virtual and physical address 0x00000000 - 0x003FFFFF
    DD (1 << 22) | 10000011b	; page directory entry 769, virtual and physical address 0x00400000 - 0x007FFFFF
    DD (2 << 22) | 10000011b	; page directory entry 770, virtual and physical address 0x00800000 - 0x00BFFFFF
    DD (3 << 22) | 10000011b	; page directory entry 771, virtual and physical address 0x00C00000 - 0x00FFFFFF
	; physical and virtual adresses are curr linear because i haven't switched them up yet
    TIMES 256-4 DD 0			; fill up the next 252 entrys with 0 (771 - 1023)
	; now all 1024 entrys from the page directory are filled


