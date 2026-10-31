    .section ".text","ax"
.global arch_context_switch
arch_context_switch:
    sub sp, sp, #(16*13)
    stp x19,x20,[sp,#16*0]
    stp x21,x22,[sp,#16*1]
    stp x23,x24,[sp,#16*2]
    stp x25,x26,[sp,#16*3]
    stp x27,x28,[sp,#16*4]
    stp x29,x30,[sp,#16*5]
    mov x2, sp
    str x2, [x0]
    mov sp, x1
    ldp x19,x20,[sp,#16*0]
    ldp x21,x22,[sp,#16*1]
    ldp x23,x24,[sp,#16*2]
    ldp x25,x26,[sp,#16*3]
    ldp x27,x28,[sp,#16*4]
    ldp x29,x30,[sp,#16*5]
    add sp, sp, #(16*13)
    ret
.global arch_enter_user
arch_enter_user:
    mov x16, x0
    ldr x17, [x16, #8*31]
    msr sp_el0, x17
    ldr x17, [x16, #8*32]
    msr elr_el1, x17
    ldr x17, [x16, #8*33]
    msr spsr_el1, x17
    ldp x0,x1,[x16,#8*0]
    ldp x2,x3,[x16,#8*2]
    ldp x4,x5,[x16,#8*4]
    ldp x6,x7,[x16,#8*6]
    ldp x8,x9,[x16,#8*8]
    ldp x10,x11,[x16,#8*10]
    ldp x12,x13,[x16,#8*12]
    ldp x14,x15,[x16,#8*14]
    ldp x18,x19,[x16,#8*18]
    ldp x20,x21,[x16,#8*20]
    ldp x22,x23,[x16,#8*22]
    ldp x24,x25,[x16,#8*24]
    ldp x26,x27,[x16,#8*26]
    ldp x28,x29,[x16,#8*28]
    ldr x30,[x16,#8*30]
    ldr x17,[x16,#8*17]
    ldr x16,[x16,#8*16]
    eret
