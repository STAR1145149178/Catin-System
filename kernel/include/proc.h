#ifndef _PROC_H
#define _PROC_H
#include "types.h"
#include "arch.h"
#define PROC_MAX 32
#define KSTACK_SIZE 16384
#define PROC_NAME_MAX 32
typedef enum { PROC_UNUSED=0, PROC_READY, PROC_RUNNING, PROC_BLOCKED, PROC_ZOMBIE } proc_state_t;
typedef struct proc {
    int pid, ppid, exit_code;
    proc_state_t state;
    regs_t regs;
    u8 kstack[KSTACK_SIZE];
    u64 kstack_top;
    u64 ksp;
    u64 ttbr0, user_base, user_size, heap_top, stack_top;
    int is_32bit;
    int wait_pid;
    u64 wakeup_tick;
    char name[PROC_NAME_MAX];
} proc_t;
void proc_init(void);
proc_t *proc_current(void);
proc_t *proc_find(int pid);
proc_t *proc_alloc(void);
void proc_free(proc_t *p);
void proc_start_first(proc_t *p) __attribute__((noreturn));
void proc_yield(void);
void proc_exit(int code) __attribute__((noreturn));
int proc_fork(void);
int proc_wait(int *status);
void proc_sleep_ms(u32 ms);
void proc_timer_tick(void);
void proc_dump(void);
void proc_save_regs(const regs_t *r);
int  user_copy_in(void *dst, u64 src_va, usize n);
int  user_copy_out(u64 dst_va, const void *src, usize n);
int  user_str_in(char *dst, u64 src_va, usize max);
#endif
