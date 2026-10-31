#ifndef _ARCH_H
#define _ARCH_H
#include "types.h"
#if defined(__aarch64__)
  #define ARCH_ID 1
  #define ARCH_NAME "arm64-v8a"
#elif defined(__arm__)
  #define ARCH_ID 2
  #define ARCH_NAME "armeabi-v7a"
#elif defined(__x86_64__)
  #define ARCH_ID 3
  #define ARCH_NAME "x86_64"
#elif defined(__i386__)
  #define ARCH_ID 4
  #define ARCH_NAME "x86"
#endif

typedef struct regs {
    u64 r[16];
    u64 pc;
    u64 pstate;
    u64 sp_user;
    u64 syscall_nr;
    u64 args[6];
} regs_t;

void arch_early_init(void);
void arch_mmu_enable(void);
void arch_irq_init(void);
void arch_irq_enable(int irq);
void arch_irq_enable_all(void);
void arch_irq_disable_all(void);
void arch_timer_init(u32 hz);
u64  arch_timer_ticks(void);
u32  arch_timer_hz(void);
void arch_uart_init(void);
void arch_uart_putc(char c);
int  arch_uart_rx_ready(void);
int  arch_uart_getc(void);
void arch_halt(void);
void arch_wfi(void);
void arch_context_switch(u64 *save_ksp, u64 new_ksp);
void arch_enter_user(regs_t *r) __attribute__((noreturn));
void arch_irq_handler(regs_t *r);
void arch_syscall_handler(regs_t *r);
void arch_panic_dump(regs_t *r) __attribute__((noreturn));

static inline void mmio_w32(u64 a, u32 v){ *(volatile u32*)a = v; }
static inline u32  mmio_r32(u64 a){ return *(volatile u32*)a; }
static inline void mmio_w64(u64 a, u64 v){ *(volatile u64*)a = v; }
static inline u64  mmio_r64(u64 a){ return *(volatile u64*)a; }
#endif
