ENTRY(_start)
SECTIONS {
    . = 0x100000;
    .multiboot : ALIGN(8) { *(.multiboot) }
    .text : ALIGN(4096) { *(.text.boot) *(.text*) }
    .rodata : ALIGN(4096) { *(.rodata*) }
    .data : ALIGN(4096) { *(.data*) }
    .bss : ALIGN(4096) { __bss_start = .; *(.bss*) *(COMMON) __bss_end = .; }
    . = ALIGN(4096); __kernel_end = .;
}
