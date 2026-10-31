#include "catin.h"

static u32 be32(const u8*p){ return ((u32)p[0]<<24)|((u32)p[1]<<16)|((u32)p[2]<<8)|p[3]; }
static u64 be64(const u8*p){ return ((u64)be32(p)<<32)|be32(p+4); }

u64 g_fb_base = 0;
u32 g_fb_w = 0, g_fb_h = 0, g_fb_stride = 0;

// 补充这两个全局变量，给 virtio 用
u32 g_virtio_irq = 31;
u64 g_virtio_base = 0x0a000000ULL;

static int nm(const char *n, const char *lit){
    usize k = kstrlen(lit);
    if (kstrncmp(n, lit, k)) return 0;
    return n[k]==0 || n[k]=='@';
}

void fdt_parse(u64 dtb){
    if (!dtb) return;
    const u8 *f = (const u8*)(uintptr_t)dtb;
    if (be32(f) != 0xD00DFEEDU){ log_err("fdt: bad magic"); return; }
    
    u32 os=be32(f+8), ost=be32(f+12), sz=be32(f+36);
    const u8 *p = f+os, *e = p+sz;
    int ifb=0, iu=0, iv=0;
    
    while (p<e){
        u32 t = be32(p); p += 4;
        if (t == 1){
            const char *n = (const char*)p;
            usize l = kstrlen(n); p += (l+4)&~3UL;
            ifb = nm(n,"framebuffer") || nm(n,"simple-framebuffer");
            iu = nm(n,"pl011") || nm(n,"serial") || nm(n,"uart") || nm(n,"uartdm");
            iv = nm(n,"virtio_mmio") || nm(n,"virtio");
        } else if (t == 2){ 
            ifb=iu=iv=0; 
        } else if (t == 3){
            u32 l = be32(p); p += 4;
            u32 no = be32(p); p += 4;
            const char *pn = (const char*)(f+ost+no);
            
            if (!kstrcmp(pn,"reg")){
                if (ifb && l>=16) g_fb_base = be64(p);
                if (iv && l>=8) g_virtio_base = be64(p);
            }
            if (!kstrcmp(pn,"width") && ifb) g_fb_w = be32(p);
            if (!kstrcmp(pn,"height") && ifb) g_fb_h = be32(p);
            if (!kstrcmp(pn,"stride") && ifb) g_fb_stride = be32(p);
            if (!kstrcmp(pn,"interrupts")){
                if (iv && l>=8) g_virtio_irq = be32(p+4) + 32;
                else if (iv && l>=4){ u32 v = be32(p); if (v>0 && v<1024) g_virtio_irq = v; }
            }
            
            p += (l+3)&~3UL;
        } else break;
    }
    log_info("fdt: fb_base=0x%lx w=%u h=%u", g_fb_base, g_fb_w, g_fb_h);
}