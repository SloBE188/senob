ENTRY(_start)
SECTIONS
{
    . = 0x00100000;

    .multiboot ALIGN(4K): {
        *(.multiboot)
    }

    .note.gnu.build-id ALIGN(4K): {
        *(.note.gnu.build-id)
    }

    .boot ALIGN(4K) : {
        *(.boot)
    }

    . += 0xC0000000;

    .text ALIGN(4K): AT(ADDR(.text) - 0xC0000000){
        *(.text)
    }

    .rodata ALIGN(4K): AT(ADDR(.rodata) - 0xC0000000){
        *(.rodata)
    }

    .data ALIGN(4K): AT(ADDR(.data) - 0xC0000000){
        *(.data)
    }

    .bss ALIGN(4K): AT(ADDR(.bss) - 0xC0000000){
        *(COMMON)
        *(.bss)
    }

    _kernel_end = .;

        /* Sektion, um das Userland-Programm ins Kernel-Image einzubinden */
    .userland_bin ALIGN(4K): {
        _userland_start = .;  /* Markiert den Anfang des Userland-Programms */
        KEEP(*(.userland_bin)) /* Fügt die Binärdatei blank.bin ein */
        _userland_end = .;    /* Markiert das Ende des Userland-Programms */
    }
}
