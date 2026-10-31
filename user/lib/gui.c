#include "catin.h"

int catin_rect_contains(catin_rect_t r,int px,int py){
    return px>=r.x && py>=r.y && px<r.x+r.w && py<r.y+r.h;
}

extern u32 *catin_platform_fb(void);
extern void catin_platform_present(void);

static catin_canvas_t g_cv;

static void cv_clear(catin_canvas_t*c,u32 col){ for(int i=0;i<c->width*c->height;i++) c->pixels[i]=col; }
static void cv_pixel(catin_canvas_t*c,int x,int y,u32 col){
    if(x<0||y<0||x>=c->width||y>=c->height) return;
    c->pixels[y*c->width+x] = col;
}
static void cv_rect_fill(catin_canvas_t*c,int x,int y,int w,int h,u32 col){
    for(int j=0;j<h;j++) for(int i=0;i<w;i++) cv_pixel(c,x+i,y+j,col);
}
static void cv_rect_outline(catin_canvas_t*c,int x,int y,int w,int h,int t,u32 col){
    for(int k=0;k<t;k++){
        for(int i=0;i<w;i++){ cv_pixel(c,x+i,y+k,col); cv_pixel(c,x+i,y+h-1-k,col); }
        for(int j=0;j<h;j++){ cv_pixel(c,x+k,y+j,col); cv_pixel(c,x+w-1-k,y+j,col); }
    }
}
static void cv_circle(catin_canvas_t*c,int cx,int cy,int r,u32 col){
    for(int y=-r;y<=r;y++) for(int x=-r;x<=r;x++) if(x*x+y*y<=r*r) cv_pixel(c,cx+x,cy+y,col);
}
extern const u8 font8x16[128][16];
static void cv_text(catin_canvas_t*c,int x,int y,const char*s,u32 col){
    while(*s){
        u8 ch = (u8)*s++;
        if(ch > 0x8F) ch='?';
        const u8 *rows = font8x16[ch];
        for(int r=0;r<16;r++){
            u8 bits=rows[r];
            for(int k=0;k<8;k++) if(bits&(0x80>>k)) cv_pixel(c,x+k,y+r,col);
        }
        x += 9;
    }
}
static void cv_text_center(catin_canvas_t*c,int cx,int y,int w,const char*s,u32 col){
    (void)w;
    int len = catin_strlen(s);
    cv_text(c, cx - len*4, y, s, col);
}
static void cv_present(catin_canvas_t*c){ (void)c; catin_platform_present(); }

catin_canvas_t *catin_canvas_get(void){
    if(!g_cv.pixels){
        g_cv.width = 720; g_cv.height = 1280;
        g_cv.pixels = (u32*)catin_platform_fb();
        g_cv.clear = cv_clear;
        g_cv.pixel = cv_pixel;
        g_cv.rect_fill = cv_rect_fill;
        g_cv.rect_outline = cv_rect_outline;
        g_cv.circle = cv_circle;
        g_cv.text = cv_text;
        g_cv.text_center = cv_text_center;
        g_cv.present = cv_present;
    }
    return &g_cv;
}

void catin_button_draw(catin_canvas_t*cv,catin_button_t*b){
    u32 bg = b->pressed ? 0xFF404040 : (b->hover ? 0xFF505050 : b->bg);
    cv->rect_fill(cv, b->r.x, b->r.y, b->r.w, b->r.h, bg);
    cv->rect_outline(cv, b->r.x, b->r.y, b->r.w, b->r.h, 2, b->border);
    cv->text_center(cv, b->r.x + b->r.w/2, b->r.y + b->r.h/2 - 8, b->r.w, b->text, b->fg);
}
void catin_button_event(catin_button_t*b,catin_event_t*e){
    switch(e->type){
    case CEV_PEN_MOVE: b->hover = catin_rect_contains(b->r, e->x, e->y); break;
    case CEV_PEN_DOWN: if(catin_rect_contains(b->r, e->x, e->y)) b->pressed=1; break;
    case CEV_PEN_UP:
        if(b->pressed && catin_rect_contains(b->r, e->x, e->y)) b->clicked=1;
        b->pressed=0; break;
    default: break;
    }
}
void catin_textbox_draw(catin_canvas_t*cv,catin_textbox_t*t){
    cv->rect_fill(cv, t->r.x, t->r.y, t->r.w, t->r.h, C_BLACK);
    cv->rect_outline(cv, t->r.x, t->r.y, t->r.w, t->r.h, 2, t->focused?C_BLUE:C_GRAY);
    cv->text(cv, t->r.x+8, t->r.y+t->r.h/2-8, t->text, C_WHITE);
    if(t->focused){
        int cx = t->r.x + 8 + t->len * 9;
        cv->rect_fill(cv, cx, t->r.y+6, 2, t->r.h-12, C_WHITE);
    }
}
void catin_textbox_event(catin_textbox_t*t,catin_event_t*e){
    switch(e->type){
    case CEV_PEN_DOWN: t->focused = catin_rect_contains(t->r, e->x, e->y); break;
    case CEV_KEY_DOWN:
        if(!t->focused) break;
        if(e->key == 8){ if(t->len>0) t->text[--t->len]=0; }
        else if(e->key >= 32 && e->key < 127 && t->len < t->max_len-1){
            t->text[t->len++]=(char)e->key;
            t->text[t->len]=0;
        }
        break;
    default: break;
    }
}
void catin_slider_draw(catin_canvas_t*cv,catin_slider_t*s){
    int cy = s->r.y + s->r.h/2;
    cv->rect_fill(cv, s->r.x, cy-3, s->r.w, 6, C_GRAY);
    int range = s->max - s->min; if(range<=0) range=1;
    int pos = (s->value - s->min) * s->r.w / range;
    cv->rect_fill(cv, s->r.x, cy-3, pos, 6, C_BLUE);
    cv->rect_fill(cv, s->r.x + pos - 8, s->r.y, 16, s->r.h, C_WHITE);
}
void catin_slider_event(catin_slider_t*s,catin_event_t*e){
    switch(e->type){
    case CEV_PEN_DOWN: if(catin_rect_contains(s->r, e->x, e->y)) s->dragging=1; break;
    case CEV_PEN_UP: s->dragging=0; break;
    case CEV_PEN_MOVE:
        if(s->dragging){
            int rel = e->x - s->r.x;
            if(rel<0) rel=0; if(rel>s->r.w) rel=s->r.w;
            s->value = s->min + rel * (s->max - s->min) / s->r.w;
        }
        break;
    default: break;
    }
}

// ================= 开关控件 =================
void catin_switch_draw(catin_canvas_t *cv, catin_switch_t *s, catin_event_t *e) {
    (void)e;
    int row_h = s->r.h, tw = 80, th = 40;
    int tx = s->r.x + s->r.w - tw - 20, ty = s->r.y + (row_h - th) / 2;
    
    cv->text(cv, s->r.x + 20, s->r.y + 20, s->title, C_WHITE);
    if (s->subtitle) {
        cv->text(cv, s->r.x + 20, s->r.y + 44, s->subtitle, C_GRAY);
    }
    
    int tg = s->on ? 100 : 0;
    if (s->anim < tg) s->anim += 20;
    if (s->anim > tg) s->anim -= 20;
    if (s->anim < 0) s->anim = 0;
    if (s->anim > 100) s->anim = 100;
    
    u32 bg = (s->anim > 50) ? C_GREEN : C_GRAY;
    cv->rect_fill(cv, tx, ty, tw, th, bg);
    
    int kr = th/2 - 4;
    int kx = tx + 4 + kr + (tw - th) * s->anim / 100;
    cv->circle(cv, kx, ty + th/2, kr, C_WHITE);
}
void catin_switch_event(catin_switch_t *s, catin_event_t *e) {
    if (e->type == CEV_PEN_UP && catin_rect_contains(s->r, e->x, e->y)) {
        s->on = !s->on;
        if (s->on_change) s->on_change(s->user, s->on);
    }
}

// ================= 软键盘 =================
static const char *keymap[6] = {
    "1234567890",
    "qwertyuiop",
    "asdfghjkl_",
    "zxcvbnm./:",
    "-=+()[]{}@",
    "        <"
};

void catin_keyboard_init(catin_keyboard_t *kb) {
    kb->visible = 0;
}

int catin_keyboard_draw_and_event(catin_canvas_t *cv, catin_keyboard_t *kb, catin_event_t *e, char *target_text, int *target_len, int max_len) {
    if (!kb->visible) return 0;

    int W = cv->width;
    int H = cv->height;
    int key_w = W / 10;
    int key_h = 60;
    int start_y = H - (6 * key_h) - 20;

    cv->rect_fill(cv, 0, start_y - 10, W, (6 * key_h) + 30, C_DARK);

    int trigger_search = 0;

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 10; col++) {
            char ch = keymap[row][col];
            if (ch == '\0') continue;

            int kx = col * key_w;
            int ky = start_y + row * key_h;
            
            cv->rect_fill(cv, kx + 2, ky + 2, key_w - 4, key_h - 4, C_GRAY);
            char str[2] = {ch, 0};
            cv->text_center(cv, kx + key_w/2, ky + key_h/2 - 8, key_w, str, C_WHITE);

            if (e->type == CEV_PEN_DOWN && e->x >= kx && e->x < kx + key_w && e->y >= ky && e->y < ky + key_h) {
                if (*target_len < max_len - 1) {
                    target_text[*target_len] = ch;
                    (*target_len)++;
                    target_text[*target_len] = 0;
                }
            }
        }
    }

    int ky = start_y + 5 * key_h;
    // 删除键
    cv->rect_fill(cv, 2, ky + 2, key_w*4 - 4, key_h - 4, C_RED);
    cv->text_center(cv, key_w*2, ky + key_h/2 - 8, key_w*4, "DEL", C_WHITE);
    if (e->type == CEV_PEN_DOWN && e->x >= 0 && e->x < key_w*4 && e->y >= ky && e->y < ky + key_h) {
        if (*target_len > 0) {
            (*target_len)--;
            target_text[*target_len] = 0;
        }
    }

    // 回车键
    cv->rect_fill(cv, key_w*4 + 2, ky + 2, key_w*6 - 4, key_h - 4, C_BLUE);
    cv->text_center(cv, key_w*7, ky + key_h/2 - 8, key_w*6, "ENTER", C_WHITE);
    if (e->type == CEV_PEN_DOWN && e->x >= key_w*4 && e->x < W && e->y >= ky && e->y < ky + key_h) {
        trigger_search = 1;
    }

    return trigger_search;
}