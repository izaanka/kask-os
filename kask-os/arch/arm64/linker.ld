ENTRY(_start)
OUTPUT_FORMAT(elf64-littleaarch64)

SECTIONS {
    /*
     * RPi GPU firmware (start.elf) loads kernel8.img to 0x80000
     * and begins execution there. This address is fixed by the
     * VideoCore bootloader for AArch64 kernels.
     */
    . = 0x80000;

    .text : {
        KEEP(*(.text.boot))   /* boot stub MUST be at 0x80000 */
        *(.text)
        *(.text.*)
    }

    .rodata : ALIGN(4096) {
        *(.rodata)
        *(.rodata.*)
    }

    .data : ALIGN(4096) {
        *(.data)
        *(.data.*)
    }

    __bss_start = .;
    .bss : ALIGN(4096) {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }
    __bss_end = .;
}
