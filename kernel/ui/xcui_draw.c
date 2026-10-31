#include "catin.h"
#include "xcui.h"
extern int xc_clip_has(void), xc_clip_x0(void), xc_clip_y0(void), xc_clip_x1(void), xc_clip_y1(void);
static inline int isq(int x){ if(x<=0)return 0; int r=0; while((r+1)*(r+1)<=x)r++; return r; }
static void hline(int x0,int x1,int y,xc_color_t c){
    int w=fb_w(),h=fb_h();
    if(y<0||y>=h) return;
    if(x0<0)x0=0; if(x1>w)x1=w;
    if(xc_clip_has()){
        int cy0=xc_clip_y0(), cy1=xc_clip_y1();
        if(y<cy0||y>=cy1) return;
        int cx0=xc_clip_x0(), cx1=xc_clip_x1();
        if(x0<cx0)x0=cx0; if(x1>cx1)x1=cx1;
    }
    if(x0>=x1) return;
    u32*p=fb_backbuffer(); if(!p) return;
    if(c.a==255){
        u32 v=((u32)c.r<<16)|((u32)c.g<<8)|c.b;
        u32*pp=p+(usize)y*w+x0;
        for(int i=x0;i<x1;i++)*pp++=v;
    } else if(c.a){
        u32*pp=p+(usize)y*w+x0;
        for(int i=x0;i<x1;i++){
            u32 o=*pp;
            u32 orr=(o>>16)&0xFF, og=(o>>8)&0xFF, ob=o&0xFF;
            *pp++=(((c.r*c.a+orr*(255-c.a))/255)<<16)|
                   (((c.g*c.a+og *(255-c.a))/255)<<8)|
                    ((c.b*c.a+ob *(255-c.a))/255);
        }
    }
}
void xc_fill(xc_rect_t r,xc_color_t c){ if(r.w<=0||r.h<=0)return; for(int y=0;y<r.h;y++) hline(r.x,r.x+r.w,r.y+y,c); }
void xc_round(xc_rect_t r,int rad,xc_color_t c){
    if(rad>r.w/2)rad=r.w/2; if(rad>r.h/2)rad=r.h/2;
    if(rad<=0){ xc_fill(r,c); return; }
    int r2=rad*rad;
    for(int y=0;y<r.h;y++){
        int dy=0;
        if(y<rad) dy=rad-1-y;
        else if(y>=r.h-rad) dy=y-(r.h-rad);
        int dx=0;
        if(dy>0){ int dy2=dy*dy; if(dy2<r2) dx=rad-isq(r2-dy2); else dx=rad; }
        hline(r.x+dx,r.x+r.w-dx,r.y+y,c);
    }
}
void xc_shadow(xc_rect_t r,int rad,int spr,u8 a){
    for(int s=1;s<=spr;s++){
        xc_rect_t rr={r.x-s,r.y-s,r.w+2*s,r.h+2*s};
        xc_color_t c={0,0,0,(u8)(a/(s+1))};
        xc_round(rr,rad+s,c);
    }
}
void xc_vgradient(xc_rect_t r,xc_color_t t,xc_color_t b){
    for(int y=0;y<r.h;y++){
        int tt=y*255/(r.h>1?r.h-1:1);
        xc_color_t c;
        c.r=(u8)((t.r*(255-tt)+b.r*tt)/255);
        c.g=(u8)((t.g*(255-tt)+b.g*tt)/255);
        c.b=(u8)((t.b*(255-tt)+b.b*tt)/255);
        c.a=255;
        hline(r.x,r.x+r.w,r.y+y,c);
    }
}
void xc_circle(int cx,int cy,int rad,xc_color_t c){
    int r2=rad*rad;
    for(int y=-rad;y<=rad;y++){
        int dy2=y*y;
        if(dy2>r2) continue;
        int dx=isq(r2-dy2);
        hline(cx-dx,cx+dx+1,cy+y,c);
    }
}
