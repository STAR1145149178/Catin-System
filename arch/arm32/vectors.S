    .section ".text.vectors","ax"
    .align 5
    .global exception_vectors
    .arm
exception_vectors:
    b arm_reset
    b arm_undef
    b arm_svc
    b arm_abort
    b arm_reserved
    nop
    b arm_irq
    b arm_fiq
arm_reset: b arm_reset
arm_undef: b arm_undef
arm_abort: b arm_abort
arm_reserved: b arm_reserved
arm_fiq: b arm_fiq
arm_svc:
    stmdb sp!, {r0-r12, lr}
    mov r0, sp
    bl  arch_syscall_handler
    ldmia sp!, {r0-r12, lr}
    movs pc, lr
arm_irq:
    sub lr, lr, #4
    stmdb sp!, {r0-r12, lr}
    mov r0, sp
    bl  arch_irq_handler
    ldmia sp!, {r0-r12, lr}
    subs pc, lr, #4
