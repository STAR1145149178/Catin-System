#include "catin.h"
#include "boot_cfg.h"

void boot_cfg_save(boot_cfg_t *cfg) {
    char buf[64];
    ksnprintf(buf, sizeof(buf), "slot=%c\ncrash=%d\nstatus=%d\n", 
              cfg->current_slot, cfg->crash_count, cfg->boot_status);
    vfs_writefile(BOOT_CFG_PATH, buf, kstrlen(buf));
}

void boot_cfg_load(boot_cfg_t *cfg) {
    char buf[64] = {0};
    cfg->current_slot = 'A';
    cfg->crash_count = 0;
    cfg->boot_status = 0;

    int n = vfs_readfile(BOOT_CFG_PATH, buf, sizeof(buf));
    if (n <= 0) {
        boot_cfg_save(cfg);
        return;
    }

    char *p = buf;
    while (*p) {
        if (kstrncmp(p, "slot=", 5) == 0) cfg->current_slot = p[5];
        else if (kstrncmp(p, "crash=", 6) == 0) cfg->crash_count = katoi(p + 6);
        else if (kstrncmp(p, "status=", 7) == 0) cfg->boot_status = katoi(p + 7);
        while (*p && *p != '\n') p++;
        if (*p == '\n') p++;
    }
}