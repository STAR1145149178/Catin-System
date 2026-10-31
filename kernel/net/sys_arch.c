#include "catin.h"
#include "lwip/sys.h"

// =========================================================
// 1. 裸机临界区保护 (NO_SYS=1 下实际用不到，但为了链接必须写)
// =========================================================
sys_prot_t sys_arch_protect(void) {
    // 在纯裸机单线程环境里，不需要真的去关中断
    return 1; 
}

void sys_arch_unprotect(sys_prot_t pval) {
    (void)pval;
}

// =========================================================
// 2. lwIP 核心依赖：获取系统时间 (极其重要！)
// lwIP 需要毫秒级的时间来计算 TCP 超时重传、ARP 老化等
// =========================================================
u32_t sys_now(void) {
    // 你的 arch_timer_ticks() 已经被初始化为 1000Hz，即 1 tick = 1ms
    return (u32_t)arch_timer_ticks();
}