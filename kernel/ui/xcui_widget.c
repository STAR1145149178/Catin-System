#include "catin.h"
#include "xcui.h"
static const u16 wifi_l1[16]={0x0180,0x03C0,0x07E0,0x0E70,0x0C30,0x1C38,0x1818,0x381C,0x300C,0x700E,0x6006,0,0x0180,0x0180,0x0180,0};
static const u16 wifi_l2[16]={0,0,0x0FF0,0x1FF8,0x3C3C,0x381C,0x700E,0x6006,0,0,0x0180,0x03C0,0x03C0,0x0180,0,0};
static const u16 wifi_l3[16]={0,0,0,0,0x7FFE,0xFFFF,0xC003,0x8001,0,0x0180,0x03C0,0x03C0,0x0180,0,0,0};
static void draw_wifi_bmp(int x,int y,int st,xc_color_t on){
    xc_color_t dim=XC_RGBA(on.r,on.g,on.b,60);
    for(int r=0;r<16;r++) for(int c=0;c<16;c++){
        int b=0;
        if(wifi_l3[r]&(0x8000>>c))b=3;
        else if(wifi_l2[r]&(0x8000>>c))b=2;
        else if(wifi_l1[r]&(0x8000>>c))b=1;
        if(!b)continue;
        xc_pixel(x+c,y+r,(b<=st)?on:dim);
    }
    xc_fill(XC_RECT(x+7,y+13,3,3),on);
}
static void draw_signal(int x,int base_y,int lv,xc_color_t c){
    for(int i=0;i<4;i++){
        int hh=6+i*4;
        xc_color_t cc=(i<lv)?c:XC_RGBA(c.r,c.g,c.b,60);
        xc_fill(XC_RECT(x+i*6,base_y-hh,4,hh),cc);
    }
}
static const u8 bolt_bmp[8]={0x10,0x18,0x0C,0x06,0x0F,0x03,0x06,0x0C};
static xc_color_t contrast(xc_color_t c){ return XC_RGB(255-c.r,255-c.g,255-c.b); }
static void draw_battery(int x,int y,int w,int h,int pct,int chg){
    xc_color_t fg=xc_theme.text;
    xc_fill(XC_RECT(x,y,w,2),fg); xc_fill(XC_RECT(x,y+h-2,w,2),fg);
    xc_fill(XC_RECT(x,y,2,h),fg); xc_fill(XC_RECT(x+w-2,y,2,h),fg);
    xc_fill(XC_RECT(x+w,y+h/3,2,h/3),fg);
    if(pct<0)pct=0; if(pct>100)pct=100;
    int iw=w-8, fw=iw*pct/100;
    if(fw<0)fw=0; if(fw>iw)fw=iw;
    xc_color_t fc;
    if(pct<20) fc=xc_theme.danger;
    else if(chg) fc=xc_theme.success;
    else fc=fg;
    if(fw>0) xc_fill(XC_RECT(x+4,y+4,fw,h-8),fc);
    if(chg){
        int bx=x+w/2-2,by=y+h/2-4;
        xc_color_t bc=XC_RGB(255,245,140), bd=contrast(fc);
        for(int yy=0;yy<8;yy++) for(int xx=0;xx<5;xx++)
            if(bolt_bmp[yy]&(0x10>>xx)){
                xc_pixel(bx+xx-1,by+yy,bd); xc_pixel(bx+xx+1,by+yy,bd);
                xc_pixel(bx+xx,by+yy-1,bd); xc_pixel(bx+xx,by+yy+1,bd);
            }
        for(int yy=0;yy<8;yy++) for(int xx=0;xx<5;xx++)
            if(bolt_bmp[yy]&(0x10>>xx)) xc_pixel(bx+xx,by+yy,bc);
    }
}
void xc_status_bar(const xc_status_t *s){
    int W=fb_w(),H=56;
    xc_color_t fg=xc_theme.text, dim=xc_theme.text_dim;
    xc_fill(XC_RECT(0,0,W,H),xc_theme.bg);
    char tb[8]; ksnprintf(tb,sizeof(tb),"%02d:%02d",s->hour,s->minute);
    xc_text(24,12,tb,fg,2);
    if(s->carrier){ int cw=xc_text_w(s->carrier,1); xc_text(W/2-cw/2,20,s->carrier,dim,1); }
    int xr=W-24;
    char bb[8]; ksnprintf(bb,sizeof(bb),"%d%%",s->battery);
    int bw=xc_text_w(bb,2); xr-=bw; xc_text(xr,12,bb,fg,2); xr-=8;
    xr-=28; draw_battery(xr,21,26,14,s->battery,s->charging); xr-=10;
    if(s->wifi_on){ xr-=16; draw_wifi_bmp(xr,20,s->wifi_strength,fg); xr-=8; }
    xr-=26; draw_signal(xr,34,s->signal_level,fg);
    xc_fill(XC_RECT(0,H,W,1),xc_theme.divider);
}
void xc_button(xc_button_t *b,xc_input_t *in){
    xc_color_t bg,fg=xc_theme.text;
    int r=b->radius>0?b->radius:16;
    switch(b->style){
    case XC_BTN_PRIMARY: bg=b->pressed?xc_theme.primary_dark:xc_theme.primary; fg=XC_RGB(255,255,255); break;
    case XC_BTN_SECONDARY: bg=b->pressed?xc_theme.primary_dark:xc_theme.surface_alt; break;
    case XC_BTN_DANGER: bg=b->pressed?(xc_color_t){180,48,64,255}:xc_theme.danger; fg=XC_RGB(255,255,255); break;
    default: bg=b->pressed?xc_theme.surface_alt:(xc_color_t){0,0,0,0}; break;
    }
    if(!b->enabled) bg=(xc_color_t){60,64,78,200};
    xc_shadow(b->r,r,5,70); xc_round(b->r,r,bg);
    xc_text_center(b->r,b->label,fg,2);
    if(!b->enabled||in->consumed) return;
    if(in->down&&xc_rect_contains(b->r,in->x,in->y)){ b->pressed=1; in->consumed=1; }
    if(b->pressed&&in->up){
        if(in->clicked&&xc_rect_contains(b->r,in->click_x,in->click_y))
            if(b->on_click) b->on_click(b->user);
        b->pressed=0; in->consumed=1;
    }
}
void xc_switch(xc_switch_t *s,xc_input_t *in){
    int row_h=s->r.h, tw=88, th=48;
    int tx=s->r.x+s->r.w-tw-24, ty=s->r.y+(row_h-th)/2;
    if(s->icon){ xc_rect_t ib={s->r.x+16,s->r.y+(row_h-40)/2,40,40};
        xc_round(ib,12,xc_theme.surface_alt); xc_text_center(ib,s->icon,xc_theme.primary,2); }
    xc_text(s->r.x+72,s->r.y+22,s->title,xc_theme.text,2);
    if(s->subtitle) xc_text(s->r.x+72,s->r.y+50,s->subtitle,xc_theme.text_dim,1);
    int tg=s->on?100:0;
    if(s->anim<tg) s->anim=(s->anim+22>tg)?tg:s->anim+22;
    if(s->anim>tg) s->anim=(s->anim-22<tg)?tg:s->anim-22;
    xc_rect_t tr={tx,ty,tw,th};
    xc_color_t off=(xc_color_t){60,68,88,255},on=xc_theme.primary;
    xc_color_t c=xc_blend((xc_color_t){on.r,on.g,on.b,(u8)(s->anim*255/100)},
                          (xc_color_t){off.r,off.g,off.b,255});
    xc_round(tr,th/2,c);
    int kr=th/2-5;
    int kx=tx+5+kr+(tw-th)*s->anim/100;
    xc_circle(kx,ty+th/2,kr,XC_RGB(255,255,255));
    if(in->consumed) return;
    if(in->clicked&&xc_rect_contains(s->r,in->click_x,in->click_y)){
        in->consumed=1; s->on=!s->on;
        if(s->on_change) s->on_change(s->user,s->on);
    }
}
void xc_slider(xc_slider_t *s,xc_input_t *in){
    int th=12,ty=s->r.y+s->r.h/2-th/2;
    int tx=s->r.x+44,tw=s->r.w-88;
    if(s->icon){ xc_rect_t ib={s->r.x+4,s->r.y+(s->r.h-36)/2,36,36};
        xc_round(ib,10,xc_theme.surface_alt); xc_text_center(ib,s->icon,xc_theme.primary,2); }
    xc_round(XC_RECT(tx,ty,tw,th),th/2,(xc_color_t){50,58,78,255});
    int fw=tw*s->value/100;
    if(fw>0) xc_round(XC_RECT(tx,ty,fw,th),th/2,xc_theme.primary);
    int kx=tx+fw,ky=s->r.y+s->r.h/2;
    xc_circle(kx,ky,14,XC_RGB(255,255,255));
    xc_circle(kx,ky,10,xc_theme.primary);
    if(in->consumed) return;
    if(in->down&&xc_rect_contains(s->r,in->x,in->y)){ s->dragging=1; in->consumed=1; }
    if(s->dragging&&in->holding){
        int v=(in->x-tx)*100/(tw>0?tw:1);
        if(v<0)v=0; if(v>100)v=100;
        if(v!=s->value){ s->value=v; if(s->on_change) s->on_change(s->user,v); }
    }
    if(s->dragging&&!in->holding) s->dragging=0;
}
void xc_card(xc_rect_t r,const char*t,const char*sub){
    xc_shadow(r,24,8,90); xc_round(r,24,xc_theme.surface);
    int y=r.y+20;
    if(t){ xc_text(r.x+28,y,t,xc_theme.text,2); y+=32; }
    if(sub) xc_text(r.x+28,y,sub,xc_theme.text_dim,1);
}
void xc_nav_bar(int active,const char*const labels[],const char*const icons[],int n){
    int W=fb_w(),H=fb_h(),bh=96;
    xc_rect_t r={0,H-bh,W,bh};
    xc_shadow(r,0,4,100); xc_fill(r,xc_theme.surface);
    xc_fill(XC_RECT(0,r.y,W,1),xc_theme.divider);
    int slot=W/n;
    for(int i=0;i<n;i++){
        xc_rect_t sr={i*slot,r.y,slot,bh};
        xc_color_t fg=(i==active)?xc_theme.primary:xc_theme.text_dim;
        if(icons&&icons[i]){ int iw=xc_text_w(icons[i],2); xc_text(sr.x+(sr.w-iw)/2,sr.y+16,icons[i],fg,2); }
        if(labels&&labels[i]){ int lw=xc_text_w(labels[i],1); xc_text(sr.x+(sr.w-lw)/2,sr.y+60,labels[i],fg,1); }
    }
}
int xc_list_item(xc_rect_t r,const char*icon,const char*t,const char*v,xc_input_t *in){
    if(icon){ xc_rect_t ib={r.x+20,r.y+(r.h-44)/2,44,44};
        xc_round(ib,12,xc_theme.surface_alt); xc_text_center(ib,icon,xc_theme.primary,2); }
    xc_text(r.x+84,r.y+(r.h-32)/2,t,xc_theme.text,2);
    if(v){ int vw=xc_text_w(v,1); xc_text(r.x+r.w-24-vw,r.y+(r.h-16)/2,v,xc_theme.text_dim,1); }
    xc_fill(XC_RECT(r.x+24,r.y+r.h-1,r.w-48,1),xc_theme.divider);
    if(!in->consumed&&in->down&&xc_rect_contains(r,in->x,in->y)){ in->consumed=1; return 1; }
    return 0;
}
