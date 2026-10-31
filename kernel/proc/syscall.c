#include "catin.h"
static int current_pid(void){ proc_t*p=proc_current(); return p?p->pid:-1; }
u64 syscall_dispatch(u64 nr,u64 a0,u64 a1,u64 a2,u64 a3,u64 a4,u64 a5){
    (void)a3;(void)a4;(void)a5;
    switch(nr){
    case SYS_exit: proc_exit((int)a0); return 0;
    case SYS_fork: return (u64)proc_fork();
    case SYS_exec: return (u64)-1;
    case SYS_wait: return (u64)proc_wait((int*)(uintptr_t)a0);
    case SYS_getpid: return (u64)current_pid();
    case SYS_yield: proc_yield(); return 0;
    case SYS_sleep: proc_sleep_ms((u32)a0); return 0;
    case SYS_write: return (u64)vfs_write((int)a0,(void*)(uintptr_t)a1,(u32)a2);
    case SYS_read: return (u64)vfs_read((int)a0,(void*)(uintptr_t)a1,(u32)a2);
    case SYS_open: { char p[PATH_MAX]; user_str_in(p,a0,PATH_MAX); return (u64)vfs_open(p,(int)a1); }
    case SYS_close: return (u64)vfs_close((int)a0);
    case SYS_lseek: return (u64)vfs_lseek((int)a0,(int)a1,(int)a2);
    case SYS_mkdir: { char p[PATH_MAX]; user_str_in(p,a0,PATH_MAX); return (u64)vfs_mkdir(p); }
    case SYS_unlink: { char p[PATH_MAX]; user_str_in(p,a0,PATH_MAX); return (u64)vfs_unlink(p); }
    case SYS_readdir: { char p[PATH_MAX]; user_str_in(p,a0,PATH_MAX); return (u64)vfs_readdir(p,(char*)(uintptr_t)a1,(u32)a2); }
    case SYS_brk: { proc_t*p=proc_current(); if(!p)return -1; p->heap_top=a0; return a0; }
    case SYS_uptime: return arch_timer_ticks();
    case SYS_getchar: return (u64)(char)arch_uart_getc();
    case SYS_putchar: arch_uart_putc((char)a0); return 0;
    case SYS_ps: proc_dump(); return 0;
    default: return (u64)-1;
    }
}
void syscall_init(void){ log_info("syscall: %d calls", SYS_max); }
