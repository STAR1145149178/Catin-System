    .section ".text.boot", "ax"
    .global _start
_start:
    /* 打开 FP/SIMD 访问 (关键!) */
    mov x0, #(0x3 << 20)
    msr cpacr_el1, x0
    isb

    mov x19, x0
    mrs x0, CurrentEL
    lsr x0, x0, #2
    cmp x0, #2
    b.ne 1f
    mov x0, #(1<<31)
    msr hcr_el2, x0
    isb
    adrp x0, __boot_stack_top
    add x0, x0, :lo12:__boot_stack_top
    msr sp_el2, x0
    adrp x0, 1f
    add x0, x0, :lo12:1f
    msr elr_el2, x0
    mov x0, #0x3c5
    msr spsr_el2, x0
    eret

1:  ldr x0, =__boot_stack_top
    mov sp, x0
    ldr x0, =__bss_start
    ldr x1, =__bss_end
    mov x2, #0
2:  cmp x0, x1
    b.hs 3f
    str x2, [x0], #8
    b 2b

3:  ldr x0, =exception_vectors
    msr vbar_el1, x0
    isb

    /* MMU 暂时跳过 */
    /* bl arch_mmu_enable */

    mov x0, x19
    ldr x1, =kernel_main
    br x1

    .section ".bss","aw",%nobits
    .align 12
__boot_stack_bottom: .space 0x8000
__boot_stack_top: