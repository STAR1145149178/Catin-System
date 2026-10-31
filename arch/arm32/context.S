    .section ".text","ax"
    .global arch_context_switch
    .global arch_enter_user
    .arm
arch_context_switch:
    stmdb sp!, {r4-r11, lr}
    str sp, [r0]
    mov sp, r1
    ldmia sp!, {r4-r11, lr}
    bx lr
arch_enter_user:
    mov r12, r0
    ldr r1, [r12, #4*31]
    msr cpsr_c, #0x10
    ldr sp, [r12, #4*31]
    ldr lr, [r12, #4*32]
    mov r0, r12
    ldmia r0!, {r0-r12}
    bx lr
