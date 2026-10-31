#ifndef _BOOT_CFG_H
#define _BOOT_CFG_H
#include "types.h"

#define BOOT_CFG_PATH "/Catin System/System/boot_status.cfg"

typedef struct {
    char current_slot;
    int crash_count;
    int boot_status;
} boot_cfg_t;

void boot_cfg_load(boot_cfg_t *cfg);
void boot_cfg_save(boot_cfg_t *cfg);
void boot_loader_decision(void);
void boot_loader_confirm(void);

#endif