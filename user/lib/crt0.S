    .section ".text._start","ax"
#if defined(__aarch64__)
    .global _start
_start:
    bl  main
    mov x0, #0
    mov x8, #0
    svc 0
    b   .

    .global catin_sys
catin_sys:
    mov x8, x0
    mov x0, x1
    mov x1, x2
    mov x2, x3
    mov x3, x4
    mov x4, x5
    mov x5, x6
    svc 0
    ret
#endif
