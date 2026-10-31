    .section ".text","ax"
    .code32
    .global arch_context_switch
    .global arch_enter_user

arch_context_switch:
    push %ebx
    push %esi
    push %edi
    push %ebp
    mov %esp, (%eax)
    mov %edx, %esp
    pop %ebp
    pop %edi
    pop %esi
    pop %ebx
    ret

arch_enter_user:
    mov %eax, %ecx
    mov $0x23, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    push $0x23
    push 4*31(%ecx)
    push 4*33(%ecx)
    push $0x1B
    push 4*32(%ecx)
    mov 4*0(%ecx), %eax
    mov 4*1(%ecx), %ebx
    mov 4*2(%ecx), %edx
    mov 4*3(%ecx), %esi
    mov 4*4(%ecx), %edi
    mov 4*5(%ecx), %ebp
    iret