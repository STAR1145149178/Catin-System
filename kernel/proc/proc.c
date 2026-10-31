#include "catin.h"
static proc_t g_procs[PROC_MAX];
static int g_next_pid=1;
static proc_t *g_current;
static u64 g_dummy_ksp;
void proc_init(void){ kmemset(g_procs,0,sizeof(g_procs)); g_current=NULL; log_info("proc: max=%d",PROC_MAX); }
proc_t *proc_current(void){ return g_current; }
proc_t *proc_find(int pid){ for(int i=0;i<PROC_MAX;i++) if(g_procs[i].state!=PROC_UNUSED&&g_procs[i].pid==pid) return &g_procs[i]; return NULL; }
proc_t *proc_alloc(void){
    for(int i=0;i<PROC_MAX;i++) if(g_procs[i].state==PROC_UNUSED){
        kmemset(&g_procs[i],0,sizeof(proc_t));
        g_procs[i].pid=g_next_pid++;
        g_procs[i].state=PROC_READY;
        g_procs[i].kstack_top=(u64)(uintptr_t)&g_procs[i].kstack[KSTACK_SIZE];
        g_procs[i].ppid=g_current?g_current->pid:0;
        return &g_procs[i];
    }
    return NULL;
}
void proc_free(proc_t*p){ p->state=PROC_UNUSED; }
void proc_save_regs(const regs_t*r){ if(!g_current)return; kmemcpy(&g_current->regs,r,sizeof(regs_t)); }
static proc_t *pick_next(void){
    int start=0;
    if(g_current){ start=(int)(g_current-g_procs); start=(start+1)%PROC_MAX; }
    for(int i=0;i<PROC_MAX;i++){ int idx=(start+i)%PROC_MAX; if(g_procs[idx].state==PROC_READY) return &g_procs[idx]; }
    return g_current;
}
void proc_yield(void){
    proc_t*prev=g_current;
    proc_t*next=pick_next();
    if(!next||next==prev) return;
    if(prev&&prev->state==PROC_RUNNING) prev->state=PROC_READY;
    next->state=PROC_RUNNING;
    g_current=next;
    if(prev) arch_context_switch(&prev->ksp,next->ksp);
    else arch_context_switch(&g_dummy_ksp,next->ksp);
}
void proc_timer_tick(void){
    u64 now=arch_timer_ticks();
    for(int i=0;i<PROC_MAX;i++){
        proc_t*p=&g_procs[i];
        if(p->state==PROC_BLOCKED&&p->wakeup_tick&&now>=p->wakeup_tick){ p->state=PROC_READY; p->wakeup_tick=0; }
    }
}
void proc_sleep_ms(u32 ms){
    if(!g_current) return;
    g_current->state=PROC_BLOCKED;
    g_current->wakeup_tick=arch_timer_ticks()+ms;
    proc_yield();
}
static void wake_waiters(int pid){
    for(int i=0;i<PROC_MAX;i++){
        proc_t*p=&g_procs[i];
        if(p->state==PROC_BLOCKED&&p->wait_pid==pid){ p->wait_pid=0; p->state=PROC_READY; }
    }
}
void proc_exit(int code){
    if(!g_current) for(;;) arch_wfi();
    g_current->state=PROC_ZOMBIE; g_current->exit_code=code;
    wake_waiters(g_current->pid); proc_yield();
    for(;;) arch_wfi();
}
int proc_fork(void){
    proc_t*parent=g_current; if(!parent) return -1;
    proc_t*child=proc_alloc(); if(!child) return -1;
    kmemcpy(&child->regs,&parent->regs,sizeof(regs_t));
    child->regs.r[0]=0;
    child->ttbr0=parent->ttbr0; child->user_base=parent->user_base;
    child->user_size=parent->user_size; child->heap_top=parent->heap_top;
    child->stack_top=parent->stack_top; child->is_32bit=parent->is_32bit;
    kstrncpy(child->name,parent->name,PROC_NAME_MAX);
    if(parent->user_size&&parent->user_base)
        kmemcpy((void*)(uintptr_t)child->user_base,(void*)(uintptr_t)parent->user_base,parent->user_size);
    return child->pid;
}
int proc_wait(int*status){
    if(!g_current) return -1;
    proc_t*p=g_current;
    for(;;){
        for(int i=0;i<PROC_MAX;i++){
            proc_t*c=&g_procs[i];
            if(c->state==PROC_ZOMBIE&&c->ppid==p->pid){
                if(status)*status=c->exit_code;
                int pid=c->pid; proc_free(c); return pid;
            }
        }
        p->state=PROC_BLOCKED; p->wait_pid=-1; proc_yield();
        if(p->state==PROC_READY) p->state=PROC_RUNNING;
    }
}
void proc_dump(void){
    log_info("=== processes ===");
    for(int i=0;i<PROC_MAX;i++){
        proc_t*p=&g_procs[i];
        if(p->state==PROC_UNUSED) continue;
        log_info("  pid=%d ppid=%d state=%d name=%s",p->pid,p->ppid,(int)p->state,p->name);
    }
}
void proc_start_first(proc_t *p){
    if (!p) for(;;) arch_wfi();
    g_current = p;
    p->state = PROC_RUNNING;
    arch_enter_user(&p->regs);
    for(;;) arch_wfi();
}
void proc_child_entry(void){
    proc_t *p = g_current;
    if (!p) proc_exit(-1);
    arch_enter_user(&p->regs);
    for(;;) arch_wfi();
}
int user_copy_in(void*dst,u64 src_va,usize n){ kmemcpy(dst,(void*)(uintptr_t)src_va,n); return 0; }
int user_copy_out(u64 dst_va,const void*src,usize n){ kmemcpy((void*)(uintptr_t)dst_va,src,n); return 0; }
int user_str_in(char*dst,u64 src_va,usize max){
    const char*s=(const char*)(uintptr_t)src_va; usize i=0;
    while(i+1<max&&s[i]){ dst[i]=s[i]; i++; }
    dst[i]=0; return (int)i;
}