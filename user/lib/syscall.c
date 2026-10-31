#include "catin.h"

int catin_exit(int code){ catin_sys(0,code,0,0,0,0,0); for(;;); }
int catin_fork(void){ return catin_sys(1,0,0,0,0,0,0); }
int catin_exec(const char *p){ return catin_sys(2,(long)p,0,0,0,0,0); }
int catin_wait(int *s){ return catin_sys(3,(long)s,0,0,0,0,0); }
int catin_getpid(void){ return catin_sys(4,0,0,0,0,0,0); }
void catin_yield(void){ catin_sys(5,0,0,0,0,0,0); }
void catin_sleep(u32 ms){ catin_sys(6,ms,0,0,0,0,0); }
int catin_open(const char *p,int f){ return catin_sys(9,(long)p,f,0,0,0,0); }
int catin_close(int fd){ return catin_sys(10,fd,0,0,0,0,0); }
int catin_read(int fd,void*b,u32 n){ return catin_sys(8,fd,(long)b,n,0,0,0); }
int catin_write(int fd,const void*b,u32 n){ return catin_sys(7,fd,(long)b,n,0,0,0); }
int catin_lseek(int fd,int o,int w){ return catin_sys(11,fd,o,w,0,0,0); }
int catin_mkdir(const char*p){ return catin_sys(12,(long)p,0,0,0,0,0); }
int catin_unlink(const char*p){ return catin_sys(13,(long)p,0,0,0,0,0); }
int catin_readdir(const char*p,char*o,u32 c){ return catin_sys(14,(long)p,(long)o,c,0,0,0); }
int catin_stat(const char*p,u32*s,int*d){ return catin_sys(20,(long)p,(long)s,(long)d,0,0,0); }
u64 catin_uptime(void){ return catin_sys(16,0,0,0,0,0,0); }
int catin_getchar(void){ return catin_sys(17,0,0,0,0,0,0); }
void catin_putchar(char c){ catin_sys(18,c,0,0,0,0,0); }
int catin_socket(int t){ return catin_sys(30,t,0,0,0,0,0); }
int catin_connect(int s,u32 ip,u16 p){ return catin_sys(31,s,ip,p,0,0,0); }
int catin_send(int s,const void*b,int n){ return catin_sys(35,s,(long)b,n,0,0,0); }
int catin_recv(int s,void*b,int n){ return catin_sys(36,s,(long)b,n,0,0,0); }
int catin_close_sock(int s){ return catin_sys(37,s,0,0,0,0,0); }
u32 catin_dns_lookup(const char*n){ return (u32)catin_sys(38,(long)n,0,0,0,0,0); }
int catin_priv(int c,long a){ return catin_sys(50,c,a,0,0,0,0); }

void *catin_brk(void *top){
    long r = catin_sys(15,(long)top,0,0,0,0,0);
    return (void*)r;
}
