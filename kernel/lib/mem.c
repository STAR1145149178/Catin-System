#include "types.h"
typedef unsigned long ulong_t;

void *memset(void *s, int c, ulong_t n){
    unsigned char *p = (unsigned char*)s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

void *memcpy(void *d, const void *s, ulong_t n){
    unsigned char *a = (unsigned char*)d;
    const unsigned char *b = (const unsigned char*)s;
    while (n--) *a++ = *b++;
    return d;
}

// lwIP 点名要的 memmove
void *memmove(void *dest, const void *src, ulong_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}

int memcmp(const void *a, const void *b, ulong_t n){
    const unsigned char *x = (const unsigned char*)a;
    const unsigned char *y = (const unsigned char*)b;
    while (n--) { if (*x != *y) return (int)*x - (int)*y; x++; y++; }
    return 0;
}
void raise(int sig){ (void)sig; }
long __aeabi_ldiv0(long a, long b){ (void)b; return a; }
int  __aeabi_idiv0(int a, int b){ (void)b; return a; }