#include "catin.h"
#include "codec.h"
extern u32 rd_be32(const u8*);

typedef struct { const u8*buf; usize size; usize pos; u32 bb; int bc; } dbs_t;
static u32 dr(dbs_t*b,int n){
    while(b->bc<n){
        if(b->pos>=b->size) return 0;
        b->bb|=(u32)b->buf[b->pos++]<<b->bc;
        b->bc+=8;
    }
    u32 v=b->bb&((1U<<n)-1);
    b->bb>>=n; b->bc-=n;
    return v;
}
#define HB 15
typedef struct { u16 count[HB+1]; u16 symbol[288]; } huff_t;
static int hb(huff_t*h, const u8*lens, int n){
    int i;
    for(i=0;i<=HB;i++) h->count[i]=0;
    for(i=0;i<n;i++) h->count[lens[i]]++;
    if(h->count[0]==n) return 0;
    int left=1;
    for(i=1;i<=HB;i++){ left<<=1; left-=h->count[i]; if(left<0) return -1; }
    u16 offs[HB+2]; offs[1]=0;
    for(i=1;i<=HB;i++) offs[i+1]=offs[i]+h->count[i];
    for(i=0;i<n;i++) if(lens[i]) h->symbol[offs[lens[i]]++]=(u16)i;
    return 0;
}
static int hdec(dbs_t*b, const huff_t*h){
    int code=0, first=0, index=0;
    for(int len=1;len<=HB;len++){
        code|=(int)dr(b,1);
        int count=h->count[len];
        if(code-count<first) return h->symbol[index+(code-first)];
        index+=count; first+=count; first<<=1; code<<=1;
    }
    return -1;
}
static const u16 l_b[29]={3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258};
static const u8  l_e[29]={0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};
static const u16 d_b[30]={1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577};
static const u8  d_e[30]={0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

static int inflate(const u8*in, usize in_len, u8*out, usize out_cap, usize*out_len){
    dbs_t b={in,in_len,0,0,0};
    usize opos=0;
    while(1){
        int bfinal=(int)dr(&b,1);
        int btype=(int)dr(&b,2);
        if(btype==0){
            b.bb=0; b.bc=0;
            if(b.pos+4>b.size) return -1;
            u16 len=b.buf[b.pos]|(b.buf[b.pos+1]<<8);
            u16 nlen=b.buf[b.pos+2]|(b.buf[b.pos+3]<<8);
            b.pos+=4;
            if((len^0xFFFF)!=nlen) return -1;
            if(b.pos+len>b.size) return -1;
            if(opos+len>out_cap) return -1;
            kmemcpy(out+opos,b.buf+b.pos,len);
            b.pos+=len; opos+=len;
        } else if(btype==1||btype==2){
            huff_t lit,dst;
            if(btype==1){
                u8 lens[288];
                for(int i=0;i<144;i++)lens[i]=8;
                for(int i=144;i<256;i++)lens[i]=9;
                for(int i=256;i<280;i++)lens[i]=7;
                for(int i=280;i<288;i++)lens[i]=8;
                if(hb(&lit,lens,288)!=0) return -1;
                for(int i=0;i<30;i++) lens[i]=5;
                if(hb(&dst,lens,30)!=0) return -1;
            } else {
                int hlit=(int)dr(&b,5)+257;
                int hdist=(int)dr(&b,5)+1;
                int hclen=(int)dr(&b,4)+4;
                static const u8 order[19]={16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
                u8 cl_lens[19]={0};
                for(int i=0;i<hclen;i++) cl_lens[order[i]]=(u8)dr(&b,3);
                huff_t cl;
                if(hb(&cl,cl_lens,19)!=0) return -1;
                u8 lens[320]={0};
                int total=hlit+hdist, i=0;
                while(i<total){
                    int sym=hdec(&b,&cl);
                    if(sym<0) return -1;
                    if(sym<16) lens[i++]=(u8)sym;
                    else if(sym==16){
                        if(i==0) return -1;
                        u8 prev=lens[i-1];
                        int r=3+(int)dr(&b,2);
                        while(r--&&i<total) lens[i++]=prev;
                    } else if(sym==17){
                        int r=3+(int)dr(&b,3);
                        while(r--&&i<total) lens[i++]=0;
                    } else {
                        int r=11+(int)dr(&b,7);
                        while(r--&&i<total) lens[i++]=0;
                    }
                }
                if(hb(&lit,lens,hlit)!=0) return -1;
                if(hb(&dst,lens+hlit,hdist)!=0) return -1;
            }
            while(1){
                int sym=hdec(&b,&lit);
                if(sym<0) return -1;
                if(sym<256){ if(opos>=out_cap) return -1; out[opos++]=(u8)sym; }
                else if(sym==256) break;
                else {
                    sym-=257;
                    if(sym>=29) return -1;
                    int length=l_b[sym]+(int)dr(&b,l_e[sym]);
                    int dsym=hdec(&b,&dst);
                    if(dsym<0||dsym>=30) return -1;
                    int dist=d_b[dsym]+(int)dr(&b,d_e[dsym]);
                    if((usize)dist>opos) return -1;
                    if(opos+length>out_cap) return -1;
                    for(int i=0;i<length;i++){ out[opos]=out[opos-dist]; opos++; }
                }
            }
        } else return -1;
        if(bfinal) break;
    }
    *out_len=opos;
    return 0;
}
static void unfilter(u8*cur, const u8*prev, int stride, int bpp, int filter){
    int i;
    switch(filter){
    case 0: break;
    case 1: for(i=bpp;i<stride;i++) cur[i]+=cur[i-bpp]; break;
    case 2: if(prev) for(i=0;i<stride;i++) cur[i]+=prev[i]; break;
    case 3: for(i=0;i<stride;i++){ int a=(i>=bpp)?cur[i-bpp]:0; int b=prev?prev[i]:0; cur[i]+=(u8)((a+b)>>1); } break;
    case 4: for(i=0;i<stride;i++){
        int a=(i>=bpp)?cur[i-bpp]:0;
        int b=prev?prev[i]:0;
        int c=(prev&&i>=bpp)?prev[i-bpp]:0;
        int p=a+b-c;
        int pa=p>a?p-a:a-p, pb=p>b?p-b:b-p, pc=p>c?p-c:c-p;
        int pred=(pa<=pb&&pa<=pc)?a:(pb<=pc?b:c);
        cur[i]+=(u8)pred;
    } break;
    }
}
int png_decode(const void *buf, usize len, image_t *out){
    if(!buf||len<8) return -1;
    static const u8 sig[8]={0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A};
    if(kmemcmp(buf,sig,8)!=0) return -1;
    const u8*p=(const u8*)buf;
    usize pos=8;
    int W=0,H=0,bd=0,ct=0,interlace=0;
    u8 pal[256][3]; int np=0;
    u8 trns[256]; int nt=0;
    u8*idat=NULL; usize idat_cap=0,idat_len=0;
    while(pos+12<=len){
        u32 clen=rd_be32(p+pos);
        u32 ctype=rd_be32(p+pos+4);
        const u8*cd=p+pos+8;
        if(pos+12+clen>len) return -1;
        if(ctype==0x49484452){
            W=rd_be32(cd+0); H=rd_be32(cd+4);
            bd=cd[8]; ct=cd[9]; interlace=cd[12];
            if(interlace!=0) return -1;
        } else if(ctype==0x504C5445){
            np=clen/3; if(np>256) np=256;
            for(int i=0;i<np;i++){ pal[i][0]=cd[i*3]; pal[i][1]=cd[i*3+1]; pal[i][2]=cd[i*3+2]; }
        } else if(ctype==0x74524E53){
            nt=clen>256?256:(int)clen;
            kmemcpy(trns,cd,nt);
        } else if(ctype==0x49444154){
            if(idat_len+clen>idat_cap){
                usize nc=(idat_cap?idat_cap*2:65536);
                while(nc<idat_len+clen) nc*=2;
                u8*ni=(u8*)codec_alloc(nc);
                if(!ni) return -1;
                if(idat){ kmemcpy(ni,idat,idat_len); codec_free(idat); }
                idat=ni; idat_cap=nc;
            }
            kmemcpy(idat+idat_len,cd,clen);
            idat_len+=clen;
        } else if(ctype==0x49454E44) break;
        pos+=12+clen;
    }
    if(!W||!H||!idat){ if(idat) codec_free(idat); return -1; }
    if(bd!=8){ codec_free(idat); return -1; }
    int ch=(ct==0)?1:(ct==2)?3:(ct==3)?1:(ct==4)?2:4;
    int bpp=ch, stride=W*bpp;
    usize raw_cap=(usize)(stride+1)*H+1024;
    u8*raw=(u8*)codec_alloc(raw_cap);
    if(!raw){ codec_free(idat); return -1; }
    usize raw_len=0;
    if(inflate(idat,idat_len,raw,raw_cap,&raw_len)!=0){
        codec_free(idat); codec_free(raw); return -1;
    }
    codec_free(idat);
    u8*unf=(u8*)codec_alloc((usize)stride*H);
    if(!unf){ codec_free(raw); return -1; }
    const u8*rp=raw; usize rleft=raw_len;
    for(int y=0;y<H;y++){
        if(rleft<1) break;
        int f=*rp++; rleft--;
        if(rleft<(usize)stride) break;
        kmemcpy(unf+(usize)y*stride,rp,stride);
        rp+=stride; rleft-=stride;
        const u8*prev=(y>0)?unf+(usize)(y-1)*stride:NULL;
        unfilter(unf+(usize)y*stride,prev,stride,bpp,f);
    }
    codec_free(raw);
    out->w=W; out->h=H; out->stride=W;
    out->pixels=(u32*)codec_alloc((usize)W*H*4);
    if(!out->pixels){ codec_free(unf); return -1; }
    for(int y=0;y<H;y++){
        const u8*row=unf+(usize)y*stride;
        u32*dst=out->pixels+(usize)y*W;
        for(int x=0;x<W;x++){
            u8 R,G,B,A=255;
            if(ct==0){ R=G=B=row[x]; }
            else if(ct==2){ R=row[x*3]; G=row[x*3+1]; B=row[x*3+2]; }
            else if(ct==3){ int i=row[x]; R=pal[i][0]; G=pal[i][1]; B=pal[i][2]; if(i<nt) A=trns[i]; }
            else if(ct==4){ R=G=B=row[x*2]; A=row[x*2+1]; }
            else { R=row[x*4]; G=row[x*4+1]; B=row[x*4+2]; A=row[x*4+3]; }
            dst[x]=((u32)R<<24)|((u32)G<<16)|((u32)B<<8)|A;
        }
    }
    codec_free(unf);
    log_info("png: %dx%d ct=%d",W,H,ct);
    return 0;
}
