ENTRY(_start)
SECTIONS {
    . = 0x40008000;
    .text : ALIGN(4096) { *(.text.boot) *(.text.vectors) *(.text*) }
    .rodata : ALIGN(4096) { *(.rodata*) }
    .data : ALIGN(4096) { *(.data*) }
    .bss : ALIGN(4096) { __bss_start = .; *(.bss*) *(COMMON) __bss_end = .; }
    . = ALIGN(4096); __kernel_end = .;
}
