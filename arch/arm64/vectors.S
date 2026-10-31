    .section ".text.vectors","ax"
    .align 11
    .global exception_vectors
.macro SAVE
    sub sp, sp, #(8*28)
    stp x0,x1,[sp,#8*0]
    stp x2,x3,[sp,#8*2]
    stp x4,x5,[sp,#8*4]
    stp x6,x7,[sp,#8*6]
    stp x8,x9,[sp,#8*8]
    stp x10,x11,[sp,#8*10]
    stp x12,x13,[sp,#8*12]
    stp x14,x15,[sp,#8*14]
    stp x16,x17,[sp,#8*16]
    stp x18,x19,[sp,#8*18]
    stp x20,x21,[sp,#8*20]
    stp x22,x23,[sp,#8*22]
    stp x24,x25,[sp,#8*24]
    stp x26,x27,[sp,#8*26]
    mrs x0, sp_el0
    str x0, [sp,#8*4]
    mrs x0, elr_el1
    mrs x1, spsr_el1
    stp x0,x1,[sp,#8*5]
.endm
.macro RST
    ldp x0,x1,[sp,#8*0]
    ldp x2,x3,[sp,#8*2]
    ldr x4,[sp,#8*4]
    msr sp_el0, x4
    ldp x0,x1,[sp,#8*5]
    msr elr_el1, x0
    msr spsr_el1, x1
    ldp x0,x1,[sp,#8*0]
    ldp x2,x3,[sp,#8*2]
    ldp x4,x5,[sp,#8*4]
    ldp x6,x7,[sp,#8*6]
    ldp x8,x9,[sp,#8*8]
    ldp x10,x11,[sp,#8*10]
    ldp x12,x13,[sp,#8*12]
    ldp x14,x15,[sp,#8*14]
    ldp x16,x17,[sp,#8*16]
    ldp x18,x19,[sp,#8*18]
    ldp x20,x21,[sp,#8*20]
    ldp x22,x23,[sp,#8*22]
    ldp x24,x25,[sp,#8*24]
    ldp x26,x27,[sp,#8*26]
    add sp, sp, #(8*28)
.endm
exception_vectors:
    b panic_tramp
    .align 7
    b irq_tramp
    .align 7
    b lp
    .align 7
    b lp
    .align 7
    b panic_tramp
    .align 7
    b irq_tramp
    .align 7
    b lp
    .align 7
    b lp
    .align 7
    b el0_sync
    .align 7
    b irq_tramp
    .align 7
    b lp
    .align 7
    b lp
    .align 7
    b el0_sync
    .align 7
    b irq_tramp
    .align 7
    b lp
    .align 7
    b lp
panic_tramp:
    SAVE
    mov x0, sp
    bl arch_panic_dump
    b panic_tramp
irq_tramp:
    SAVE
    mov x0, sp
    bl arch_irq_handler
    RST
    eret
el0_sync:
    SAVE
    mrs x9, esr_el1
    lsr x9, x9, #26
    and x9, x9, #0x3F
    cmp x9, #0x15
    b.ne 1f
    ldr x9, [sp, #8*8]
    str x9, [sp, #8*13]
    ldr x10, [sp, #8*0]
    str x10, [sp, #8*14]
    ldr x10, [sp, #8*1]
    str x10, [sp, #8*15]
    ldr x10, [sp, #8*2]
    str x10, [sp, #8*16]
    ldr x10, [sp, #8*3]
    str x10, [sp, #8*17]
    ldr x10, [sp, #8*4]
    str x10, [sp, #8*18]
    ldr x10, [sp, #8*5]
    str x10, [sp, #8*19]
    mov x0, sp
    bl arch_syscall_handler
    ldr x9, [sp, #8*14]
    str x9, [sp, #8*0]
    RST
    eret
1:  mov x0, sp
    bl arch_panic_dump
    b 1b
lp: b lp
