#include "catin.h"
#include "codec.h"
#include "boot_cfg.h"
#include "log.h"
#include "security.h"
#include "virtio.h"

extern u32 g_virtio_irq;
extern void codec_run_tests(void);

static void banner(void){
    uart_puts("\n");
    uart_puts("  Catin Kernel " CATIN_VERSION " (" CATIN_CODENAME ")\n");
    uart_puts("  arch: " ARCH_NAME "\n");
    uart_puts("  codecs: WAV MP3 MP4 PNG JPG GIF\n\n");
}

void kernel_main(u64 dtb){
    uart_puts("K0\n");  arch_early_init();
    uart_puts("K1\n");  arch_uart_init();
    uart_puts("Parsing DTB...\n"); fdt_parse(dtb);
    uart_puts("K2\n");  banner();
    uart_puts("K3\n");  mm_init();
    uart_puts("K4\n");  arch_irq_init();
    uart_puts("K5\n");  arch_timer_init(1000);
    uart_puts("K6\n");  arch_irq_enable_all();
    uart_puts("K7\n");  vfs_init();
    
    log_init();
    uart_puts("K7.1: Log System Ready\n");
    boot_loader_decision();
    uart_puts("K7.2: Boot Decision Done\n");
    load_security_policy();
    uart_puts("K7.3: Security Policy Ready\n");
    
    uart_puts("K8\n");  proc_init();
    uart_puts("K9\n");  syscall_init();
    uart_puts("K10\n");

    if (fb_init() != 0) log_err("fb failed");
    uart_puts("K11\n");
    virtio_init();
    uart_puts("K12\n");
    if (touch_init() != 0) log_err("touch failed");
    uart_puts("K13\n");

    codec_run_tests();
    uart_puts("K14\n");

    shell_init();
    uart_puts("K15\n");

    // ================== 核心：硬编码注入内置应用注册表 ==================
    // 假设 build_all.sh 已经把应用放到了 /Catin System/rootfs/apps/ 下
    vfs_mkdir("/apps"); // 确保 apps 目录存在
    const char *default_registry = 
        "settings|设置|/Catin System/rootfs/apps/settings/assets/icon.png|portrait|1.0.0|sys|/Catin System/rootfs/apps/settings/bin/app\n"
        "fm|文件管理|/Catin System/rootfs/apps/fm/assets/icon.png|portrait|1.0.0|sys|/Catin System/rootfs/apps/fm/bin/app\n"
        "browser|浏览器|/Catin System/rootfs/apps/browser/assets/icon.png|portrait|1.0.0|sys|/Catin System/rootfs/apps/browser/bin/app\n";
    
    vfs_writefile("/apps/registry.db", default_registry, kstrlen(default_registry));
    log_info("Registry initialized with 3 built-in apps.");
    // ====================================================================

    boot_loader_confirm();
    log_info("ready");

    // 初始化桌面
    demo_init();

    // 主循环
    for (;;){
        shell_tick();
        demo_frame();  // 渲染桌面、读取触摸事件
        arch_wfi();
    }
}