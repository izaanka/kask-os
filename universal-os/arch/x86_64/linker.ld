ENTRY(_start)
OUTPUT_FORMAT(elf32-i386) /* Forces a standard multiboot readable format */

SECTIONS {
    . = 1M;
    .text : {
        *(.multiboot)
        *(.text)
    }
    .rodata : { *(.rodata) }
    .data   : { *(.data) }
    .bss    : { *(.bss) }
}
