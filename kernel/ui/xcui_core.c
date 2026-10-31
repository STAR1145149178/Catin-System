#include "catin.h"
#include "xcui.h"
xc_theme_t xc_theme = {
    .bg={16,20,30,255}, .surface={30,36,52,255}, .surface_alt={44,52,72,255},
    .primary={88,156,255,255}, .primary_dark={56,108,200,255},
    .text={235,240,250,255}, .text_dim={140,152,176,255},
    .divider={44,52,72,255}, .success={72,200,120,255},
    .danger={240,80,96,255}, .warning={240,180,72,255}, .shadow={0,0,0,90},
};
#define CLIP_MAX 4
static xc_rect_t g_clip[CLIP_MAX]; static int g_clip_n;
void xc_push_clip(xc_rect_t r){
    if(g_clip_n>=CLIP_MAX) return;
    if(g_clip_n>0){
        xc_rect_t p=g_clip[g_clip_n-1];
        int x0=r.x>p.x?r.x:p.x, y0=r.y>p.y?r.y:p.y;
        int x1=r.x+r.w<p.x+p.w?r.x+r.w:p.x+p.w;
        int y1=r.y+r.h<p.y+p.h?r.y+r.h:p.y+p.h;
        if(x1<x0)x1=x0; if(y1<y0)y1=y0;
        r=(xc_rect_t){x0,y0,x1-x0,y1-y0};
    }
    g_clip[g_clip_n++]=r;
}
void xc_pop_clip(void){ if(g_clip_n>0) g_clip_n--; }
int xc_clip_has(void){ return g_clip_n>0; }
int xc_clip_x0(void){ return g_clip_n?g_clip[g_clip_n-1].x:0; }
int xc_clip_y0(void){ return g_clip_n?g_clip[g_clip_n-1].y:0; }
int xc_clip_x1(void){ return g_clip_n?g_clip[g_clip_n-1].x+g_clip[g_clip_n-1].w:fb_w(); }
int xc_clip_y1(void){ return g_clip_n?g_clip[g_clip_n-1].y+g_clip[g_clip_n-1].h:fb_h(); }
void xc_init(void){ xc_clear(xc_theme.bg); g_clip_n=0; }
void xc_begin(void){ g_clip_n=0; }
void xc_end(void){ fb_present(); }
void xc_clear(xc_color_t c){
    int w=fb_w(),h=fb_h(); u32*p=fb_backbuffer();
    u32 v=((u32)c.r<<16)|((u32)c.g<<8)|c.b;
    if(!p)return; for(int i=0;i<w*h;i++) p[i]=v;
}
void xc_pixel(int x,int y,xc_color_t c){
    int w=fb_w(),h=fb_h();
    if(x<0||y<0||x>=w||y>=h) return;
    if(g_clip_n){ xc_rect_t cl=g_clip[g_clip_n-1];
        if(x<cl.x||y<cl.y||x>=cl.x+cl.w||y>=cl.y+cl.h) return; }
    u32*p=fb_backbuffer(); if(!p) return;
    if(c.a==255) p[y*w+x]=((u32)c.r<<16)|((u32)c.g<<8)|c.b;
    else if(c.a){
        u32 o=p[y*w+x];
        u32 orr=(o>>16)&0xFF, og=(o>>8)&0xFF, ob=o&0xFF;
        p[y*w+x] = (((c.r*c.a+orr*(255-c.a))/255)<<16)|
                   (((c.g*c.a+og *(255-c.a))/255)<<8)|
                    ((c.b*c.a+ob *(255-c.a))/255);
    }
}
xc_color_t xc_blend(xc_color_t fg,xc_color_t bg){
    u32 a=fg.a; xc_color_t r;
    r.r=(u8)((fg.r*a+bg.r*(255-a))/255);
    r.g=(u8)((fg.g*a+bg.g*(255-a))/255);
    r.b=(u8)((fg.b*a+bg.b*(255-a))/255);
    r.a=255; return r;
}
int xc_rect_contains(xc_rect_t r,int px,int py){
    return px>=r.x&&py>=r.y&&px<r.x+r.w&&py<r.y+r.h;
}
