#ifndef _C_H
#define _C_H
#include "types.h"
#include "arch.h"
#define CATIN_NAME "Catin"
#define CATIN_VERSION "2.2.0"
#define CATIN_CODENAME "Cascade"
#define SCREEN_W 720
#define SCREEN_H 1280

void mm_init(void); void *kmalloc(usize n); void *kzalloc(usize n);
void kfree(void *p); usize mm_used(void); usize mm_free(void);
void uart_init(void); void uart_puts(const char *s);
void log_info(const char *fmt, ...); void log_err(const char *fmt, ...);

int  fb_init(void); int fb_w(void); int fb_h(void);
u32 *fb_backbuffer(void); void fb_present(void);

typedef enum { TOUCH_NONE=0, TOUCH_DOWN, TOUCH_MOVE, TOUCH_UP } touch_kind_t;
typedef struct { touch_kind_t kind; int x, y; } touch_ev_t;
int  touch_init(void); int touch_poll(touch_ev_t *ev);
void touch_feed(touch_kind_t k, int x, int y);
void touch_tick(void); int touch_online(void);

void fdt_parse(u64 dtb);

usize kstrlen(const char *s); void *kmemset(void *d, int c, usize n);
void *kmemcpy(void *d, const void *s, usize n);
int kstrcmp(const char *a, const char *b);
int kstrncmp(const char *a, const char *b, usize n);
int kmemcmp(const void *a, const void *b, usize n);
void kstrncpy(char *d, const char *s, usize n);
int ksnprintf(char *buf, usize cap, const char *fmt, ...);
int katoi(const char *s);

#include "proc.h"
#include "vfs.h"
#include "syscall.h"

void shell_init(void); void shell_frame(void); void shell_tick(void);
void demo_init(void);  void demo_frame(void);
#endif
