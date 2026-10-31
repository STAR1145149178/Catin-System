#ifndef LWIP_ARCH_CC_H
#define LWIP_ARCH_CC_H

#include "types.h"

// 强行定义 size_t 和 NULL 
typedef u64 size_t;
typedef s64 ptrdiff_t;
#ifndef NULL
#define NULL ((void*)0)
#endif

// 1. 基础数据类型
typedef u8    u8_t;
typedef s8    s8_t;
typedef u16   u16_t;
typedef s16   s16_t;
typedef u32   u32_t;
typedef s32   s32_t;
typedef u64   u64_t;
typedef s64   s64_t;
typedef u64   mem_ptr_t;

// 2. 打印格式
#define U16_F "hu"
#define S16_F "hd"
#define X16_F "hx"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"
#define SZT_F "lu"
#define U64_F "llu"
#define S64_F "lld"
#define X64_F "llx"

// 3. 补全 limits.h
#define CHAR_BIT 8
#define SCHAR_MIN (-128)
#define SCHAR_MAX 127
#define UCHAR_MAX 255
#define SHRT_MIN (-32768)
#define SHRT_MAX 32767
#define USHRT_MAX 65535
#define INT_MIN (-2147483647 - 1)
#define INT_MAX 2147483647
#define UINT_MAX 4294967295U

// 4. 字符串函数 (注意：kstrncpy 改成 void 了！)
usize kstrlen(const char *s);
void *kmemset(void *d, int c, usize n);
void *kmemcpy(void *d, const void *s, usize n);
int kstrcmp(const char *a, const char *b);
int kstrncmp(const char *a, const char *b, usize n);
void kstrncpy(char *d, const char *s, usize n);

// 5. 结构体打包与字节序
#define BYTE_ORDER LITTLE_ENDIAN
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

// 6. 日志
extern void log_info(const char *fmt, ...);
extern void log_err(const char *fmt, ...);
#define LWIP_PLATFORM_DIAG(x) do { log_info x; } while(0)
#define LWIP_PLATFORM_ASSERT(x) do { log_err("LWIP Assert: %s\n", x); for(;;); } while(0)

// 7. ctype 宏全删了，lwIP 内部会自己搞定！
// 放在 cc.h 最下面
typedef u32 sys_prot_t;
sys_prot_t sys_arch_protect(void);
void sys_arch_unprotect(sys_prot_t pval);
// 提供 lwIP 需要的随机数 (借用你的系统定时器)
extern u64 arch_timer_ticks(void);
#define LWIP_RAND() ((u32_t)arch_timer_ticks())
#endif /* LWIP_ARCH_CC_H */