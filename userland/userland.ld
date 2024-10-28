ENTRY(_userstart)

SECTIONS {
    . = 0x00400000;
    
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
