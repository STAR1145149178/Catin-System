#include "catin.h"
typedef __builtin_va_list va_t;
typedef struct { char *b, *e; } o_t;
static void oc(o_t*o,char c){ if(o->b<o->e-1) *o->b++=c; }
static void os(o_t*o,const char*s){ if(!s)s="(null)"; while(*s)oc(o,*s++); }
static void ou(o_t*o,u64 v,int base,int up){
    const char*d=up?"0123456789ABCDEF":"0123456789abcdef";
    char t[32]; int n=0;
    if(!v){ oc(o,'0'); return; }
    while(v&&n<31){ t[n++]=d[v%base]; v/=base; }
    while(n--) oc(o,t[n]);
}
int kvsnprintf(char *buf, usize cap, const char *fmt, va_t ap){
    o_t o = { buf, buf+cap };
    while (*fmt){
        if (*fmt != '%'){ oc(&o, *fmt++); continue; }
        fmt++;
        int l = 0;
        while (*fmt == 'l'){ l++; fmt++; }
        switch (*fmt){
        case 'd': case 'i': {
            s64 v;
            if(l>=2)v=__builtin_va_arg(ap,s64);
            else if(l==1)v=__builtin_va_arg(ap,long);
            else v=__builtin_va_arg(ap,int);
            if(v<0){ oc(&o,'-'); v=-v; }
            ou(&o,(u64)v,10,0); break;
        }
        case 'u': {
            u64 v; if(l>=2)v=__builtin_va_arg(ap,u64);
            else if(l==1)v=__builtin_va_arg(ap,unsigned long);
            else v=__builtin_va_arg(ap,u32);
            ou(&o,v,10,0); break;
        }
        case 'x': case 'X': {
            u64 v; if(l>=2)v=__builtin_va_arg(ap,u64);
            else if(l==1)v=__builtin_va_arg(ap,unsigned long);
            else v=__builtin_va_arg(ap,u32);
            ou(&o,v,16,*fmt=='X'); break;
        }
        case 'p': os(&o,"0x"); ou(&o,(u64)__builtin_va_arg(ap,void*),16,0); break;
        case 's': os(&o,__builtin_va_arg(ap,const char*)); break;
        case 'c': oc(&o,(char)__builtin_va_arg(ap,int)); break;
        case '%': oc(&o,'%'); break;
        default: oc(&o,'?'); break;
        }
        if (*fmt) fmt++;
    }
    *o.b = 0;
    return (int)(o.b - buf);
}
int ksnprintf(char *buf, usize cap, const char *fmt, ...){
    va_t ap; __builtin_va_start(ap, fmt);
    int n = kvsnprintf(buf, cap, fmt, ap);
    __builtin_va_end(ap);
    return n;
}
