#include "catin.h"
#include "codec.h"
extern u16 rd_le16(const u8*); extern u32 rd_le32(const u8*);
int wav_decode(const void *buf, usize len, audio_t *out){
    if (!buf || len < 44 || !out) return -1;
    const u8 *p = (const u8*)buf;
    if (rd_le32(p) != 0x46464952) return -1;
    if (rd_le32(p+8) != 0x45564157) return -1;
    u32 pos = 12;
    u16 format = 0, channels = 0, bits = 0;
    u32 rate = 0, data_off = 0, data_len = 0;
    while (pos + 8 <= len){
        u32 id = rd_le32(p + pos);
        u32 sz = rd_le32(p + pos + 4);
        pos += 8;
        if (id == 0x20746d66){
            if (sz < 16) return -1;
            format   = rd_le16(p + pos);
            channels = rd_le16(p + pos + 2);
            rate     = rd_le32(p + pos + 4);
            bits     = rd_le16(p + pos + 14);
        } else if (id == 0x61746164){
            data_off = pos; data_len = sz;
            if (data_off + data_len > len) data_len = len - data_off;
            break;
        }
        pos += (sz + 1) & ~1U;
    }
    if (!channels || !bits || !data_len) return -1;
    if (format != 1 && format != 3) return -1;
    out->sample_rate = rate; out->channels = channels; out->bits = 16;
    if (bits == 16){
        u32 n = data_len / 2;
        out->n_samples = n / channels;
        out->data = (s16*)codec_alloc(n * 2);
        if (!out->data) return -1;
        for (u32 i = 0; i < n; i++) out->data[i] = (s16)rd_le16(p + data_off + i*2);
    } else if (bits == 8){
        u32 n = data_len;
        out->n_samples = n / channels;
        out->data = (s16*)codec_alloc(n * 2);
        if (!out->data) return -1;
        for (u32 i = 0; i < n; i++) out->data[i] = ((s16)p[data_off + i] - 128) << 8;
    } else if (bits == 24){
        u32 n = data_len / 3;
        out->n_samples = n / channels;
        out->data = (s16*)codec_alloc(n * 2);
        if (!out->data) return -1;
        for (u32 i = 0; i < n; i++){
            const u8 *q = p + data_off + i*3;
            s32 v = (q[0]) | (q[1]<<8) | ((s32)(s8)q[2] << 16);
            out->data[i] = (s16)(v >> 8);
        }
    } else return -1;
    log_info("wav: %u Hz %u ch %u bit, %u samples", rate, channels, bits, out->n_samples);
    return 0;
}
