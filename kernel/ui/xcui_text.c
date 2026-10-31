#include "catin.h"
#include "xcui.h"
extern const u8 font8x16[128][16];
static void glyph(int x,int y,u8 ch,xc_color_t c,int sc){
    if(ch>0x8F) ch='?';
    const u8*rows=font8x16[ch];
    for(int r=0;r<16;r++){ u8 bits=rows[r];
        for(int k=0;k<8;k++) if(bits&(0x80>>k))
            for(int sy=0;sy<sc;sy++) for(int sx=0;sx<sc;sx++) xc_pixel(x+k*sc+sx,y+r*sc+sy,c);
    }
}
void xc_text(int x,int y,const char*s,xc_color_t c,int sc){
    if(sc<1)sc=1; int cx=x;
    while(*s){ u8 ch=(u8)*s;
        if(ch=='\n'){ cx=x; y+=16*sc+4; s++; continue; }
        if(ch>=32){ glyph(cx,y,ch,c,sc); cx+=8*sc+2; }
        s++;
    }
}
int xc_text_w(const char*s,int sc){ if(sc<1)sc=1; int w=0; while(*s&&*s!='\n'){w+=8*sc+2;s++;} return w; }
int xc_text_h(int sc){ return 16*(sc<1?1:sc); }
void xc_text_center(xc_rect_t r,const char*s,xc_color_t c,int sc){
    int w=xc_text_w(s,sc),h=xc_text_h(sc);
    xc_text(r.x+(r.w-w)/2,r.y+(r.h-h)/2,s,c,sc);
}
