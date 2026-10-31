#ifndef _X_H
#define _X_H
#include "types.h"
typedef struct { u8 r, g, b, a; } xc_color_t;
#define XC_RGB(r,g,b) ((xc_color_t){(r),(g),(b),255})
#define XC_RGBA(r,g,b,a) ((xc_color_t){(r),(g),(b),(a)})
xc_color_t xc_blend(xc_color_t fg, xc_color_t bg);
typedef struct { int x, y, w, h; } xc_rect_t;
#define XC_RECT(x,y,w,h) ((xc_rect_t){(x),(y),(w),(h)})
int xc_rect_contains(xc_rect_t r, int px, int py);
typedef struct {
    xc_color_t bg, surface, surface_alt, primary, primary_dark;
    xc_color_t text, text_dim, divider, success, danger, warning, shadow;
} xc_theme_t;
extern xc_theme_t xc_theme;
void xc_init(void); void xc_begin(void); void xc_end(void);
void xc_clear(xc_color_t c);
void xc_fill(xc_rect_t r, xc_color_t c);
void xc_round(xc_rect_t r, int radius, xc_color_t c);
void xc_shadow(xc_rect_t r, int radius, int spread, u8 alpha);
void xc_vgradient(xc_rect_t r, xc_color_t top, xc_color_t bot);
void xc_circle(int cx, int cy, int radius, xc_color_t c);
void xc_pixel(int x, int y, xc_color_t c);
void xc_text(int x, int y, const char *s, xc_color_t c, int scale);
void xc_text_center(xc_rect_t r, const char *s, xc_color_t c, int scale);
int  xc_text_w(const char *s, int scale); int xc_text_h(int scale);
void xc_push_clip(xc_rect_t r); void xc_pop_clip(void);
#define XC_ICON_WIFI "\x80"
#define XC_ICON_BT   "\x81"
#define XC_ICON_HOME "\x84"
#define XC_ICON_APPS "\x85"
#define XC_ICON_BELL "\x86"
#define XC_ICON_SET  "\x87"
#define XC_ICON_INFO "\x88"
#define XC_ICON_MOON "\x8A"
typedef struct {
    int x, y, down, up, holding, clicked;
    int click_x, click_y, down_x, down_y, consumed;
} xc_input_t;
void xc_input_begin(xc_input_t *in);
void xc_input_feed(xc_input_t *in, touch_ev_t ev);
void xc_input_end(xc_input_t *in);
typedef enum { XC_BTN_PRIMARY=0, XC_BTN_SECONDARY, XC_BTN_GHOST, XC_BTN_DANGER } xc_btn_style_t;
typedef struct {
    xc_rect_t r; const char *label; xc_btn_style_t style;
    int pressed, enabled, radius;
    void (*on_click)(void *ud); void *user;
} xc_button_t;
void xc_button(xc_button_t *b, xc_input_t *in);
typedef struct {
    xc_rect_t r; const char *title, *subtitle, *icon;
    int on, anim;
    void (*on_change)(void *ud, int on); void *user;
} xc_switch_t;
void xc_switch(xc_switch_t *s, xc_input_t *in);
typedef struct {
    xc_rect_t r; const char *icon;
    int value; int dragging;
    void (*on_change)(void *ud, int v); void *user;
} xc_slider_t;
void xc_slider(xc_slider_t *s, xc_input_t *in);
void xc_card(xc_rect_t r, const char *title, const char *subtitle);
typedef struct {
    int hour, minute, signal_level, wifi_on, wifi_strength, battery, charging;
    const char *carrier;
} xc_status_t;
void xc_status_bar(const xc_status_t *s);
void xc_nav_bar(int active, const char *const labels[], const char *const icons[], int n);
int  xc_list_item(xc_rect_t r, const char *icon, const char *title, const char *value, xc_input_t *in);
#define XC_TOAST_MS 2400
#define XC_TOAST_MAX 4
typedef enum { XC_TOAST_INFO=0, XC_TOAST_OK, XC_TOAST_WARN, XC_TOAST_ERR } xc_toast_kind_t;
void xc_toast(xc_toast_kind_t kind, const char *fmt, ...);
void xc_toast_tick(void); void xc_toast_draw(void);
#endif
