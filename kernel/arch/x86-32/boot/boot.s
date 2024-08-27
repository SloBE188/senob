; Copyright (C) 2024 Nils Burkard
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; at your option, any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program. If not, see <http://www.gnu.org/licenses/>.

MULTIBOOT_MAGIC EQU 0x1BADB002
MULTIBOOT_FLAGS EQU 0x00000007  ; Memory info and video mode
MULTIBOOT_CHECKSUM EQU -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

KERNEL_VIRTUAL_START EQU 0xC0000000

BITS 32

global _start
extern kernel_main
global kernel_directory

section .multiboot
    ALIGN 4
    DD MULTIBOOT_MAGIC      ; Magic Number
    DD MULTIBOOT_FLAGS      ; Flags
    DD MULTIBOOT_CHECKSUM   ; Checksum
    DD 0                    ; Header Address
    DD 0                    ; Load Address
    DD 0                    ; Load End Address
    DD 0                    ; BSS End Address
    DD 0                    ; Entry Address
    DD 0                    ; Mode Type (0 for linear framebuffer)
    DD 1024                 ; Framebuffer Width
    DD 768                  ; Framebuffer Height
    DD 32                   ; Framebuffer Depth

section .bss
stack_bottom:
    RESB 16384 * 8              ; 16 KiB stack
stack_top:

section .data
ALIGN 4096
kernel_directory:
    DD 0x00000083 ; First Page Table Entry (0x00000000 - 0x003FFFFF)
    DD 0x00400083 ; Second Page Table Entry (0x00400000 - 0x007FFFFF)
    DD 0x00800083 ; Third Page Table Entry (0x00800000 - 0x00BFFFFF)
    DD 0x00C00083 ; Fourth Page Table Entry (0x00C00000 - 0x00FFFFFF)
    TIMES 768-4 DD 0 ; Fill up
    ; Mapping the kernel from 0xC0000000 - 0xC03FFFFF (4 MiB) to 0x00000000 - 0x003FFFFF
    DD 0x00000083
    TIMES 128-1 DD 0 ; Fill up to entry 896
    ; Mapping the heap from 0xD0000000 - 0xD003FFFF (4 MiB) to 0x00800000 - 0x00BFFFFF
    DD 0x00800083
    ; Mapping für den Framebuffer (virtuelle Adresse 0xE0000000)
    DD 0xFD000083 ; should map 0xE0000000 - 0xE03FFFFF (4 MiB) to 0xFD000000 - 0xFD3FFFFF

    TIMES 128-1 DD 0 ; fill up to entry 1024

section .text
_start:
    ; Load the address of kernel_directory into ecx
    mov ecx, kernel_directory
    sub ecx, KERNEL_VIRTUAL_START
    mov cr3, ecx           ; Setze CR3 auf die Adresse von kernel_directory

    ; Enable PSE with 4 MiB pages
    mov ecx, cr4
    or ecx, 0x00000010
    mov cr4, ecx

    ; Enable paging
    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    ; Jump to higher half kernel start
    lea ecx, [start_higher_half_kernel]
    jmp ecx

start_higher_half_kernel:
    ; Setup stack
    mov esp, stack_top
    xor ebp, ebp
    push ebx
    push eax
    cli
    
    ; Call kernel_main
    call kernel_main
    
    ; Halt if kernel_main returns
halt:
    hlt
    jmp halt

