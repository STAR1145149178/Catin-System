#include "catin.h"
#include "xcui.h"
#include "codec.h"  // 现在 codec.h 已经修好了，直接 include 就没问题

static int wifi_on=1,bt_on=0,dnd_on=0,charging=0,battery=78,brightness=70;
static xc_input_t input;

extern const unsigned char wallpaper_png[];
extern const unsigned int  wallpaper_png_len;
static u32 *g_wallpaper_fb = 0;

extern void launch_app(const char *appid, const char *entry_path);

static void draw_image_scaled(image_t *img, int x, int y, int target_w, int target_h) {
    if (!img || !img->pixels) return;
    for (int dy = 0; dy < target_h; dy++) {
        int src_y = dy * img->h / target_h;
        for (int dx = 0; dx < target_w; dx++) {
            int src_x = dx * img->w / target_w;
            u32 px = img->pixels[src_y * img->w + src_x];
            u8 r = (px >> 24) & 0xFF;
            u8 g = (px >> 16) & 0xFF;
            u8 b = (px >> 8) & 0xFF;
            u8 a = px & 0xFF;
            if (a == 0) continue; 
            xc_color_t color = {r, g, b, a};
            xc_pixel(x + dx, y + dy, color);
        }
    }
}

static void draw_app_grid(xc_input_t *in) {
    u32 sz = 0;
    int is_dir = 0;
    if (vfs_stat("/apps/registry.db", &sz, &is_dir) != 0) return;

    char *db = (char*)kmalloc(sz + 1);
    if (!db) return;
    vfs_readfile("/apps/registry.db", db, sz);
    db[sz] = 0;

    int start_x = 60, start_y = 350;
    int icon_size = 120, gap = 40;
    int x = start_x, y = start_y;

    char *p = db;
    while (*p) {
        char *nl = p;
        while (*nl && *nl != '\n') nl++;
        char save = *nl;
        *nl = 0;

        char *fields[8] = {0};
        int fi = 0;
        char *q = p;
        fields[fi++] = q;
        while (*q && fi < 8) {
            if (*q == '|') { *q = 0; fields[fi++] = q + 1; }
            q++;
        }

        if (fi >= 7) {
            const char *appid = fields[0];
            const char *app_name = fields[1];
            const char *logo_path = fields[2];
            const char *entry_path = fields[6];

            xc_rect_t r = {x, y, icon_size, icon_size};
            xc_round(r, 24, XC_RGB(44, 52, 72));

            u32 png_sz = 0;
            if (vfs_stat(logo_path, &png_sz, &is_dir) == 0) {
                char *png_data = (char*)kmalloc(png_sz);
                if (png_data) {
                    vfs_readfile(logo_path, png_data, png_sz);
                    image_t img;
                    if (png_decode(png_data, png_sz, &img) == 0) {
                        int pad = 20;
                        draw_image_scaled(&img, x + pad, y + pad, icon_size - pad*2, icon_size - pad*2);
                        image_free(&img);
                    }
                    kfree(png_data);
                }
            } else {
                xc_text_center(r, "?", xc_theme.text, 4);
            }

            int tw = xc_text_w(app_name, 2);
            xc_text(x + (icon_size - tw)/2, y + icon_size + 12, app_name, xc_theme.text, 2);

            if (in->clicked && xc_rect_contains(r, in->click_x, in->click_y)) {
                in->consumed = 1; 
                xc_toast(XC_TOAST_INFO, "启动 %s...", app_name);
                launch_app(appid, entry_path);
            }

            x += icon_size + gap;
            if (x > fb_w() - icon_size - 20) {
                x = start_x;
                y += icon_size + 60;
            }
        }
        *nl = save;
        p = (save) ? nl + 1 : nl;
    }
    kfree(db);
}

static void get_time(int*hh,int*mm){
    u64 secs=arch_timer_ticks()/1000;
    *hh=(9+(int)(secs/3600))%24;
    *mm=(int)((secs/60)%60);
}

static void load_wallpaper(void){
    if (wallpaper_png_len == 0) return;
    image_t wp;
    if (png_decode(wallpaper_png, wallpaper_png_len, &wp) != 0) return;
    int W = fb_w(), H = fb_h();
    int src_w = wp.w, src_h = wp.h;
    g_wallpaper_fb = (u32*)kmalloc((usize)W * H * 4);
    if (!g_wallpaper_fb){ image_free(&wp); return; }
    u32 s_num, s_den;
    if ((u64)W * src_h > (u64)H * src_w){ s_num = W; s_den = src_w; }
    else                                 { s_num = H; s_den = src_h; }
    u32 sw = (u32)((u64)src_w * s_num / s_den);
    u32 sh = (u32)((u64)src_h * s_num / s_den);
    int ox = (W - (int)sw) / 2;
    int oy = (H - (int)sh) / 2;
    for (int y = 0; y < H; y++){
        u32 *dst = g_wallpaper_fb + (usize)y * W;
        int sy = y - oy;
        if (sy < 0 || sy >= (int)sh){ for (int x = 0; x < W; x++) dst[x] = 0; continue; }
        int src_y = (int)((u64)sy * s_den / s_num);
        if (src_y < 0) src_y = 0; if (src_y >= src_h) src_y = src_h - 1;
        for (int x = 0; x < W; x++){
            int sx = x - ox;
            if (sx < 0 || sx >= (int)sw){ dst[x] = 0; continue; }
            int src_x = (int)((u64)sx * s_den / s_num);
            if (src_x < 0) src_x = 0; if (src_x >= src_w) src_x = src_w - 1;
            u32 px = wp.pixels[(usize)src_y * src_w + src_x];
            u8 r = (px >> 24) & 0xFF; u8 g = (px >> 16) & 0xFF; u8 b = (px >> 8) & 0xFF;
            dst[x] = ((u32)r << 16) | ((u32)g << 8) | b;
        }
    }
    image_free(&wp);
}

void demo_init(void){
    xc_toast(XC_TOAST_OK,"Catin " CATIN_VERSION);
    load_wallpaper();
}

void demo_frame(void){
    xc_input_begin(&input);
    touch_ev_t ev;
    while(touch_poll(&ev)) xc_input_feed(&input,ev);

    if (g_wallpaper_fb){
        int W = fb_w(), H = fb_h();
        u32 *fb = fb_backbuffer();
        kmemcpy(fb, g_wallpaper_fb, (usize)W * H * 4);
    } else {
        xc_clear(xc_theme.bg);
    }

    xc_status_t st;
    get_time(&st.hour,&st.minute);
    st.signal_level=4; st.wifi_on=wifi_on; st.wifi_strength=wifi_on?3:0;
    st.battery=battery; st.charging=charging; st.carrier="CATIN 5G";
    xc_status_bar(&st);

    xc_text(48,100,CATIN_NAME,xc_theme.text,6);
    xc_text(48,230,"XCUI " CATIN_VERSION,xc_theme.text_dim,2);

    draw_app_grid(&input);

    static const char*nl[]={"主页","应用","通知","设置"};
    static const char*ni[]={XC_ICON_HOME,XC_ICON_APPS,XC_ICON_BELL,XC_ICON_SET};
    xc_nav_bar(0,nl,ni,4);

    if(input.holding||input.x||input.y){
        xc_color_t cur=input.holding?xc_theme.primary:(xc_color_t){200,220,255,180};
        xc_circle(input.x,input.y,12,cur);
        xc_circle(input.x,input.y,4,XC_RGB(255,255,255));
    }
}