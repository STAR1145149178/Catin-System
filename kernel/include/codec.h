#ifndef _CODEC_H
#define _CODEC_H
#include "types.h"

typedef enum { MEDIA_UNKNOWN=0, MEDIA_AUDIO, MEDIA_VIDEO, MEDIA_IMAGE } media_kind_t;

typedef struct {
    int w, h;
    u32 *pixels;
    int stride;
} image_t;

typedef struct {
    u32 sample_rate;
    u16 channels;
    u16 bits;
    u32 n_samples;
    s16 *data;
} audio_t;

int  wav_decode(const void *buf, usize len, audio_t *out);
void wav_free(audio_t *a);

typedef struct mp3_ctx mp3_ctx_t;
mp3_ctx_t *mp3_open(const void *buf, usize len);
int  mp3_decode_frame(mp3_ctx_t *c, audio_t *out);
void mp3_close(mp3_ctx_t *c);
int  mp3_decode_all(const void *buf, usize len, audio_t *out);

typedef struct mp4_ctx mp4_ctx_t;
mp4_ctx_t *mp4_open(const void *buf, usize len);
int  mp4_track_count(mp4_ctx_t *c);
int  mp4_get_audio_track(mp4_ctx_t *c, audio_t *out);
void mp4_close(mp4_ctx_t *c);

int  png_decode(const void *buf, usize len, image_t *out);
void image_free(image_t *img);

int  jpg_decode(const void *buf, usize len, image_t *out);

typedef struct gif_ctx gif_ctx_t;
gif_ctx_t *gif_open(const void *buf, usize len);
int  gif_frame_count(gif_ctx_t *c);
int  gif_next_frame(gif_ctx_t *c, image_t *out, int *delay_ms);
void gif_close(gif_ctx_t *c);
int  gif_decode_first(const void *buf, usize len, image_t *out);

void *codec_alloc(usize n);
void  codec_free(void *p);
#endif