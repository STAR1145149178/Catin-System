    .section ".text.boot","ax"
    .global _start
    .arm
_start:
    /* 开 FPU (CP10/CP11 全访问) */
    mrc p15, 0, r0, c1, c0, 2
    orr r0, r0, #(0xF << 20)
    mcr p15, 0, r0, c1, c0, 2
    isb
    /* 开 VFP */
    mov r0, #(1 << 30)
    vmsr fpexc, r0
    isb

    cpsid if
    mov r4, r1
    mov r5, r2
    ldr sp, =__boot_stack_top
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
1:  cmp r0, r1
    strlo r2, [r0], #4
    blo 1b
    ldr r0, =exception_vectors
    mcr p15, 0, r0, c12, c0, 0
    isb
    mov r0, r4
    mov r1, r5
    ldr r2, =kernel_main
    bx r2
    .section ".bss","aw",%nobits
    .align 12
__boot_stack_bottom: .space 0x4000
__boot_stack_top: