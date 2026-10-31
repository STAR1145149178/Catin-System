#include "catin.h"

usize catin_strlen(const char*s){ usize n=0; while(s&&s[n])n++; return n; }
char *catin_strcpy(char*d,const char*s){ char*r=d; while((*r++=*s++)); return d; }
char *catin_strncpy(char*d,const char*s,usize n){ usize i=0; for(;i<n&&s[i];i++)d[i]=s[i]; for(;i<n;i++)d[i]=0; return d; }
int catin_strcmp(const char*a,const char*b){ while(*a&&*a==*b){a++;b++;} return (int)(u8)*a-(int)(u8)*b; }
int catin_strncmp(const char*a,const char*b,usize n){ while(n&&*a&&*a==*b){a++;b++;n--;} if(!n)return 0; return (int)(u8)*a-(int)(u8)*b; }
char *catin_strchr(const char*s,int c){ while(*s){if(*s==(char)c)return(char*)s;s++;} return (c==0)?(char*)s:NULL; }
char *catin_strrchr(const char*s,int c){ const char*r=NULL; while(*s){if(*s==(char)c)r=s;s++;} return (c==0)?(char*)s:(char*)r; }
char *catin_strstr(const char*h,const char*n){ if(!*n)return(char*)h; for(;*h;h++){ const char*a=h,*b=n; while(*a&&*b&&*a==*b){a++;b++;} if(!*b)return(char*)h; } return NULL; }
void *catin_memset(void*d,int c,usize n){ u8*p=d; while(n--)*p++=(u8)c; return d; }
void *catin_memcpy(void*d,const void*s,usize n){ u8*a=d;const u8*b=s; while(n--)*a++=*b++; return d; }
int catin_memcmp(const void*a,const void*b,usize n){ const u8*x=a,*y=b; while(n--){if(*x!=*y)return(int)*x-(int)*y;x++;y++;} return 0; }
char *catin_strcat(char*d,const char*s){ char*r=d; while(*r)r++; while((*r++=*s++)); return d; }
int catin_atoi(const char*s){ int v=0,n=0; while(*s==' ')s++; if(*s=='-'){n=1;s++;} while(*s>='0'&&*s<='9')v=v*10+(*s++-'0'); return n?-v:v; }
long catin_atol(const char*s){ long v=0; int n=0; while(*s==' ')s++; if(*s=='-'){n=1;s++;} while(*s>='0'&&*s<='9')v=v*10+(*s++-'0'); return n?-v:v; }
void catin_itoa(int v,char*out){ char t[16]; int i=0,n=0; if(v<0){n=1;v=-v;} if(!v)t[i++]='0'; while(v){t[i++]='0'+v%10;v/=10;} int j=0; if(n)out[j++]='-'; while(i--)out[j++]=t[i]; out[j]=0; }

// ====================================================
// 核心魔法：撤销掉头文件里的宏定义，给 GCC 提供原生实现
// ====================================================
#undef memcpy
#undef memset
#undef memcmp

void *memcpy(void *d, const void *s, unsigned long n) {
    return catin_memcpy(d, s, n);
}

void *memset(void *s, int c, unsigned long n) {
    return catin_memset(s, c, n);
}

int memcmp(const void *a, const void *b, unsigned long n) {
    return catin_memcmp(a, b, n);
}