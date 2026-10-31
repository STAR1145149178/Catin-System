#include "catin.h"
#include "gesture.h"
void gesture_init(gesture_t*g){ kmemset(g,0,sizeof(*g)); }
void gesture_set_panel(gesture_t*g,int open){ g->panel_open=open; }
void gesture_feed(gesture_t*g,touch_ev_t ev){
    switch(ev.kind){
    case TOUCH_DOWN:
        g->active=1; g->triggered=0;
        g->start_x=g->last_x=ev.x; g->start_y=g->last_y=ev.y;
        g->from_top=(ev.y<EDGE_ZONE);
        g->from_bottom=(ev.y>SCREEN_H-EDGE_ZONE);
        g->start_half=(ev.x<SCREEN_W/2)?0:1;
        g->result=G_NONE; break;
    case TOUCH_MOVE: {
        if(!g->active) break;
        g->last_x=ev.x; g->last_y=ev.y;
        if(g->triggered) break;
        int dy=g->last_y-g->start_y, dx=g->last_x-g->start_x;
        int adx=dx<0?-dx:dx;
        if(g->panel_open){
            if(dy<-SWIPE_MIN&&adx<DRIFT_MAX){ g->result=G_CLOSE_PANEL; g->triggered=1; }
            break;
        }
        if(g->from_top&&dy>0&&g->last_y>HALF_SCREEN){
            if(adx<DRIFT_MAX){ g->result=g->start_half?G_OPEN_CTRL:G_OPEN_NOTIF; g->triggered=1; }
        } else if(g->from_bottom&&dy<0&&g->last_y<HALF_SCREEN){
            if(adx<DRIFT_MAX){ g->result=G_OPEN_SHELL; g->triggered=1; }
        }
        break;
    }
    case TOUCH_UP: {
        if(g->active&&!g->triggered){
            int ddx=ev.x-g->start_x, ddy=ev.y-g->start_y;
            if(ddx<0)ddx=-ddx; if(ddy<0)ddy=-ddy;
            if(ddx<TAP_MAX&&ddy<TAP_MAX) g->result=G_TAP;
        }
        g->active=0; break;
    }
    default: break;
    }
}
gesture_kind_t gesture_take(gesture_t*g){ gesture_kind_t r=g->result; g->result=G_NONE; return r; }
