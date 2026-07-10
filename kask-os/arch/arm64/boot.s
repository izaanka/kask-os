.section .text
.global _start

_start:
    /* Read CPU ID, park all cores except core 0 */
    mrs x0, mpidr_el1
    and x0, x0, #3
    cbz x0, .Lcpu0

.Lpark:
    wfe
    b .Lpark

.Lcpu0:
    /* Set up stack pointer */
    ldr x0, =stack_top
    mov sp, x0

    /* Clear BSS */
    ldr x0, =__bss_start
    ldr x1, =__bss_end
.Lclear_bss:
    cmp x0, x1
    b.ge .Ldone_bss
    str xzr, [x0], #8
    b .Lclear_bss
.Ldone_bss:

    /* Jump to C kernel */
    bl kernel_main

    /* Halt if kernel returns */
1:  wfi
    b 1b

.section .bss
.align 16
stack_bottom:
.skip 32768
stack_top:
