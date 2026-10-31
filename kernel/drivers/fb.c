#include "catin.h"

// 这些变量在 fdt.c 里定义，开机时去设备树里挖出来的
extern u64 g_fb_base;
extern u32 g_fb_w, g_fb_h, g_fb_stride;

static u32 *g_fb;
static int g_w, g_h;

int fb_init(void) {
    // 如果 fdt_parse 成功找到了真机的屏幕地址，就用真机的
    if (g_fb_base != 0 && g_fb_base >= 0x80000000ULL) {
        g_fb = (u32*)(uintptr_t)g_fb_base;
        g_w = g_fb_w > 0 ? g_fb_w : 1080;
        g_h = g_fb_h > 0 ? g_fb_h : 1920;
        log_info("fb: real hardware framebuffer at 0x%lx (%dx%d)", g_fb_base, g_w, g_h);
    } else {
        // 如果没读到（或者你在 QEMU 里没配），就走兜底逻辑
        // 注意：这里的 0xE3940000 是高通 MSM8939 常见的开机预留内存地址
        // 如果真机上不亮，说明 Bootloader 没给你留这块内存，需要手写 MIPI 驱动
        g_fb_base = 0xE3940000ULL;
        g_w = 1080;
        g_h = 1920;
        g_fb = (u32*)(uintptr_t)g_fb_base;
        log_info("fb: fallback to default MSM8939 physical address 0x%lx", g_fb_base);
    }
    return 0;
}

int fb_w(void) { return g_w; }
int fb_h(void) { return g_h; }
u32 *fb_backbuffer(void) { return g_fb; }
void fb_present(void) {}