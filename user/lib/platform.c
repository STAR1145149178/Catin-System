#include "catin.h"

// 这些是给 gui.c 提供底层支持的函数
u32 *catin_platform_fb(void) {
    // 告诉系统：请把屏幕的显存地址给我 (syscall 随便定一个号，比如 100)
    // 如果你内核里还没写这个 syscall，这里先返回 0（后续再补）
    // return (u32*)catin_sys(100, 0, 0, 0, 0, 0, 0);
    return (u32*)0; 
}

void catin_platform_present(void) {
    // 告诉系统：画面画完了，请刷新到屏幕上 (syscall 101)
    // catin_sys(101, 0, 0, 0, 0, 0, 0);
}

int catin_platform_poll(catin_event_t *e) {
    // 告诉系统：有没有触摸事件？(syscall 102)
    // return (int)catin_sys(102, (long)e, 0, 0, 0, 0, 0);
    return 0; 
}