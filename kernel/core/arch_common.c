#include "catin.h"

static void put_str(const char *s){
    while (*s) arch_uart_putc(*s++);
}

static void put_hex(u64 v){
    const char *d = "0123456789abcdef";
    arch_uart_putc('0');
    arch_uart_putc('x');
    for (int i = 15; i >= 0; i--)
        arch_uart_putc(d[(v >> (i*4)) & 0xF]);
}

void arch_panic_dump(regs_t *r) {
    arch_irq_disable_all();

#if ARCH_ID == 1
    u64 esr = 0, far = 0, elr = 0;
    __asm__ volatile("mrs %0, esr_el1" : "=r"(esr));
    __asm__ volatile("mrs %0, far_el1" : "=r"(far));
    __asm__ volatile("mrs %0, elr_el1" : "=r"(elr));
    put_str("\n\n===== PANIC (arm64) =====\n");
    put_str("ESR = "); put_hex(esr); put_str("\n");
    put_str("FAR = "); put_hex(far); put_str("\n");
    put_str("ELR = "); put_hex(elr); put_str("\n");
#else
    put_str("\n\n===== PANIC =====\n");
#endif

    if (r){
        for (int i = 0; i < 16; i++){
            put_str("r");
            char num[3];
            num[0] = '0' + (i / 10);
            num[1] = '0' + (i % 10);
            num[2] = 0;
            put_str(num);
            put_str(" = ");
            put_hex(r->r[i]);
            put_str("\n");
        }
        put_str("pc = "); put_hex(r->pc); put_str("\n");
        put_str("sp = "); put_hex(r->sp_user); put_str("\n");
    }

    put_str("\nHALTED.\n");
    arch_halt();
}

void uart_init(void){ arch_uart_init(); }

void log_info(const char *f,...){
    char b[256]; __builtin_va_list ap; __builtin_va_start(ap,f);
    extern int kvsnprintf(char*,usize,const char*,__builtin_va_list);
    kvsnprintf(b,sizeof(b),f,ap); __builtin_va_end(ap);
    uart_puts("[I] "); uart_puts(b); arch_uart_putc('\n');
}

void log_err(const char *f,...){
    char b[256]; __builtin_va_list ap; __builtin_va_start(ap,f);
    extern int kvsnprintf(char*,usize,const char*,__builtin_va_list);
    kvsnprintf(b,sizeof(b),f,ap); __builtin_va_end(ap);
    uart_puts("[E] "); uart_puts(b); arch_uart_putc('\n');
}