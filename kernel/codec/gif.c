#include "catin.h"
#include "codec.h"
extern u16 rd_le16(const u8*);
struct gif_ctx {
    const u8*buf; usize size;
    int W,H,n_frames;
    u32*frames; int*delays;
    int cur_frame;
    usize global_pal_off;
};
typedef struct { const u8*buf; usize size; usize pos; u8 blk[256]; int blen,bpos; } gbs_t;
static int grb(gbs_t*g){
    if(g->bpos<g->blen) return g->blk[g->bpos++];
    if(g->pos>=g->size) return -1;
    int sz=g->buf[g->pos++];
    if(sz==0) return -1;
    if(g->pos+sz>g->size) return -1;
    kmemcpy(g->blk,g->buf+g->pos,sz);
    g->pos+=sz;
    g->blen=sz; g->bpos=0;
    return g->blk[g->bpos++];
}
static int gif_lzw(gbs_t*g, u8*out, int out_size, int mcs){
    int clear_code=1<<mcs;
    int end_code=clear_code+1;
    int code_size=mcs+1;
    int next_code=end_code+1;
    static u16 prefix[4096];
    static u8 suffix[4096], first[4096];
    for(int i=0;i<clear_code;i++){ prefix[i]=0xFFFF; suffix[i]=(u8)i; first[i]=(u8)i; }
    prefix[clear_code]=0xFFFF; prefix[end_code]=0xFFFF;
    int old_code=-1, first_char=0, out_pos=0;
    int bit_buf=0, bit_cnt=0;
    while(out_pos<out_size){
        while(bit_cnt<code_size){
            int b=grb(g);
            if(b<0) return out_pos;
            bit_buf|=b<<bit_cnt; bit_cnt+=8;
        }
        int code=bit_buf&((1<<code_size)-1);
        bit_buf>>=code_size; bit_cnt-=code_size;
        if(code==clear_code){
            code_size=mcs+1;
            next_code=end_code+1;
            old_code=-1;
            continue;
        }
        if(code==end_code) break;
        int in_code=code;
        u8 stack[4096]; int sp=0;
        if(code>=next_code){ stack[sp++]=(u8)first_char; code=old_code; }
        while(code>=clear_code){
            stack[sp++]=suffix[code];
            code=prefix[code];
        }
        first_char=suffix[code];
        stack[sp++]=(u8)first_char;
        while(sp>0&&out_pos<out_size) out[out_pos++]=stack[--sp];
        if(old_code!=-1&&next_code<4096){
            prefix[next_code]=(u16)old_code;
            suffix[next_code]=(u8)first_char;
            first[next_code]=first[old_code];
            next_code++;
            if(next_code>=(1<<code_size)&&code_size<12) code_size++;
        }
        old_code=in_code;
    }
    return out_pos;
}
static int gif_parse_frame(gif_ctx_t*c, usize*ppos, int frame_idx){
    const u8*p=c->buf;
    usize pos=*ppos;
    if(pos+10>c->size) return -1;
    int transparent_idx=-1, delay_ms=0;
    for(;;){
        if(pos>=c->size) return -1;
        u8 blk=p[pos++];
        if(blk==0x21){
            if(pos>=c->size) return -1;
            u8 label=p[pos++];
            if(label==0xF9){
                if(pos+6>c->size) return -1;
                u8 bsz=p[pos++];
                if(bsz!=4) return -1;
                u8 flags=p[pos++];
                delay_ms=((int)p[pos]|((int)p[pos+1]<<8))*10;
                pos+=2;
                transparent_idx=p[pos++];
                if(!(flags&1)) transparent_idx=-1;
                pos++;
            } else {
                while(pos<c->size){ u8 sz=p[pos++]; if(sz==0) break; pos+=sz; }
            }
        } else if(blk==0x2C) break;
        else if(blk==0x3B) return -1;
        else return -1;
    }
    if(pos+9>c->size) return -1;
    int ix=((int)p[pos]|((int)p[pos+1]<<8)); pos+=2;
    int iy=((int)p[pos]|((int)p[pos+1]<<8)); pos+=2;
    int iw=((int)p[pos]|((int)p[pos+1]<<8)); pos+=2;
    int ih=((int)p[pos]|((int)p[pos+1]<<8)); pos+=2;
    u8 flags=p[pos++];
    int has_local=(flags&0x80)!=0;
    int local_size=1<<((flags&7)+1);
    u8 local_pal[256][3]={{0}};
    if(has_local){
        if(pos+local_size*3>c->size) return -1;
        for(int i=0;i<local_size;i++){
            local_pal[i][0]=p[pos++];
            local_pal[i][1]=p[pos++];
            local_pal[i][2]=p[pos++];
        }
    }
    if(pos>=c->size) return -1;
    int mcs=p[pos++];
    gbs_t bs={p,c->size,pos,{0},0,0};
    int npx=iw*ih;
    u8*idx=(u8*)codec_alloc(npx);
    if(!idx) return -1;
    int dec=gif_lzw(&bs,idx,npx,mcs);
    while(bs.pos<c->size){
        u8 sz=p[bs.pos];
        if(sz==0){ bs.pos++; break; }
        bs.pos+=1+sz;
    }
    *ppos=bs.pos;
    u8*pal=has_local?(u8*)local_pal:(u8*)(p+c->global_pal_off);
    u32*dst=c->frames+(usize)frame_idx*c->W*c->H;
    if(frame_idx>0) kmemcpy(dst,c->frames+(usize)(frame_idx-1)*c->W*c->H,c->W*c->H*4);
    else kmemset(dst,0,c->W*c->H*4);
    for(int y=0;y<ih;y++) for(int x=0;x<iw;x++){
        int src=y*iw+x;
        if(src>=dec) break;
        int id=idx[src];
        if(id==transparent_idx) continue;
        int dx=ix+x, dy=iy+y;
        if(dx<0||dy<0||dx>=c->W||dy>=c->H) continue;
        u32 R=pal[id*3], G=pal[id*3+1], B=pal[id*3+2];
        dst[dy*c->W+dx]=(R<<24)|(G<<16)|(B<<8)|0xFF;
    }
    c->delays[frame_idx]=delay_ms;
    codec_free(idx);
    return 0;
}
gif_ctx_t *gif_open(const void *buf, usize len){
    if(!buf||len<13) return NULL;
    const u8*p=(const u8*)buf;
    if(kmemcmp(p,"GIF87a",6)!=0 && kmemcmp(p,"GIF89a",6)!=0) return NULL;
    gif_ctx_t*c=(gif_ctx_t*)codec_alloc(sizeof(gif_ctx_t));
    if(!c) return NULL;
    kmemset(c,0,sizeof(*c));
    c->buf=p; c->size=len;
    c->W=((int)p[6]|((int)p[7]<<8));
    c->H=((int)p[8]|((int)p[9]<<8));
    u8 flags=p[10];
    int has_global=(flags&0x80)!=0;
    int gsize=1<<((flags&7)+1);
    usize pos=13;
    if(has_global){ c->global_pal_off=pos; pos+=gsize*3; }
    usize scan=pos; int nf=0;
    while(scan<len){
        u8 blk=p[scan++];
        if(blk==0x3B) break;
        if(blk==0x21){
            u8 lab=p[scan++];
            if(lab==0xF9){ u8 sz=p[scan]; scan+=sz+1; }
            while(scan<len){ u8 sz=p[scan++]; if(sz==0) break; scan+=sz; }
        } else if(blk==0x2C){
            scan+=9;
            u8 fl=p[scan-1];
            if(fl&0x80) scan+=(1<<((fl&7)+1))*3;
            scan++;
            while(scan<len){ u8 sz=p[scan++]; if(sz==0) break; scan+=sz; }
            nf++;
        } else break;
    }
    if(nf==0) nf=1;
    if(nf>128) nf=128;
    c->n_frames=nf;
    c->frames=(u32*)codec_alloc((usize)nf*c->W*c->H*4);
    c->delays=(int*)codec_alloc(nf*sizeof(int));
    if(!c->frames||!c->delays){ codec_free(c); return NULL; }
    kmemset(c->frames,0,(usize)nf*c->W*c->H*4);
    kmemset(c->delays,100,nf*sizeof(int));
    usize dp=pos;
    for(int i=0;i<nf;i++) if(gif_parse_frame(c,&dp,i)!=0) break;
    log_info("gif: %dx%d %d frames",c->W,c->H,c->n_frames);
    c->cur_frame=0;
    return c;
}
int gif_frame_count(gif_ctx_t*c){ return c?c->n_frames:0; }
int gif_next_frame(gif_ctx_t*c, image_t*out, int*delay_ms){
    if(!c||c->cur_frame>=c->n_frames) return -1;
    out->w=c->W; out->h=c->H; out->stride=c->W;
    out->pixels=(u32*)codec_alloc((usize)c->W*c->H*4);
    if(!out->pixels) return -1;
    kmemcpy(out->pixels,c->frames+(usize)c->cur_frame*c->W*c->H,(usize)c->W*c->H*4);
    if(delay_ms) *delay_ms=c->delays[c->cur_frame];
    c->cur_frame++;
    return 0;
}
void gif_close(gif_ctx_t*c){
    if(!c) return;
    if(c->frames) codec_free(c->frames);
    if(c->delays) codec_free(c->delays);
    codec_free(c);
}
int gif_decode_first(const void *buf, usize len, image_t *out){
    gif_ctx_t*c=gif_open(buf,len);
    if(!c) return -1;
    int delay;
    int r=gif_next_frame(c,out,&delay);
    gif_close(c);
    return r;
}
