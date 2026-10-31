#ifndef LWIP_ARCH_SYS_ARCH_H
#define LWIP_ARCH_SYS_ARCH_H

#include "types.h"

// 告诉 lwIP，我们的临界区保护状态用一个 32 位整数存
typedef u32 sys_prot_t;

// 声明保护函数（在裸机里其实啥也不用做）
sys_prot_t sys_arch_protect(void);
void sys_arch_unprotect(sys_prot_t pval);

#endif