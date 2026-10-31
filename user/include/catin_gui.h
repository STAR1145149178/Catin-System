#ifndef _CATIN_GUI_H
#define _CATIN_GUI_H
#include "catin_types.h"

#define C_BLACK 0xFF000000u
#define C_WHITE 0xFFFFFFFFu
#define C_RED   0xFFFF0000u
#define C_GREEN 0xFF00FF00u
#define C_BLUE  0xFF0000FFu
#define C_GRAY  0xFF808080u
#define C_DARK  0xFF202020u
#define C_LIGHT 0xFFE0E0E0u
#define C_BG    0xFF101018u

#define RGB(r,g,b) (0xFF000000u|((u32)(r)<<16)|((u32)(g)<<8)|(u32)(b))
#define RGBA(r,g,b,a) (((u32)(a)<<24)|((u32)(r)<<16)|((u32)(g)<<8)|(u32)(b))

typedef struct { int x, y, w, h; } catin_rect_t;
#define RECT(x,y,w,h) ((catin_rect_t){(x),(y),(w),(h)})
int catin_rect_contains(catin_rect_t r, int px, int py);

typedef enum { CEV_NONE=0, CEV_QUIT, CEV_PEN_DOWN, CEV_PEN_UP, CEV_PEN_MOVE, CEV_KEY_DOWN, CEV_KEY_UP } catin_event_type_t;
typedef struct { int type; int x, y; int key; } catin_event_t;

typedef struct catin_canvas catin_canvas_t;
struct catin_canvas {
    int width, height;
    u32 *pixels;
    void (*clear)(catin_canvas_t*, u32);
    void (*pixel)(catin_canvas_t*, int, int, u32);
    void (*rect_fill)(catin_canvas_t*, int, int, int, int, u32);
    void (*rect_outline)(catin_canvas_t*, int, int, int, int, int, u32);
    void (*circle)(catin_canvas_t*, int, int, int, u32);
    void (*text)(catin_canvas_t*, int, int, const char*, u32);
    void (*text_center)(catin_canvas_t*, int, int, int, const char*, u32);
    void (*present)(catin_canvas_t*);
};
catin_canvas_t *catin_canvas_get(void);

typedef struct {
    catin_rect_t r;
    const char *text;
    int hover, pressed, clicked;
    u32 bg, fg, border;
} catin_button_t;
void catin_button_draw(catin_canvas_t*, catin_button_t*);
void catin_button_event(catin_button_t*, catin_event_t*);

typedef struct {
    catin_rect_t r;
    char text[256];
    int len, focused, max_len;
} catin_textbox_t;
void catin_textbox_draw(catin_canvas_t*, catin_textbox_t*);
void catin_textbox_event(catin_textbox_t*, catin_event_t*);

typedef struct {
    catin_rect_t r;
    int value, min, max, dragging;
} catin_slider_t;
void catin_slider_draw(catin_canvas_t*, catin_slider_t*);
void catin_slider_event(catin_slider_t*, catin_event_t*);

// 开关控件
typedef struct {
    catin_rect_t r;
    const char *title, *subtitle;
    int on, anim;
    void (*on_change)(void *ud, int on); void *user;
} catin_switch_t;
void catin_switch_draw(catin_canvas_t*, catin_switch_t*, catin_event_t*);
void catin_switch_event(catin_switch_t*, catin_event_t*);

// 软键盘
typedef struct {
    int visible;
} catin_keyboard_t;
void catin_keyboard_init(catin_keyboard_t *kb);
int catin_keyboard_draw_and_event(catin_canvas_t *cv, catin_keyboard_t *kb, catin_event_t *e, char *target_text, int *target_len, int max_len);

#endif