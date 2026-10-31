#ifndef _CATIN_SCREEN_H
#define _CATIN_SCREEN_H
#include "catin_gui.h"
#define CATIN_MAX_SCREENS 16
typedef int catin_screen_id_t;
typedef struct {
    const char *name;
    void (*enter)(void);
    void (*update)(catin_canvas_t*, catin_event_t*);
    void (*leave)(void);
} catin_screen_t;
catin_screen_id_t catin_screen_register(const char *name, catin_screen_t s);
void catin_screen_goto(catin_screen_id_t id);
void catin_screen_back(void);
void catin_screen_dispatch(catin_canvas_t*, catin_event_t*);
#endif
