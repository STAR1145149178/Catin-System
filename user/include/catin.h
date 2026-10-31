#ifndef _CATIN_H
#define _CATIN_H
#include "catin_types.h"
#include "catin_syscall.h"
#include "catin_libc.h"
#include "catin_fs.h"
#include "catin_gui.h"
#include "catin_screen.h"
#include "catin_net.h"
#include "catin_http.h"
#include "catin_zip.h"
#include "catin_zzc.h"

extern u32 *catin_platform_fb(void);
extern void catin_platform_present(void);
extern int  catin_platform_poll(catin_event_t *e);

#define CATIN_APP(name) int main(void)
#endif
