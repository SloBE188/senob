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
    TIMES 768-1 DD 0
    ; Kernel Mapping: Mapping von 0xC0000000 - 0xC2FFFFFF (48 MiB)
    DD 0x00000083 ; Entry 768 (0xC0000000 - 0xC03FFFFF) mapped to 0x00000000 - 0x003FFFFF
    DD 0x00400083 ; Entry 769 (0xC0400000 - 0xC07FFFFF) mapped to 0x00400000 - 0x007FFFFF
    DD 0x00800083 ; Entry 770 (0xC0800000 - 0xC0BFFFFF) mapped to 0x00800000 - 0x00BFFFFF
    DD 0x00C00083 ; Entry 771 (0xC0C00000 - 0xC0FFFFFF) mapped to 0x00C00000 - 0x00FFFFFF
    DD 0x01000083 ; Entry 772 (0xC1000000 - 0xC13FFFFF) mapped to 0x01000000 - 0x013FFFFF
    DD 0x01400083 ; Entry 773 (0xC1400000 - 0xC17FFFFF) mapped to 0x01400000 - 0x017FFFFF
    DD 0x01800083 ; Entry 774 (0xC1800000 - 0xC1BFFFFF) mapped to 0x01800000 - 0x01BFFFFF
    DD 0x01C00083 ; Entry 775 (0xC1C00000 - 0xC1FFFFFF) mapped to 0x01C00000 - 0x01FFFFFF
    DD 0x02000083 ; Entry 776 (0xC2000000 - 0xC23FFFFF) mapped to 0x02000000 - 0x023FFFFF
    DD 0x02400083 ; Entry 777 (0xC2400000 - 0xC27FFFFF) mapped to 0x02400000 - 0x027FFFFF
    DD 0x02800083 ; Entry 778 (0xC2800000 - 0xC2BFFFFF) mapped to 0x02800000 - 0x02BFFFFF
    DD 0x02C00083 ; Entry 779 (0xC2C00000 - 0xC2FFFFFF) mapped to 0x02C00000 - 0x02FFFFFF
    TIMES 128-12 DD 0

    ; Framebuffer Mapping bei 0xFD000000
    DD 0xFD000083 ; Entry 896 (0xE0000000 - 0xE03FFFFF) mapped to 0xFD000000 - 0xFD3FFFFF

    TIMES 1024-897 DD 0 ; Fill up the rest of the Page Directory




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
