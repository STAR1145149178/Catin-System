    .section ".text","ax"
    .global arch_mmu_enable
arch_mmu_enable:
    /* MAIR: Attr0=Device, Attr1=Normal WB */
    mov x0, #0xFF
    msr mair_el1, x0
    isb

    /* L0[0] = __l1_tt | 有效 */
    ldr x0, =__l0_tt
    ldr x1, =__l1_tt
    orr x1, x1, #0x3
    str x1, [x0]

    /* 填 L1 的 16 个 1GB 块: Normal WB, AF=1, SH=3, AttrIndx=1 */
    ldr x0, =__l1_tt
    mov x2, #0
    mov x3, #16
    ldr x4, =0x0000000000000705
1:  mov x5, x2, lsl #30
    orr x5, x5, x4
    str x5, [x0, x2, lsl #3]
    add x2, x2, #1
    cmp x2, x3
    b.lt 1b

    /* TCR_EL1 = 0x2B5593519
       T0SZ=25, EPD0=0, IRGN0=1, ORGN0=1, SH0=3, TG0=0
       T1SZ=25, EPD1=1(禁用), IRGN1=1, ORGN1=1, SH1=3, TG1=2
       IPS=2 (40-bit PA)
    */
    ldr x0, =0x00000002B5593519
    msr tcr_el1, x0
    isb

    ldr x0, =__l0_tt
    msr ttbr0_el1, x0
    isb

    tlbi vmalle1
    dsb sy
    isb

    mrs x0, sctlr_el1
    orr x0, x0, #(1 << 0)
    orr x0, x0, #(1 << 2)
    orr x0, x0, #(1 << 12)
    msr sctlr_el1, x0
    isb
    ret

    .section ".bss","aw",%nobits
    .align 12
__l0_tt: .space 4096
__l1_tt: .space 4096