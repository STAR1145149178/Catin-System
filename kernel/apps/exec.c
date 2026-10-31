#include "catin.h"
#include "elf.h"

// 预留给应用的用户栈大小 (1MB)
#define USER_STACK_SIZE (1024 * 1024)
static u8 *g_user_stack = NULL;

// 引用汇编里写好的跳转函数
extern void arch_enter_user(regs_t *r) __attribute__((noreturn));

void launch_app(const char *appid, const char *entry_path) {
    char full_path[256];
    // 拼接完整路径：/apps/{appid}/{entry_path}
    ksnprintf(full_path, sizeof(full_path), "/apps/%s/%s", appid, entry_path);

    u32 sz = 0;
    void *app_data = vfs_readfile(full_path, &sz);
    if (!app_data) {
        log_err("Exec: App not found: %s", full_path);
        return;
    }

    elf64_header_t *elf = (elf64_header_t *)app_data;
    if (elf->e_magic != ELF_MAGIC) {
        log_err("Exec: Not a valid ELF file!");
        kfree(app_data);
        return;
    }

    log_info("Exec: Loading %s (Entry: 0x%lx)", full_path, elf->e_entry);

    // 1. 将程序头表中的 LOAD 段（代码和数据）拷贝到目标物理内存
    elf64_phdr_t *phdr = (elf64_phdr_t *)((u8*)app_data + elf->e_phoff);
    for (int i = 0; i < elf->e_phnum; i++) {
        if (phdr[i].p_type == 1) { // PT_LOAD
            void *dest = (void *)(uintptr_t)phdr[i].p_vaddr;
            void *src  = (void *)((u8*)app_data + phdr[i].p_offset);
            
            kmemcpy(dest, src, phdr[i].p_filesz);
            
            // 处理 BSS 段（.bss 是未初始化的全局变量，ELF里不占文件大小，但在内存里占，必须清零）
            if (phdr[i].p_memsz > phdr[i].p_filesz) {
                kmemset((u8*)dest + phdr[i].p_filesz, 0, phdr[i].p_memsz - phdr[i].p_filesz);
            }
        }
    }

    // 2. 分配用户栈内存
    if (!g_user_stack) {
        g_user_stack = (u8*)kmalloc(USER_STACK_SIZE);
        if (!g_user_stack) {
            log_err("Exec: Failed to allocate user stack!");
            kfree(app_data);
            return;
        }
    }
    
    // 3. 构造用户态寄存器环境
    regs_t regs;
    kmemset(&regs, 0, sizeof(regs));
    
    // PC 指向编译时的入口（你在 build_all.sh 里指定的 0x200000）
    regs.pc = elf->e_entry;
    
    // SP（用户栈指针）指向分配的栈最高处，ARM64 栈是从高往低生长的
    regs.sp_user = (u64)(uintptr_t)(g_user_stack + USER_STACK_SIZE - 16);
    
    // 默认传参 (x0=argc, x1=argv)
    regs.r[0] = 1; 
    regs.r[1] = 0; 

    kfree(app_data); // 代码已经搬到物理内存了，释放文件缓存

    log_info("Exec: Jumping to user mode at 0x%lx...", elf->e_entry);
    
    // 4. 通过汇编切入用户态（警告：正常情况下，这条函数永远不会返回）
    arch_enter_user(&regs);
}