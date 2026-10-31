#include "catin.h"
#include "xcui.h"
static const struct { const char*t,*b,*a; int r; } notifs[]={
    {"系统更新","Catin 2.2.0","System",0},
    {"新消息","你有一条未读消息","Messages",0},
    {"电量提醒","当前 78%","System",1},
    {"WiFi","CATIN-5G","Network",1},
};
#define N (sizeof(notifs)/sizeof(notifs[0]))
void panel_notif_draw(xc_input_t *in,int oy){
    int W=fb_w(),H=fb_h();
    xc_push_clip(XC_RECT(0,0,W,H));
    xc_fill(XC_RECT(0,oy,W,H),(xc_color_t){20,24,34,250});
    xc_fill(XC_RECT(0,oy,W,100),(xc_color_t){26,30,42,255});
    xc_text(24,oy+34,"通知",xc_theme.text,3);
    xc_text(W-160,oy+40,"全部清除",xc_theme.primary,1);
    xc_fill(XC_RECT(0,oy+100,W,1),xc_theme.divider);
    int y=oy+120;
    for(usize i=0;i<N;i++){
        xc_rect_t r={20,y,W-40,120};
        xc_round(r,18,notifs[i].r?(xc_color_t){28,32,44,255}:xc_theme.surface);
        if(!notifs[i].r) xc_circle(40,y+60,6,xc_theme.primary);
        xc_text(60,y+20,notifs[i].a,xc_theme.text_dim,1);
        xc_text(60,y+44,notifs[i].t,xc_theme.text,2);
        xc_text(60,y+80,notifs[i].b,xc_theme.text_dim,1);
        y+=136; if(y>oy+H-200) break;
    }
    xc_text(W/2-90,oy+H-120,"底部上滑关闭",xc_theme.text_dim,1);
    if(in->holding||in->x||in->y){
        xc_color_t c=in->holding?xc_theme.primary:(xc_color_t){200,220,255,180};
        xc_circle(in->x,in->y,12,c); xc_circle(in->x,in->y,4,XC_RGB(255,255,255));
    }
    xc_pop_clip();
}
void panel_ctrl_draw(xc_input_t *in,int oy,int*wf,int*bt,int*br){
    int W=fb_w(),H=fb_h();
    xc_push_clip(XC_RECT(0,0,W,H));
    xc_fill(XC_RECT(0,oy,W,H),(xc_color_t){20,24,34,250});
    xc_fill(XC_RECT(0,oy,W,100),(xc_color_t){26,30,42,255});
    xc_text(24,oy+34,"控制中心",xc_theme.text,3);
    xc_fill(XC_RECT(0,oy+100,W,1),xc_theme.divider);
    int y0=oy+130,cw=(W-80)/2,ch=140;
    xc_rect_t w1={20,y0,cw,ch};
    xc_round(w1,20,*wf?xc_theme.primary:xc_theme.surface);
    xc_text_center(XC_RECT(w1.x,w1.y+20,cw,40),XC_ICON_WIFI,*wf?XC_RGB(255,255,255):xc_theme.text_dim,3);
    xc_text_center(XC_RECT(w1.x,w1.y+80,cw,30),"WiFi",*wf?XC_RGB(255,255,255):xc_theme.text,2);
    if(in->clicked&&xc_rect_contains(w1,in->click_x,in->click_y)){ *wf=!*wf; in->consumed=1; }
    xc_rect_t w2={20+cw+40,y0,cw,ch};
    xc_round(w2,20,*bt?xc_theme.primary:xc_theme.surface);
    xc_text_center(XC_RECT(w2.x,w2.y+20,cw,40),XC_ICON_BT,*bt?XC_RGB(255,255,255):xc_theme.text_dim,3);
    xc_text_center(XC_RECT(w2.x,w2.y+80,cw,30),"蓝牙",*bt?XC_RGB(255,255,255):xc_theme.text,2);
    if(in->clicked&&xc_rect_contains(w2,in->click_x,in->click_y)){ *bt=!*bt; in->consumed=1; }
    xc_rect_t cb={20,y0+ch+30,W-40,200}; xc_card(cb,"亮度",NULL);
    xc_slider_t sl={.r={cb.x+16,cb.y+70,cb.w-32,80},.icon=XC_ICON_MOON,.value=*br};
    xc_slider(&sl,in); *br=sl.value;
    xc_text(W/2-90,oy+H-120,"底部上滑关闭",xc_theme.text_dim,1);
    if(in->holding||in->x||in->y){
        xc_color_t c=in->holding?xc_theme.primary:(xc_color_t){200,220,255,180};
        xc_circle(in->x,in->y,12,c); xc_circle(in->x,in->y,4,XC_RGB(255,255,255));
    }
    xc_pop_clip();
}
