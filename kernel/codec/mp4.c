#include "catin.h"
#include "codec.h"
extern u32 rd_be32(const u8*);
static u64 rd_be64(const u8 *p){ return ((u64)rd_be32(p)<<32)|rd_be32(p+4); }

struct mp4_ctx {
    const u8 *buf; usize size;
    int has_audio; u32 timescale, duration;
    u16 channels, sample_size;
    u32 sample_rate, codec;
    u32 *sample_sizes; u32 n_samples;
    u32 *chunk_offsets; u32 n_chunks;
    u32 *stsc_first; u32 *stsc_spp; u32 *stsc_idx; u32 n_stsc;
    u32 *stts_count; u32 *stts_delta; u32 n_stts;
};

typedef struct { const u8*start; const u8*end; } box_iter_t;
static int box_next(box_iter_t*it, u32*type, const u8**data, u32*size){
    if(it->start+8>it->end) return 0;
    u32 sz=rd_be32(it->start);
    u32 tp=rd_be32(it->start+4);
    const u8*d=it->start+8; u32 hdr=8;
    if(sz==1){
        if(it->start+16>it->end) return 0;
        u64 big=rd_be64(it->start+8);
        if(big>0xFFFFFFFFULL) return 0;
        sz=(u32)big; d=it->start+16; hdr=16;
    } else if(sz==0){ sz=(u32)(it->end-it->start); }
    if(sz<hdr||it->start+sz>it->end) return 0;
    *type=tp; *data=d; *size=sz-hdr;
    it->start+=sz;
    return 1;
}
static int box_find(const u8*data, u32 size, u32 type, const u8**out, u32*outsz){
    box_iter_t it={data,data+size};
    u32 t; const u8*d; u32 s;
    while(box_next(&it,&t,&d,&s)) if(t==type){ *out=d; *outsz=s; return 0; }
    return -1;
}
static int parse_stsd(const u8*data, u32 size, mp4_ctx_t*c){
    if(size<8) return -1;
    if(size<8+8+8) return -1;
    const u8*e=data+8;
    u32 fmt=rd_be32(e+4);
    if(fmt!=0x6D703461 && fmt!=0x61616320) return -1;
    const u8*a=e+8;
    c->channels=((u16)a[8]<<8)|a[9];
    c->sample_size=((u16)a[10]<<8)|a[11];
    c->sample_rate=((u32)a[16]<<24|(u32)a[17]<<16|(u32)a[18]<<8|a[19])>>16;
    c->codec=fmt; c->has_audio=1; return 0;
}
static int parse_stsz(const u8*data, u32 size, mp4_ctx_t*c){
    if(size<12) return -1;
    u32 ss=rd_be32(data+4), count=rd_be32(data+8);
    c->n_samples=count;
    c->sample_sizes=(u32*)codec_alloc(count*4);
    if(!c->sample_sizes) return -1;
    if(ss!=0){ for(u32 i=0;i<count;i++) c->sample_sizes[i]=ss; }
    else { for(u32 i=0;i<count;i++) c->sample_sizes[i]=rd_be32(data+12+i*4); }
    return 0;
}
static int parse_stco(const u8*data, u32 size, mp4_ctx_t*c){
    if(size<8) return -1;
    u32 count=rd_be32(data+4);
    c->n_chunks=count;
    c->chunk_offsets=(u32*)codec_alloc(count*4);
    if(!c->chunk_offsets) return -1;
    for(u32 i=0;i<count;i++) c->chunk_offsets[i]=rd_be32(data+8+i*4);
    return 0;
}
static int parse_stsc(const u8*data, u32 size, mp4_ctx_t*c){
    if(size<8) return -1;
    u32 count=rd_be32(data+4);
    c->n_stsc=count;
    c->stsc_first=(u32*)codec_alloc(count*4);
    c->stsc_spp=(u32*)codec_alloc(count*4);
    c->stsc_idx=(u32*)codec_alloc(count*4);
    if(!c->stsc_first) return -1;
    for(u32 i=0;i<count;i++){
        c->stsc_first[i]=rd_be32(data+8+i*12);
        c->stsc_spp[i]=rd_be32(data+8+i*12+4);
        c->stsc_idx[i]=rd_be32(data+8+i*12+8);
    }
    return 0;
}
static int parse_stts(const u8*data, u32 size, mp4_ctx_t*c){
    if(size<8) return -1;
    u32 count=rd_be32(data+4);
    c->n_stts=count;
    c->stts_count=(u32*)codec_alloc(count*4);
    c->stts_delta=(u32*)codec_alloc(count*4);
    if(!c->stts_count) return -1;
    for(u32 i=0;i<count;i++){
        c->stts_count[i]=rd_be32(data+8+i*8);
        c->stts_delta[i]=rd_be32(data+8+i*8+4);
    }
    return 0;
}
static int parse_moov(const u8*data, u32 size, mp4_ctx_t*c){
    box_iter_t it={data,data+size};
    u32 t; const u8*d; u32 s;
    while(box_next(&it,&t,&d,&s)){
        if(t==0x7472616B){
            const u8*mdia_d; u32 mdia_s;
            if(box_find(d,s,0x6D646961,&mdia_d,&mdia_s)!=0) continue;
            const u8*hdlr_d; u32 hdlr_s;
            if(box_find(mdia_d,mdia_s,0x68646C72,&hdlr_d,&hdlr_s)!=0) continue;
            if(hdlr_s<12) continue;
            u32 handler=rd_be32(hdlr_d+8);
            if(handler!=0x736F756E) continue;
            const u8*minf_d; u32 minf_s;
            if(box_find(mdia_d,mdia_s,0x6D696E66,&minf_d,&minf_s)!=0) continue;
            const u8*stbl_d; u32 stbl_s;
            if(box_find(minf_d,minf_s,0x7374626C,&stbl_d,&stbl_s)!=0) continue;
            box_iter_t bit={stbl_d,stbl_d+stbl_s};
            const u8*sd; u32 ssz;
            while(box_next(&bit,&t,&sd,&ssz)){
                switch(t){
                case 0x73747364: parse_stsd(sd,ssz,c); break;
                case 0x7374737A: parse_stsz(sd,ssz,c); break;
                case 0x7374636F: parse_stco(sd,ssz,c); break;
                case 0x73747363: parse_stsc(sd,ssz,c); break;
                case 0x73747473: parse_stts(sd,ssz,c); break;
                }
            }
        }
    }
    const u8*mvhd; u32 mvhd_s;
    if(box_find(data,size,0x6D766864,&mvhd,&mvhd_s)==0 && mvhd_s>=20){
        u8 ver=mvhd[0];
        if(ver==0){ c->timescale=rd_be32(mvhd+12); c->duration=rd_be32(mvhd+16); }
        else { c->timescale=rd_be32(mvhd+20); c->duration=(u32)rd_be64(mvhd+24); }
    }
    return 0;
}

mp4_ctx_t *mp4_open(const void *buf, usize len){
    if(!buf||len<16) return NULL;
    mp4_ctx_t*c=(mp4_ctx_t*)codec_alloc(sizeof(mp4_ctx_t));
    if(!c) return NULL;
    kmemset(c,0,sizeof(*c));
    c->buf=(const u8*)buf; c->size=len;
    box_iter_t it={(const u8*)buf,(const u8*)buf+len};
    u32 t; const u8*d; u32 s;
    while(box_next(&it,&t,&d,&s)){
        if(t==0x6D6F6F76){ parse_moov(d,s,c); break; }
    }
    if(!c->has_audio){ codec_free(c); return NULL; }
    log_info("mp4: %u ch %u Hz %u samples",c->channels,c->sample_rate,c->n_samples);
    return c;
}
int mp4_track_count(mp4_ctx_t*c){ return c && c->has_audio ? 1 : 0; }
int mp4_get_audio_track(mp4_ctx_t*c, audio_t*out){
    if(!c||!out||!c->has_audio) return -1;
    u32 n_total=0;
    if(c->n_stts) for(u32 i=0;i<c->n_stts;i++) n_total+=c->stts_count[i];
    if(!n_total) n_total=c->n_samples*1024;
    out->sample_rate=c->sample_rate;
    out->channels=c->channels;
    out->bits=16;
    out->n_samples=n_total;
    out->data=(s16*)codec_alloc(n_total*c->channels*2);
    if(!out->data) return -1;
    kmemset(out->data,0,n_total*c->channels*2);
    log_info("mp4: extracted %u samples (stub)",n_total);
    return 0;
}
void mp4_close(mp4_ctx_t*c){
    if(!c) return;
    if(c->sample_sizes) codec_free(c->sample_sizes);
    if(c->chunk_offsets) codec_free(c->chunk_offsets);
    if(c->stsc_first) codec_free(c->stsc_first);
    if(c->stsc_spp) codec_free(c->stsc_spp);
    if(c->stsc_idx) codec_free(c->stsc_idx);
    if(c->stts_count) codec_free(c->stts_count);
    if(c->stts_delta) codec_free(c->stts_delta);
    codec_free(c);
}
