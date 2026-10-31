#include "catin.h"
#include "xcui.h"
void xc_input_begin(xc_input_t *in){ in->down=0;in->up=0;in->clicked=0;in->click_x=in->click_y=0;in->consumed=0; }
void xc_input_feed(xc_input_t *in, touch_ev_t ev){
    in->x=ev.x; in->y=ev.y;
    switch(ev.kind){
    case TOUCH_DOWN: in->down=1;in->holding=1;in->down_x=ev.x;in->down_y=ev.y; break;
    case TOUCH_MOVE: break;
    case TOUCH_UP: { in->up=1;in->holding=0;
        int dx=ev.x-in->down_x, dy=ev.y-in->down_y;
        if(dx<0)dx=-dx; if(dy<0)dy=-dy;
        if(dx<16&&dy<16){ in->clicked=1; in->click_x=ev.x; in->click_y=ev.y; }
        break; }
    default: break;
    }
}
void xc_input_end(xc_input_t *in){ (void)in; }
