#include "catin.h"

/* ==================== MSM8939 UARTDM 驱动 ==================== */
#define MSM_UART_BASE 0x078af000ULL

#define UARTDM_MR1       0x00
#define UARTDM_MR2       0x04
#define UARTDM_IPR       0x08
#define UARTDM_IMR       0x0C
#define UARTDM_SR        0x0C
#define UARTDM_CR        0x10
#define UARTDM_RF        0x14
#define UARTDM_TF        0x18
#define UARTDM_DMEN      0x3C
#define UARTDM_NCF_TX    0x40
#define UARTDM_RXFS      0x50

#define UARTDM_SR_TX_READY   (1 << 2)
#define UARTDM_SR_RX_READY   (1 << 0)

static inline u32 uart_rd(u32 off) { return mmio_r32(MSM_UART_BASE + off); }
static inline void uart_wr(u32 off, u32 val) { mmio_w32(MSM_UART_BASE + off, val); }

void arch_uart_init(void) {
    uart_wr(UARTDM_DMEN, 0);
    uart_wr(UARTDM_CR, 0x01); 
    uart_wr(UARTDM_CR, 0x00);
    uart_wr(UARTDM_IMR, 0);
}

void arch_uart_putc(char c) {
    while (!(uart_rd(UARTDM_SR) & UARTDM_SR_TX_READY));
    uart_wr(UARTDM_TF, (u32)c);
}

int arch_uart_rx_ready(void) {
    return (uart_rd(UARTDM_SR) & UARTDM_SR_RX_READY) != 0;
}

int arch_uart_getc(void) {
    while (!arch_uart_rx_ready());
    return (int)(uart_rd(UARTDM_RF) & 0xFF);
}

/* ==================== GICv2 中断控制器 ==================== */
static u64 GICD = 0x0b000000ULL;
static u64 GICC = 0x0b002000ULL;
static u32 g_lines = 0;

void arch_irq_init(void) {
    mmio_w32(GICD + 0x000, 0);
    u32 typer = mmio_r32(GICD + 0x004);
    u32 lines = ((typer & 0x1F) + 1) * 32;
    if (lines < 32) lines = 32;
    if (lines > 1024) lines = 1024;
    g_lines = lines;

    for (u32 i = 0; i < lines/32; i++) mmio_w32(GICD + 0x180 + i*4, 0xFFFFFFFFU);
    for (u32 i = 0; i < lines/4;  i++) mmio_w32(GICD + 0x400 + i*4, 0xA0A0A0A0U);
    for (u32 i = 0; i < lines/4;  i++) mmio_w32(GICD + 0x800 + i*4, 0x01010101U);

    mmio_w32(GICD + 0x000, 1);
    mmio_w32(GICC + 0x004, 0xFF);
    mmio_w32(GICC + 0x000, 1);
}

void arch_irq_enable(int irq) {
    if (irq < 0 || (u32)irq >= g_lines) return;
    mmio_w32(GICD + 0x100 + (irq/32)*4, 1U << (irq%32));
}

void arch_irq_enable_all(void) { __asm__ volatile("msr daifclr, #2" ::: "memory"); }
void arch_irq_disable_all(void) { __asm__ volatile("msr daifset, #2" ::: "memory"); }

/* ==================== ARM Generic Timer ==================== */
static volatile u64 g_ticks = 0;
static u32 g_hz = 0, g_period = 0;

void arch_timer_init(u32 hz) {
    g_hz = hz ? hz : 1000;
    u64 f; __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(f));
    g_period = (u32)(f / g_hz);
    __asm__ volatile("msr cntp_ctl_el0, %0" :: "r"(0ULL));
    __asm__ volatile("msr cntp_tval_el0, %0" :: "r"((u64)g_period));
    __asm__ volatile("msr cntp_ctl_el0, %0" :: "r"(1ULL));
    __asm__ volatile("isb");
    g_ticks = 0;
    log_info("timer: %u Hz", g_hz);
}

u64 arch_timer_ticks(void) { return g_ticks; }
u32 arch_timer_hz(void) { return g_hz; }

void arm_timer_tick_irq(void) {
    __asm__ volatile("msr cntp_tval_el0, %0" :: "r"((u64)g_period));
    g_ticks++;
}

/* ==================== 异常与中断分发 ==================== */
extern void touch_tick(void);
extern u32 g_virtio_irq;

void arch_irq_handler(regs_t *r) {
    u32 iar = mmio_r32(GICC + 0x0C);
    int irq = iar & 0x3FF;
    if (irq < 1020) {
        if (irq == 30) arm_timer_tick_irq();
        if (g_virtio_irq && (u32)irq == g_virtio_irq) touch_tick();
        mmio_w32(GICC + 0x10, iar);
    }
    proc_timer_tick();
    (void)r;
}

void arch_syscall_handler(regs_t *r) {
    proc_save_regs(r);
    u64 ret = syscall_dispatch(r->syscall_nr, r->args[0], r->args[1],
                                r->args[2], r->args[3], r->args[4], r->args[5]);
    r->r[0] = ret;
}

void arch_halt(void) { for(;;) __asm__ volatile("wfi"); }
void arch_wfi(void) { __asm__ volatile("wfi"); }
void arch_early_init(void) {}