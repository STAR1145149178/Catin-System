    .section ".multiboot","a"
    .align 4
mb_hdr:
    .long 0x1BADB002
    .long 0
    .long -(0x1BADB002 + 0)
    .section ".text.boot","ax"
    .code32
    .global _start
_start:
    cli
    mov %ebx, %edi
    mov $__boot_stack_top, %esp
    lea __boot_pd, %eax
    movl $0x83, (%eax)
    mov %eax, %cr3
    /* CR4: PSE | OSFXSR | OSXMMEXCPT */
    mov %cr4, %eax
    or $0x610, %eax
    mov %eax, %cr4
    mov %cr0, %eax
    or $0x80000001, %eax
    mov %eax, %cr0
    mov %edi, %eax
    push %eax
    call kernel_main
    cli
    hlt
    .section ".bss","aw",%nobits
    .align 4096
__boot_pd: .space 4096
__boot_stack_bottom: .space 0x8000
__boot_stack_top: