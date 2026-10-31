#include "catin.h"
#include "codec.h"
static const u32 mp3_rates[3] = { 44100, 48000, 32000 };
static const u32 mp3_br_v1[16] = { 0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0 };
static const u32 mp3_br_v2[16] = { 0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0 };

typedef struct { const u8*buf; usize size; u32 bit_pos; } bs_t;
static u32 bs_read(bs_t*b, int n){
    if(n<=0) return 0;
    u32 v=0;
    for(int i=0;i<n;i++){
        u32 by=b->bit_pos>>3, bit=7-(b->bit_pos&7);
        if(by>=b->size) return v<<(n-i);
        v=(v<<1)|((b->buf[by]>>bit)&1);
        b->bit_pos++;
    }
    return v;
}
static void bs_align(bs_t*b){ b->bit_pos=(b->bit_pos+7)&~7U; }

static int mp3_skip_id3(const u8*buf, usize len, u32*pos){
    if(len<10) return 0;
    if(buf[0]=='I'&&buf[1]=='D'&&buf[2]=='3'){
        u32 sz=((u32)(buf[6]&0x7F)<<21)|((u32)(buf[7]&0x7F)<<14)|
               ((u32)(buf[8]&0x7F)<<7)|(u32)(buf[9]&0x7F);
        *pos=10+sz; return 1;
    }
    return 0;
}
static int mp3_find_sync(const u8*buf, usize len, u32*pos){
    for(u32 i=*pos;i+4<=len;i++){
        if(buf[i]==0xFF&&(buf[i+1]&0xE0)==0xE0){
            int ver=(buf[i+1]>>3)&3;
            int lay=(buf[i+1]>>1)&3;
            if(ver==1||lay==0) continue;
            int br=(buf[i+2]>>4)&0xF;
            int sr=(buf[i+2]>>2)&0x3;
            if(br==0||br==15||sr==3) continue;
            *pos=i; return 0;
        }
    }
    return -1;
}
static u32 mp3_fsize(u32 hdr){
    int ver=(hdr>>19)&3, lay=(hdr>>17)&3;
    int br=(hdr>>12)&0xF, sr=(hdr>>10)&3, pad=(hdr>>9)&1;
    if(lay!=1||br==0||br==15||sr==3) return 0;
    u32 bitrate=(ver==3)?mp3_br_v1[br]:mp3_br_v2[br];
    u32 rate=mp3_rates[sr]>>(ver==3?0:(ver==2?1:2));
    return (144*bitrate*1000/rate)+pad;
}

static float cos_tab[36][18];
static int cos_init=0;
static float my_cos(float x){
    while(x>3.14159265f) x-=6.28318530f;
    while(x<-3.14159265f) x+=6.28318530f;
    float x2=x*x;
    return 1-x2/2+x2*x2/24-x2*x2*x2/720+x2*x2*x2*x2/40320;
}
static void cos_init_once(void){
    if(cos_init) return;
    for(int n=0;n<36;n++) for(int k=0;k<18;k++)
        cos_tab[n][k]=my_cos(3.14159265f/72.0f*(2*n+19)*(2*k+1));
    cos_init=1;
}
static void imdct_18(const float*X, float*out){
    cos_init_once();
    for(int n=0;n<36;n++){
        float s=0;
        for(int k=0;k<18;k++) s+=X[k]*cos_tab[n][k];
        out[n]=s;
    }
}

struct mp3_ctx {
    const u8 *buf; usize size; u32 pos;
    int sr_idx; u32 sample_rate; u32 bitrate;
    int channels; int version;
    float prev[32][18];
};

mp3_ctx_t *mp3_open(const void *buf, usize len){
    if(!buf||len<4) return NULL;
    mp3_ctx_t*c=(mp3_ctx_t*)codec_alloc(sizeof(mp3_ctx_t));
    if(!c) return NULL;
    kmemset(c,0,sizeof(*c));
    c->buf=(const u8*)buf; c->size=len; c->pos=0;
    mp3_skip_id3(c->buf,c->size,&c->pos);
    if(mp3_find_sync(c->buf,c->size,&c->pos)!=0){
        codec_free(c); return NULL;
    }
    const u8*p=c->buf+c->pos;
    u32 hdr=((u32)p[0]<<24)|((u32)p[1]<<16)|((u32)p[2]<<8)|p[3];
    int ver=(hdr>>19)&3, br=(hdr>>12)&0xF, sr=(hdr>>10)&3, md=(hdr>>6)&3;
    c->version=(ver==3)?1:(ver==2?2:25);
    c->sr_idx=sr;
    c->sample_rate=mp3_rates[sr]>>(ver==3?0:(ver==2?1:2));
    c->bitrate=(ver==3)?mp3_br_v1[br]:mp3_br_v2[br];
    c->channels=(md==3)?1:2;
    log_info("mp3: %u Hz %u kbps %u ch",c->sample_rate,c->bitrate,c->channels);
    return c;
}

int mp3_decode_frame(mp3_ctx_t*c, audio_t*out){
    if(!c||!out) return -1;
    if(c->pos+4>c->size) return 0;
    const u8*p=c->buf+c->pos;
    u32 hdr=((u32)p[0]<<24)|((u32)p[1]<<16)|((u32)p[2]<<8)|p[3];
    u32 fsize=mp3_fsize(hdr);
    if(fsize==0||c->pos+fsize>c->size){
        u32 next=c->pos+1;
        if(mp3_find_sync(c->buf,c->size,&next)!=0) return 0;
        c->pos=next; return -1;
    }
    int granule_samples=(c->version==1)?1152:576;
    u32 total=granule_samples*c->channels;
    s16*pcm=(s16*)codec_alloc(total*2);
    if(!pcm) return -1;
    kmemset(pcm,0,total*2);
    float X[18], imdct_out[36], pcm_buf[32][18];
    for(int g=0; g<(c->version==1?2:1); g++){
        for(int ch=0; ch<c->channels; ch++){
            for(int k=0;k<18;k++) X[k]=0;
            imdct_18(X,imdct_out);
            for(int i=0;i<18;i++){
                float v=pcm_buf[ch][i]+imdct_out[i];
                c->prev[ch][i]=imdct_out[i+18];
                if(v>32767.0f)v=32767.0f;
                if(v<-32768.0f)v=-32768.0f;
                pcm[g*granule_samples*c->channels+i*c->channels+ch]=(s16)v;
            }
        }
    }
    out->sample_rate=c->sample_rate;
    out->channels=c->channels;
    out->bits=16;
    out->n_samples=granule_samples;
    out->data=pcm;
    c->pos+=fsize;
    return 1;
}
void mp3_close(mp3_ctx_t*c){ if(c) codec_free(c); }

int mp3_decode_all(const void *buf, usize len, audio_t *out){
    mp3_ctx_t*c=mp3_open(buf,len);
    if(!c) return -1;
    u32 cap=0; audio_t tmp;
    u32 save=c->pos;
    while(1){
        int r=mp3_decode_frame(c,&tmp);
        if(r<=0) break;
        cap+=tmp.n_samples*tmp.channels;
        codec_free(tmp.data);
    }
    c->pos=save;
    out->sample_rate=c->sample_rate;
    out->channels=c->channels;
    out->bits=16;
    out->data=(s16*)codec_alloc(cap*2);
    out->n_samples=0;
    if(!out->data){ mp3_close(c); return -1; }
    u32 off=0;
    while(1){
        int r=mp3_decode_frame(c,&tmp);
        if(r<=0) break;
        u32 n=tmp.n_samples*tmp.channels;
        if(off+n>cap) n=cap-off;
        kmemcpy(out->data+off,tmp.data,n*2);
        off+=n;
        codec_free(tmp.data);
    }
    out->n_samples=off/c->channels;
    mp3_close(c);
    log_info("mp3: decoded %u samples",off);
    return 0;
}
