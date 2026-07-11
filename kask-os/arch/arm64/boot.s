/* ============================================================
 * Kask OS — AArch64 Boot Stub
 * Raspberry Pi Zero 2W (BCM2837, Cortex-A53) + Pi 4 (BCM2711, Cortex-A72)
 * GPU loads kernel8.img at 0x80000, starts all 4 cores simultaneously.
 * We park cores 1-3, drop EL2 → EL1, set up stack, clear BSS, call kernel.
 * ============================================================ */

.section .text.boot
.global _start

_start:
    /* ---- Park secondary cores (only core 0 continues) ---- */
    mrs  x0, mpidr_el1
    and  x0, x0, #0xFF          /* Aff0 = CPU number within cluster */
    cbz  x0, .Lcpu0

.Lpark:
    wfe
    b    .Lpark

.Lcpu0:
    /* ---- Determine current exception level ---- */
    mrs  x0, CurrentEL
    lsr  x0, x0, #2             /* EL in bits [3:2], shift to [1:0] */
    cmp  x0, #2
    b.eq .Ldrop_el2
    cmp  x0, #1
    b.eq .Lsetup_el1
    b    .Lpark                  /* EL0/EL3: shouldn't happen */

.Ldrop_el2:
    /* Configure EL1 to run in AArch64 mode */
    mov  x0, #(1 << 31)         /* HCR_EL2.RW = 1 → EL1 is AArch64 */
    msr  hcr_el2, x0
    isb

    /* Safe SCTLR_EL1: MMU off, D-cache off, I-cache off, little-endian.
       RES1 bits (must be 1): 29,28,23,22,20,11 per ARM DDI 0487 */
    mov  x0, #0x0800
    movk x0, #0x30D0, lsl #16
    msr  sctlr_el1, x0
    isb

    /* SPSR_EL2: return to EL1h (Handler stack), all DAIF bits masked */
    /* M[4:0] = 0b00101 = EL1h, DAIF = bits[9:6] = 0b1111 */
    mov  x0, #0x3C5
    msr  spsr_el2, x0

    /* Return address for eret */
    adr  x0, .Lsetup_el1
    msr  elr_el2, x0
    isb
    eret                         /* Drop to EL1, continue at .Lsetup_el1 */

.Lsetup_el1:
    /* ---- Set up stack pointer (grows down from stack_top) ---- */
    ldr  x0, =stack_top
    mov  sp, x0

    /* ---- Zero BSS ---- */
    ldr  x0, =__bss_start
    ldr  x1, =__bss_end
.Lclear_bss:
    cmp  x0, x1
    b.ge .Ldone_bss
    str  xzr, [x0], #8
    b    .Lclear_bss
.Ldone_bss:

    /* ---- Call C kernel ---- */
    bl   kernel_main

    /* ---- Halt on return ---- */
.Lhalt:
    wfi
    b    .Lhalt

/* ---- 32KB kernel stack ---- */
.section .bss
.align 16
stack_bottom:
    .skip 32768
stack_top:
