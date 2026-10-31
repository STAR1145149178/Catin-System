#include "catin.h"
#include "boot_cfg.h"

void boot_loader_decision(void) {
    boot_cfg_t cfg;
    boot_cfg_load(&cfg);

    if (cfg.boot_status == 1) {
        // 上次启动没走完就挂了
        log_err("[Boot] Last boot crashed!");
        if (cfg.current_slot == 'A') {
            cfg.current_slot = 'B';
            log_info("[Boot] Switching to Boot B (boot.b.img)");
        } else {
            log_err("[Boot] Both A and B crashed! Entering Recovery...");
            cfg.current_slot = 'A';
            cfg.crash_count = 0;
        }
    } else {
        log_info("[Boot] Booting from Boot %c", cfg.current_slot);
    }

    // 标记为启动中
    cfg.boot_status = 1;
    boot_cfg_save(&cfg);
}

void boot_loader_confirm(void) {
    boot_cfg_t cfg;
    boot_cfg_load(&cfg);
    if (cfg.boot_status == 1) {
        cfg.boot_status = 0;
        boot_cfg_save(&cfg);
        log_info("[Boot] Boot confirmed successful.");
    }
}