    .section ".text","ax"
    .code32
    .global arch_syscall_stub
    .global arch_irq_stub
arch_syscall_stub:
    pusha
    mov %esp, %eax
    push %eax
    call arch_syscall_handler
    add $4, %esp
    popa
    iret
arch_irq_stub:
    pusha
    mov %esp, %eax
    push %eax
    call arch_irq_handler
    add $4, %esp
    popa
    iret
