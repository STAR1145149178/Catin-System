#include "catin.h"
#if ARCH_ID == 3 || ARCH_ID == 4
static inline void outb(u16 p, u8 v){ __asm__ volatile("outb %0,%1"::"a"(v),"Nd"(p)); }
static inline u8  inb(u16 p){ u8 v; __asm__ volatile("inb %1,%0":"=a"(v):"Nd"(p)); return v; }
static inline void io_wait(void){ outb(0x80, 0); }

#define COM1 0x3F8
void arch_uart_init(void){
    outb(COM1+1, 0x00);
    outb(COM1+3, 0x80);
    outb(COM1+0, 0x03);
    outb(COM1+1, 0x00);
    outb(COM1+3, 0x03);
    outb(COM1+2, 0xC7);
    outb(COM1+4, 0x0B);
}
void arch_uart_putc(char c){
    while (!(inb(COM1+5) & 0x20));
    outb(COM1, (u8)c);
}
int arch_uart_rx_ready(void){ return (inb(COM1+5) & 1) != 0; }
int arch_uart_getc(void){ while(!arch_uart_rx_ready()); return inb(COM1); }

void arch_irq_init(void){
    outb(0x20, 0x11); io_wait();
    outb(0xA0, 0x11); io_wait();
    outb(0x21, 0x20); io_wait();
    outb(0xA1, 0x28); io_wait();
    outb(0x21, 0x04); io_wait();
    outb(0xA1, 0x02); io_wait();
    outb(0x21, 0x01); io_wait();
    outb(0xA1, 0x01); io_wait();
    outb(0x21, 0xFF); io_wait();
    outb(0xA1, 0xFF); io_wait();
    log_info("pic: ready");
}
void arch_irq_enable(int irq){
    if (irq < 8){
        u8 m = inb(0x21) & ~(1 << irq);
        outb(0x21, m);
    } else if (irq < 16){
        u8 m = inb(0xA1) & ~(1 << (irq - 8));
        outb(0xA1, m);
        outb(0x21, inb(0x21) & ~(1 << 2));
    }
}
void arch_irq_enable_all(void){ __asm__ volatile("sti":::"memory"); }
void arch_irq_disable_all(void){ __asm__ volatile("cli":::"memory"); }

static volatile u64 g_ticks = 0;
static u32 g_hz = 0;
void arch_timer_init(u32 hz){
    g_hz = hz ? hz : 1000;
    u32 div = 1193182 / g_hz;
    if (div > 0xFFFF) div = 0xFFFF;
    if (div < 1) div = 1;
    outb(0x43, 0x36);
    outb(0x40, (u8)(div & 0xFF));
    outb(0x40, (u8)(div >> 8));
    g_ticks = 0;
    log_info("pit: %u Hz", g_hz);
}
u64 arch_timer_ticks(void){ return g_ticks; }
u32 arch_timer_hz(void){ return g_hz; }
void x86_timer_tick(void){ g_ticks++; }

void arch_halt(void){ for(;;) __asm__ volatile("hlt"); }
void arch_wfi(void){ __asm__ volatile("hlt"); }
void arch_early_init(void){}
void arch_mmu_enable(void){}

void arch_irq_handler(regs_t *r){
    u32 irq = (u32)r->r[15] & 0xFF;
    if (irq == 0) x86_timer_tick();
    if (irq >= 8) outb(0xA0, 0x20);
    outb(0x20, 0x20);
    proc_timer_tick();
    (void)r;
}
void arch_syscall_handler(regs_t *r){
    proc_save_regs(r);
    u64 ret = syscall_dispatch(r->syscall_nr, r->args[0], r->args[1],
                                r->args[2], r->args[3], r->args[4], r->args[5]);
    r->r[0] = ret;
}
#endif
