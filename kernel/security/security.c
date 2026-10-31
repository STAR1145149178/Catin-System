#include "catin.h"
#include "security.h"

#define BL_UNLOCK_PATH "/Catin System/System/bl_unlocked.cfg"

// 受保护的终极核心文件
static const char *protected_paths[] = {
    "/Catin System/System/boot_status.cfg",
    "/Catin System/OS/boot.img",
    "/Catin System/OS/boot.b.img",
    NULL
};

// 检查 BL 锁是否处于解开状态
static int is_bl_unlocked(void) {
    char buf[8] = {0};
    int n = vfs_readfile(BL_UNLOCK_PATH, buf, sizeof(buf));
    if (n > 0 && buf[0] == '1') {
        return 1; // BL 锁已解开，拥有最高权限
    }
    return 0;
}

int security_check_write(const char *path) {
    // 如果 BL 锁已经解开，无视一切限制！
    if (is_bl_unlocked()) {
        log_info("[Security] BL Unlocked, bypassing write protection for: %s", path);
        return 1; 
    }

    // 否则，执行严格拦截（哪怕开发者模式开了也不行）
    for (int i = 0; protected_paths[i] != NULL; i++) {
        if (kstrncmp(path, protected_paths[i], kstrlen(protected_paths[i])) == 0) {
            log_err("[Security] Blocked write (BL Locked!): %s", path);
            return 0; 
        }
    }
    return 1;
}

int security_check_unlink(const char *path) {
    // 如果 BL 锁已经解开，无视一切限制！
    if (is_bl_unlocked()) {
        log_info("[Security] BL Unlocked, bypassing delete protection for: %s", path);
        return 1; 
    }

    for (int i = 0; protected_paths[i] != NULL; i++) {
        if (kstrncmp(path, protected_paths[i], kstrlen(protected_paths[i])) == 0) {
            log_err("[Security] Blocked unlink (BL Locked!): %s", path);
            return 0; 
        }
    }
    return 1;
}

void load_security_policy(void) {
    char buf[1024] = {0};
    int n = vfs_readfile("/Catin System/rootfs/security/security_policy.conf", buf, sizeof(buf));
    if (n > 0) log_info("[Security] Policy loaded.");
    else log_err("[Security] Policy not found, using default.");
}