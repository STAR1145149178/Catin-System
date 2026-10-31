    .section ".multiboot","a"
    .align 8
mb2_hdr:
    .long 0xE85250D6
    .long 0
    .long mb2_end - mb2_hdr
    .long -(0xE85250D6 + 0 + (mb2_end - mb2_hdr))
    .align 8
    .short 0
    .short 0
    .long 8
mb2_end:
    .section ".text.boot","ax"
    .code32
    .global _start
_start:
    cli
    mov %ebx, %edi
    lea __boot_pml4, %eax
    lea __boot_pdpt, %ecx
    lea __boot_pd, %edx
    mov $0, %esi
1:  mov %esi, (%eax)
    mov %esi, (%ecx)
    mov %esi, (%edx)
    add $8, %eax
    add $8, %ecx
    add $8, %edx
    cmp $4096, %eax
    jne 1b
    lea __boot_pml4, %eax
    lea __boot_pdpt, %ecx
    or $3, %ecx
    mov %ecx, (%eax)
    lea __boot_pdpt, %ecx
    lea __boot_pd, %edx
    or $3, %edx
    mov %edx, (%ecx)
    lea __boot_pd, %edx
    mov $0, %eax
    mov $512, %ecx
2:  mov %eax, %esi
    or $0x83, %esi
    mov %esi, (%edx)
    add $0x200000, %eax
    add $8, %edx
    loop 2b
    /* CR4: PAE | PGE | OSFXSR | OSXMMEXCPT */
    mov %cr4, %eax
    or $0xA00, %eax
    mov %eax, %cr4
    lea __boot_pml4, %eax
    mov %eax, %cr3
    mov $0xC0000080, %ecx
    rdmsr
    or $0x100, %eax
    wrmsr
    mov %cr0, %eax
    or $0x80000001, %eax
    mov %eax, %cr0
    lgdt gdt64_ptr
    ljmp $0x08, $long_mode
    .code64
long_mode:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    movabs $__boot_stack_top, %rsp
    mov %edi, %edi
    mov %rdi, %rdi
    call kernel_main
    cli
    hlt
1:  jmp 1b
    .align 8
gdt64:
    .quad 0
    .quad 0x00AF9A000000FFFF
    .quad 0x00AF92000000FFFF
gdt64_ptr:
    .word gdt64_ptr - gdt64 - 1
    .quad gdt64
    .section ".bss","aw",%nobits
    .align 4096
__boot_pml4: .space 4096
__boot_pdpt: .space 4096
__boot_pd: .space 4096
__boot_stack_bottom: .space 0x8000
__boot_stack_top: