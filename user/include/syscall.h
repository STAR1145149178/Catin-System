#ifndef _USER_SYSCALL_H
#define _USER_SYSCALL_H
#define SYS_exit 0
#define SYS_fork 1
#define SYS_wait 3
#define SYS_getpid 4
#define SYS_yield 5
#define SYS_sleep 6
#define SYS_read 8
#define SYS_write 7
#define SYS_open 9
#define SYS_close 10
#if defined(__aarch64__)
static inline long __sc0(long n){
    register long x8 __asm__("x8") = n;
    register long x0 __asm__("x0");
    __asm__ volatile("svc 0" : "=r"(x0) : "r"(x8) : "memory");
    return x0;
}
static inline long __sc1(long n, long a){
    register long x8 __asm__("x8") = n;
    register long x0 __asm__("x0") = a;
    __asm__ volatile("svc 0" : "+r"(x0) : "r"(x8) : "memory");
    return x0;
}
static inline long __sc2(long n, long a, long b){
    register long x8 __asm__("x8") = n;
    register long x0 __asm__("x0") = a;
    register long x1 __asm__("x1") = b;
    __asm__ volatile("svc 0" : "+r"(x0) : "r"(x8), "r"(x1) : "memory");
    return x0;
}
static inline long __sc3(long n, long a, long b, long c){
    register long x8 __asm__("x8") = n;
    register long x0 __asm__("x0") = a;
    register long x1 __asm__("x1") = b;
    register long x2 __asm__("x2") = c;
    __asm__ volatile("svc 0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2) : "memory");
    return x0;
}
#elif defined(__arm__)
static inline long __sc0(long n){
    register long r7 __asm__("r7") = n;
    register long r0 __asm__("r0");
    __asm__ volatile("svc 0" : "=r"(r0) : "r"(r7) : "memory");
    return r0;
}
static inline long __sc1(long n, long a){
    register long r7 __asm__("r7") = n;
    register long r0 __asm__("r0") = a;
    __asm__ volatile("svc 0" : "+r"(r0) : "r"(r7) : "memory");
    return r0;
}
static inline long __sc2(long n, long a, long b){
    register long r7 __asm__("r7") = n;
    register long r0 __asm__("r0") = a;
    register long r1 __asm__("r1") = b;
    __asm__ volatile("svc 0" : "+r"(r0) : "r"(r7), "r"(r1) : "memory");
    return r0;
}
static inline long __sc3(long n, long a, long b, long c){
    register long r7 __asm__("r7") = n;
    register long r0 __asm__("r0") = a;
    register long r1 __asm__("r1") = b;
    register long r2 __asm__("r2") = c;
    __asm__ volatile("svc 0" : "+r"(r0) : "r"(r7), "r"(r1), "r"(r2) : "memory");
    return r0;
}
#elif defined(__x86_64__)
static inline long __sc0(long n){ long r; __asm__ volatile("syscall":"=a"(r):"a"(n):"rcx","r11","memory"); return r; }
static inline long __sc1(long n, long a){ long r; __asm__ volatile("syscall":"=a"(r):"a"(n),"D"(a):"rcx","r11","memory"); return r; }
static inline long __sc2(long n, long a, long b){ long r; __asm__ volatile("syscall":"=a"(r):"a"(n),"D"(a),"S"(b):"rcx","r11","memory"); return r; }
static inline long __sc3(long n, long a, long b, long c){ long r; __asm__ volatile("syscall":"=a"(r):"a"(n),"D"(a),"S"(b),"d"(c):"rcx","r11","memory"); return r; }
#elif defined(__i386__)
static inline long __sc0(long n){ long r; __asm__ volatile("int $0x80":"=a"(r):"a"(n):"memory"); return r; }
static inline long __sc1(long n, long a){ long r; __asm__ volatile("int $0x80":"=a"(r):"a"(n),"b"(a):"memory"); return r; }
static inline long __sc2(long n, long a, long b){ long r; __asm__ volatile("int $0x80":"=a"(r):"a"(n),"b"(a),"c"(b):"memory"); return r; }
static inline long __sc3(long n, long a, long b, long c){ long r; __asm__ volatile("int $0x80":"=a"(r):"a"(n),"b"(a),"c"(b),"d"(c):"memory"); return r; }
#endif
#endif
