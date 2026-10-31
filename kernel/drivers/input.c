#include "catin.h"

#define EVQ 32
static touch_ev_t q[EVQ]; 
static int h, t;

void touch_feed(touch_kind_t k, int x, int y){
    int n = (h + 1) % EVQ; 
    if (n == t) return;
    q[h].kind = k; 
    q[h].x = x; 
    q[h].y = y; 
    h = n;
}

int touch_poll(touch_ev_t *ev){
    if (h == t) return 0;
    *ev = q[t]; 
    t = (t + 1) % EVQ; 
    return 1;
}