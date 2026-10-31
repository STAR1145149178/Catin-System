#include "catin.h"
#include "codec.h"

void *codec_alloc(usize n){ return kmalloc(n); }
void  codec_free(void *p){ kfree(p); }

u16 rd_be16(const u8 *p){ return ((u16)p[0]<<8)|p[1]; }
u32 rd_be32(const u8 *p){ return ((u32)p[0]<<24)|((u32)p[1]<<16)|((u32)p[2]<<8)|p[3]; }
u16 rd_le16(const u8 *p){ return ((u16)p[1]<<8)|p[0]; }
u32 rd_le32(const u8 *p){ return ((u32)p[3]<<24)|((u32)p[2]<<16)|((u32)p[1]<<8)|p[0]; }

void image_free(image_t *img){
    if (!img) return;
    if (img->pixels) { codec_free(img->pixels); img->pixels = NULL; }
    img->w = img->h = img->stride = 0;
}

void wav_free(audio_t *a){
    if (!a) return;
    if (a->data) { codec_free(a->data); a->data = NULL; }
    a->n_samples = 0;
}