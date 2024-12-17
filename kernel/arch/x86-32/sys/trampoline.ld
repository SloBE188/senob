SECTIONS
{
    . = 0x00101000;  /* Adresse im 32-Bit Kernel (beliebige Position über 1MB) */

    .text : {
        *(.text)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }
}
