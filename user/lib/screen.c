#include "catin.h"
static catin_screen_t screens[CATIN_MAX_SCREENS];
static int n_screens;
static catin_screen_id_t cur = -1;
static catin_screen_id_t stack[CATIN_MAX_SCREENS];
static int stack_top;
catin_screen_id_t catin_screen_register(const char *name, catin_screen_t s){
    if(n_screens >= CATIN_MAX_SCREENS) return -1;
    screens[n_screens] = s;
    screens[n_screens].name = name;
    return n_screens++;
}
void catin_screen_goto(catin_screen_id_t id){
    if(id<0 || id>=n_screens) return;
    if(cur>=0 && screens[cur].leave) screens[cur].leave();
    if(stack_top < CATIN_MAX_SCREENS) stack[stack_top++] = cur;
    cur = id;
    if(screens[cur].enter) screens[cur].enter();
}
void catin_screen_back(void){
    if(stack_top==0) return;
    if(cur>=0 && screens[cur].leave) screens[cur].leave();
    cur = stack[--stack_top];
    if(cur>=0 && screens[cur].enter) screens[cur].enter();
}
void catin_screen_dispatch(catin_canvas_t *cv, catin_event_t *e){
    if(cur<0 || cur>=n_screens) return;
    if(screens[cur].update) screens[cur].update(cv, e);
}
