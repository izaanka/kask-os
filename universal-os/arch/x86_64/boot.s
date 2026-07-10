.set MAGIC,    0xe85250d6
.set ARCH,     0
.set LENGTH,   (multiboot_header_end - multiboot_header_start)
.set CHECKSUM, -(MAGIC + ARCH + LENGTH)

.section .multiboot
multiboot_header_start:
    .long MAGIC
    .long ARCH
    .long LENGTH
    .long CHECKSUM
    .short 0, 0
    .long 8
multiboot_header_end:

.section .text
.global _start
_start:
    mov $stack_top, %esp
    call kernel_main
cli
1:  hlt
    jmp 1b

.section .bss
.skip 16384
stack_top:
