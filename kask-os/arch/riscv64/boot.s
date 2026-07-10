.section .text
.global _start

_start:
    /* Park all harts except hart 0 */
    csrr t0, mhartid
    bnez t0, .Lpark

    /* Set up stack pointer */
    la sp, stack_top

    /* Clear BSS */
    la t0, __bss_start
    la t1, __bss_end
.Lclear_bss:
    bge t0, t1, .Ldone_bss
    sd zero, 0(t0)
    addi t0, t0, 8
    j .Lclear_bss
.Ldone_bss:

    /* Jump to C kernel */
    call kernel_main

    /* Halt if kernel returns */
1:  wfi
    j 1b

.Lpark:
    wfi
    j .Lpark

.section .bss
.align 16
stack_bottom:
.skip 32768
stack_top:
