#include "catin.h"
usize kstrlen(const char *s){ usize n=0; while(s&&s[n])n++; return n; }
void *kmemset(void *d,int c,usize n){ u8*p=d; while(n--)*p++=(u8)c; return d; }
void *kmemcpy(void *d,const void*s,usize n){ u8*a=d;const u8*b=s; while(n--)*a++=*b++; return d; }
int kmemcmp(const void*a,const void*b,usize n){ const u8*x=a,*y=b; while(n--){ if(*x!=*y) return (int)*x-(int)*y; x++; y++; } return 0; }
int kstrcmp(const char*a,const char*b){ while(*a&&*a==*b){a++;b++;} return (int)(u8)*a-(int)(u8)*b; }
int kstrncmp(const char*a,const char*b,usize n){ while(n&&*a&&*a==*b){a++;b++;n--;} if(!n)return 0; return (int)(u8)*a-(int)(u8)*b; }
void kstrncpy(char*d,const char*s,usize n){ usize i=0; for(;i+1<n&&s[i];i++)d[i]=s[i]; d[i]=0; }
int katoi(const char *s) {
    int v = 0, n = 0;
    while (*s == ' ') s++;
    if (*s == '-') { n = 1; s++; }
    while (*s >= '0' && *s <= '9') v = v * 10 + (*s++ - '0');
    return n ? -v : v;
}