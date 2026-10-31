#include "catin.h"

static int g_log_fd = -1;

void log_init(void) {
    // O_TRUNC 覆盖写入
    g_log_fd = vfs_open("/Catin System/log/boot.log", O_CREATE | O_TRUNC | O_WRONLY);
    if (g_log_fd >= 0) {
        vfs_write(g_log_fd, "=== Catin OS Boot Log ===\n", 26);
    }
}

// 覆盖你原来的 arch_common.c 里的 uart_puts
void uart_puts(const char *s) {
    const char *p = s;
    while (*p) { 
        if (*p == '\n') arch_uart_putc('\r'); 
        arch_uart_putc(*p++); 
    }
    if (g_log_fd >= 0) {
        // 这里用你手写的 kstrlen
        vfs_write(g_log_fd, s, kstrlen(s));
    }
}