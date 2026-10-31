    .section ".text","ax"
    .code64
    .global arch_context_switch
arch_context_switch:
    push %rbx
    push %rbp
    push %r12
    push %r13
    push %r14
    push %r15
    mov %rsp, (%rdi)
    mov %rsi, %rsp
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %rbp
    pop %rbx
    ret
.global arch_enter_user
arch_enter_user:
    mov %rdi, %rax
    mov $0x23, %cx
    mov %cx, %ds
    mov %cx, %es
    pushq $0x23
    pushq 8*31(%rax)
    pushq 8*33(%rax)
    pushq $0x1B
    pushq 8*32(%rax)
    mov 8*0(%rax), %rdi
    mov 8*1(%rax), %rsi
    mov 8*2(%rax), %rdx
    mov 8*4(%rax), %r10
    mov 8*8(%rax), %r8
    mov 8*9(%rax), %r9
    mov 8*3(%rax), %rcx
    mov 8*5(%rax), %r11
    mov 8*6(%rax), %rbx
    mov 8*7(%rax), %rbp
    mov 8*12(%rax), %r12
    mov 8*13(%rax), %r13
    mov 8*14(%rax), %r14
    mov 8*15(%rax), %r15
    mov 8*0(%rax), %rax
    iretq
