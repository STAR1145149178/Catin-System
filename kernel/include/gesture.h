#ifndef _G_H
#define _G_H
#include "types.h"
typedef enum { G_NONE=0, G_OPEN_NOTIF, G_OPEN_CTRL, G_OPEN_SHELL, G_CLOSE_PANEL, G_TAP } gesture_kind_t;
typedef enum { PANEL_NONE=0, PANEL_NOTIF, PANEL_CTRL, PANEL_SHELL } panel_id_t;
typedef struct {
    int active, start_x, start_y, last_x, last_y;
    int from_top, from_bottom, start_half, triggered, panel_open;
    gesture_kind_t result;
} gesture_t;
#define EDGE_ZONE (SCREEN_H/10)
#define HALF_SCREEN (SCREEN_H/2)
#define SWIPE_MIN 60
#define TAP_MAX 14
#define DRIFT_MAX 80
void gesture_init(gesture_t *g);
void gesture_feed(gesture_t *g, touch_ev_t ev);
gesture_kind_t gesture_take(gesture_t *g);
void gesture_set_panel(gesture_t *g, int open);
#endif
