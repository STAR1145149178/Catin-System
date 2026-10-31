#include "catin.h"
#include "xcui.h"
typedef struct { char t[96]; xc_toast_kind_t k; u64 exp; int on; } toast_t;
static toast_t toasts[XC_TOAST_MAX]; static int head, count;
void xc_toast(xc_toast_kind_t k, const char *f, ...){
    char b[96];
    __builtin_va_list ap; __builtin_va_start(ap,f);
    extern int kvsnprintf(char*,usize,const char*,__builtin_va_list);
    kvsnprintf(b,sizeof(b),f,ap); __builtin_va_end(ap);
    int idx;
    if(count<XC_TOAST_MAX){ idx=(head+count)%XC_TOAST_MAX; count++; }
    else { idx=head; head=(head+1)%XC_TOAST_MAX; }
    kstrncpy(toasts[idx].t,b,sizeof(toasts[idx].t));
    toasts[idx].k=k;
    toasts[idx].exp=arch_timer_ticks()+(XC_TOAST_MS*arch_timer_hz()/1000);
    toasts[idx].on=1;
}
void xc_toast_tick(void){
    u64 now=arch_timer_ticks();
    while(count>0&&toasts[head].on&&now>=toasts[head].exp){
        toasts[head].on=0; head=(head+1)%XC_TOAST_MAX; count--;
    }
}
void xc_toast_draw(void){
    if(count<=0) return;
    int W=fb_w(),y=76;
    for(int i=0;i<count;i++){
        int idx=(head+i)%XC_TOAST_MAX;
        if(!toasts[idx].on) continue;
        int w=xc_text_w(toasts[idx].t,2)+60; if(w>W-80)w=W-80;
        int x=W/2-w/2;
        xc_rect_t r={x,y,w,60};
        xc_color_t ac;
        switch(toasts[idx].k){
        case XC_TOAST_OK: ac=xc_theme.success; break;
        case XC_TOAST_WARN: ac=xc_theme.warning; break;
        case XC_TOAST_ERR: ac=xc_theme.danger; break;
        default: ac=xc_theme.primary; break;
        }
        xc_shadow(r,30,6,120);
        xc_round(r,30,(xc_color_t){28,32,44,240});
        xc_round(XC_RECT(r.x+8,r.y+12,6,r.h-24),3,ac);
        xc_text(r.x+30,r.y+(r.h-32)/2,toasts[idx].t,xc_theme.text,2);
        y+=72;
    }
}
