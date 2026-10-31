#include "catin.h"

// 直接把 ELF 结构体定义在这里，抛弃外部头文件
#define ELF_MAGIC 0x464C457F

typedef struct {
    u32 e_magic; u8 e_ident[12]; u16 e_type; u16 e_machine; u32 e_version;
    u64 e_entry; u64 e_phoff; u64 e_shoff; u32 e_flags; u16 e_ehsize;
    u16 e_phentsize; u16 e_phnum; u16 e_shentsize; u16 e_shnum; u16 e_shstrndx;
} elf64_header_t;

typedef struct {
    u32 p_type; u32 p_flags; u64 p_offset; u64 p_vaddr; u64 p_paddr;
    u64 p_filesz; u64 p_memsz; u64 p_align;
} elf64_phdr_t;

#define USER_STACK_SIZE (1024 * 1024)
static u8 *g_user_stack = NULL;

extern void arch_enter_user(regs_t *r) __attribute__((noreturn));

void launch_app(const char *appid, const char *entry_path) {
    char full_path[256];
    
    // 修改点：如果注册表里给的是绝对路径，就直接用
    if (entry_path[0] == '/') {
        ksnprintf(full_path, sizeof(full_path), "%s", entry_path);
    } else {
        ksnprintf(full_path, sizeof(full_path), "/Catin System/rootfs/apps/%s/%s", appid, entry_path);
    }

    // 1. 先获取应用文件的大小
    u32 file_size = 0;
    int is_dir = 0;
    if (vfs_stat(full_path, &file_size, &is_dir) != 0 || file_size == 0) {
        log_err("Exec: App not found or empty: %s", full_path);
        return;
    }

    // 2. 给应用分配内存
    void *app_data = kmalloc(file_size);
    if (!app_data) {
        log_err("Exec: OOM loading app");
        return;
    }

    // 3. 把文件读进内存
    int n = vfs_readfile(full_path, app_data, file_size);
    if (n <= 0) {
        log_err("Exec: Failed to read app");
        kfree(app_data);
        return;
    }

    elf64_header_t *elf = (elf64_header_t *)app_data;
    if (elf->e_magic != ELF_MAGIC) { 
        log_err("Exec: Not a valid ELF!"); 
        kfree(app_data); 
        return; 
    }

    log_info("Exec: Loading %s (Entry: 0x%lx)", full_path, elf->e_entry);

    // 4. 将 ELF 段搬运到目标物理地址
    elf64_phdr_t *phdr = (elf64_phdr_t *)((u8*)app_data + elf->e_phoff);
    for (int i = 0; i < elf->e_phnum; i++) {
        if (phdr[i].p_type == 1) { // PT_LOAD
            void *dest = (void *)(uintptr_t)phdr[i].p_vaddr;
            void *src  = (void *)((u8*)app_data + phdr[i].p_offset);
            
            kmemcpy(dest, src, phdr[i].p_filesz);
            
            if (phdr[i].p_memsz > phdr[i].p_filesz) {
                kmemset((u8*)dest + phdr[i].p_filesz, 0, phdr[i].p_memsz - phdr[i].p_filesz);
            }
        }
    }

    // 5. 分配用户栈
    if (!g_user_stack) {
        g_user_stack = (u8*)kmalloc(USER_STACK_SIZE);
        if (!g_user_stack) { log_err("Exec: Failed to allocate stack!"); kfree(app_data); return; }
    }
    
    // 6. 设置寄存器，准备跳转
    regs_t regs;
    kmemset(&regs, 0, sizeof(regs));
    regs.pc = elf->e_entry;
    regs.sp_user = (u64)(uintptr_t)(g_user_stack + USER_STACK_SIZE - 16);
    regs.r[0] = 1; 
    regs.r[1] = 0; 

    kfree(app_data); 
    log_info("Exec: Jumping to user mode at 0x%lx...", elf->e_entry);
    
    // 7. 进入用户态！
    arch_enter_user(&regs);
}