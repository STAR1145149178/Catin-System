    .section ".text","ax"
    .code64
    .global arch_syscall_stub
    .global arch_irq_stub
arch_syscall_stub:
    swapgs
    push %r11
    push %rcx
    push %r10
    push %r9
    push %r8
    push %rdx
    push %rsi
    push %rdi
    push %rax
    mov %rsp, %rdi
    call arch_syscall_handler
    pop %rax
    pop %rdi
    pop %rsi
    pop %rdx
    pop %r8
    pop %r9
    pop %r10
    pop %rcx
    pop %r11
    swapgs
    sysretq
arch_irq_stub:
    push $0
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15
    mov %rsp, %rdi
    call arch_irq_handler
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax
    add $8, %rsp
    iretq
