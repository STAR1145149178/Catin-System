#include "catin.h"
#include "codec.h"
static const u8 zigzag[64]={
     0, 1, 8,16, 9, 2, 3,10,17,24,32,25,18,11, 4, 5,
    12,19,26,33,40,48,41,34,27,20,13, 6, 7,14,21,28,
    35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,
    58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63,
};
typedef struct { u8 bits[17]; u8 val[256]; int sz[256]; int code[256]; int n; } jh_t;
static void build_h(jh_t*h){
    int code=0,k=0;
    for(int len=1;len<=16;len++){
        for(int i=0;i<h->bits[len];i++){ h->sz[k]=len; h->code[k]=code; code++; k++; }
        code<<=1;
    }
    h->n=k;
}
typedef struct { const u8*buf; usize size; usize pos; u32 bb; int bc; } jbs_t;
static u32 jr(jbs_t*b,int n){
    while(b->bc<n){
        if(b->pos>=b->size) return 0;
        u8 by=b->buf[b->pos++];
        if(by==0xFF&&b->pos<b->size){ u8 nx=b->buf[b->pos]; if(nx==0x00) b->pos++; }
        b->bb=(b->bb<<8)|by; b->bc+=8;
    }
    u32 v=(b->bb>>(b->bc-n))&((1U<<n)-1);
    b->bc-=n; return v;
}
static int jh_dec(jbs_t*b, jh_t*h){
    int code=0;
    for(int len=1;len<=16;len++){
        code=(code<<1)|(int)jr(b,1);
        for(int i=0;i<h->n;i++) if(h->sz[i]==len&&h->code[i]==code) return h->val[i];
    }
    return -1;
}
static int jre(jbs_t*b, int n){
    if(n==0) return 0;
    u32 v=jr(b,n);
    u32 vt=1U<<(n-1);
    if(v<vt) v+=(0xFFFFFFFFU<<n)+1;
    return (int)v;
}
static void idct8x8(const float*in, u8*out, int stride){
    static float C[8][8]; static int inited=0;
    if(!inited){
        for(int u=0;u<8;u++) for(int x=0;x<8;x++){
            float cu=(u==0)?0.70710678f:1.0f;
            float angle=(float)(2*x+1)*u*3.14159265f/16.0f;
            float a2=angle*angle;
            float cs=1-a2/2+a2*a2/24-a2*a2*a2/720+a2*a2*a2*a2/40320-a2*a2*a2*a2*a2/3628800;
            C[u][x]=cu*cs*0.5f;
        }
        inited=1;
    }
    float tmp[64];
    for(int y=0;y<8;y++) for(int x=0;x<8;x++){
        float s=0;
        for(int u=0;u<8;u++) s+=C[u][x]*in[y*8+u];
        tmp[y*8+x]=s;
    }
    for(int x=0;x<8;x++) for(int y=0;y<8;y++){
        float s=0;
        for(int v=0;v<8;v++) s+=C[v][y]*tmp[v*8+x];
        int iv=(int)(s+128.5f);
        if(iv<0) iv=0; if(iv>255) iv=255;
        out[y*stride+x]=(u8)iv;
    }
}
typedef struct { int id,h_samp,v_samp,tq,td,ta; jh_t hdc,hac; int pred; } jc_t;
int jpg_decode(const void *buf, usize len, image_t *out){
    if(!buf||len<4) return -1;
    const u8*p=(const u8*)buf;
    if(p[0]!=0xFF||p[1]!=0xD8) return -1;
    usize pos=2;
    int W=0,H=0; jc_t cs[3]; int ncomp=0;
    u8 qt[4][64]={{0}};
    while(pos+4<=len){
        if(p[pos]!=0xFF){ pos++; continue; }
        u8 marker=p[pos+1]; pos+=2;
        if(marker==0xD9) break;
        if(marker>=0xD0&&marker<=0xD7) continue;
        if(pos+2>len) break;
        u16 seglen=((u16)p[pos]<<8)|p[pos+1];
        if(pos+seglen>len) break;
        const u8*sd=p+pos+2;
        u16 sdlen=seglen-2;
        if(marker==0xDB){
            while(sdlen>0){
                int pq=sd[0]>>4, tq=sd[0]&0xF;
                sd++; sdlen--;
                if(pq==0){ for(int i=0;i<64;i++) qt[tq][i]=sd[i]; sd+=64; sdlen-=64; }
                else { for(int i=0;i<64;i++) qt[tq][i]=sd[i*2]; sd+=128; sdlen-=128; }
            }
        } else if(marker==0xC0||marker==0xC1){
            if(sdlen<6) return -1;
            H=((int)sd[1]<<8)|sd[2];
            W=((int)sd[3]<<8)|sd[4];
            ncomp=sd[5];
            if(ncomp>3) ncomp=3;
            for(int i=0;i<ncomp;i++){
                cs[i].id=sd[6+i*3];
                cs[i].h_samp=sd[6+i*3+1]>>4;
                cs[i].v_samp=sd[6+i*3+1]&0xF;
                cs[i].tq=sd[6+i*3+2];
                cs[i].pred=0;
            }
        } else if(marker==0xC4){
            while(sdlen>0){
                int tc=sd[0]>>4, th=sd[0]&0xF;
                jh_t*h=(tc==0)?&cs[th].hdc:&cs[th].hac;
                kmemset(h,0,sizeof(*h));
                for(int i=1;i<=16;i++) h->bits[i]=sd[i];
                int total=0;
                for(int i=1;i<=16;i++) total+=h->bits[i];
                for(int i=0;i<total;i++) h->val[i]=sd[17+i];
                build_h(h);
                int consumed=17+total;
                sd+=consumed; sdlen-=consumed;
            }
        } else if(marker==0xDA){
            int ns=sd[0];
            for(int i=0;i<ns;i++){
                int cid=sd[1+i*2];
                int td=sd[2+i*2]>>4, ta=sd[2+i*2]&0xF;
                for(int j=0;j<ncomp;j++) if(cs[j].id==cid){ cs[j].td=td; cs[j].ta=ta; break; }
            }
            pos+=seglen;
            break;
        }
        pos+=seglen;
    }
    if(!W||!H||!ncomp) return -1;
    out->w=W; out->h=H; out->stride=W;
    out->pixels=(u32*)codec_alloc((usize)W*H*4);
    if(!out->pixels) return -1;
    jbs_t bs={p+pos,len-pos,0,0,0};
    int mcu_w=W/8+((W%8)?1:0);
    int mcu_h=H/8+((H%8)?1:0);
    u8*plane[3]={NULL,NULL,NULL};
    for(int c=0;c<ncomp;c++){
        plane[c]=(u8*)codec_alloc((usize)mcu_w*8*mcu_h*8);
        kmemset(plane[c],128,(usize)mcu_w*8*mcu_h*8);
    }
    for(int my=0;my<mcu_h;my++){
        for(int mx=0;mx<mcu_w;mx++){
            for(int c=0;c<ncomp;c++){
                float blk[64]; kmemset(blk,0,sizeof(blk));
                int t=jh_dec(&bs,&cs[c].hdc);
                if(t<0) t=0;
                int diff=jre(&bs,t);
                cs[c].pred+=diff;
                int q=cs[c].tq;
                blk[0]=cs[c].pred*(float)qt[q][0];
                for(int k=1;k<64;){
                    int rs=jh_dec(&bs,&cs[c].hac);
                    if(rs<0) break;
                    int s=rs&0xF, r=rs>>4;
                    if(s==0){ if(r==15){ k+=16; continue; } break; }
                    k+=r;
                    if(k>=64) break;
                    int v=jre(&bs,s);
                    int zi=zigzag[k];
                    blk[zi]=(float)(v*qt[q][zi]);
                    k++;
                }
                u8 block[64];
                idct8x8(blk,block,8);
                int px=mx*8,py=my*8;
                for(int y=0;y<8;y++) for(int x=0;x<8;x++){
                    int ox=px+x, oy=py+y;
                    if(ox<mcu_w*8&&oy<mcu_h*8)
                        plane[c][oy*mcu_w*8+ox]=block[y*8+x];
                }
            }
        }
    }
    for(int y=0;y<H;y++) for(int x=0;x<W;x++){
        u32 R,G,B;
        if(ncomp==1){ R=G=B=plane[0][y*mcu_w*8+x]; }
        else {
            int Y=plane[0][y*mcu_w*8+x];
            int Cb=plane[1][y*mcu_w*8+x]-128;
            int Cr=plane[2][y*mcu_w*8+x]-128;
            int r=Y+(int)(1.402f*Cr);
            int g=Y-(int)(0.344136f*Cb)-(int)(0.714136f*Cr);
            int b=Y+(int)(1.772f*Cb);
            if(r<0)r=0; if(r>255)r=255;
            if(g<0)g=0; if(g>255)g=255;
            if(b<0)b=0; if(b>255)b=255;
            R=r; G=g; B=b;
        }
        out->pixels[y*W+x]=(R<<24)|(G<<16)|(B<<8)|0xFF;
    }
    for(int c=0;c<ncomp;c++) if(plane[c]) codec_free(plane[c]);
    log_info("jpg: %dx%d %dcomp",W,H,ncomp);
    return 0;
}
