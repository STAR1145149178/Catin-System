#ifndef _CATIN_SYSCALL_H
#define _CATIN_SYSCALL_H
#include "catin_types.h"

#define CATIN_SYS_exit        0
#define CATIN_SYS_fork        1
#define CATIN_SYS_exec        2
#define CATIN_SYS_wait        3
#define CATIN_SYS_getpid      4
#define CATIN_SYS_yield       5
#define CATIN_SYS_sleep       6
#define CATIN_SYS_write       7
#define CATIN_SYS_read        8
#define CATIN_SYS_open        9
#define CATIN_SYS_close      10
#define CATIN_SYS_lseek      11
#define CATIN_SYS_mkdir      12
#define CATIN_SYS_unlink     13
#define CATIN_SYS_readdir    14
#define CATIN_SYS_brk        15
#define CATIN_SYS_uptime     16
#define CATIN_SYS_getchar    17
#define CATIN_SYS_putchar    18
#define CATIN_SYS_ps         19
#define CATIN_SYS_stat       20
#define CATIN_SYS_rename     21
#define CATIN_SYS_socket     30
#define CATIN_SYS_connect    31
#define CATIN_SYS_send       35
#define CATIN_SYS_recv       36
#define CATIN_SYS_close_sock 37
#define CATIN_SYS_dns        38
#define CATIN_SYS_priv       50

#define PRIV_REBOOT       0
#define PRIV_SHUTDOWN     1
#define PRIV_SET_BRIGHT   2
#define PRIV_SET_VOLUME   3
#define PRIV_WIFI_CTRL    4
#define PRIV_BT_CTRL      5

long catin_sys(long nr, long a0, long a1, long a2, long a3, long a4, long a5);

int  catin_exit(int code) __attribute__((noreturn));
int  catin_fork(void);
int  catin_exec(const char *path);
int  catin_wait(int *status);
int  catin_getpid(void);
void catin_yield(void);
void catin_sleep(u32 ms);
int  catin_open(const char *path, int flags);
int  catin_close(int fd);
int  catin_read(int fd, void *buf, u32 n);
int  catin_write(int fd, const void *buf, u32 n);
int  catin_lseek(int fd, int off, int whence);
int  catin_mkdir(const char *path);
int  catin_unlink(const char *path);
int  catin_readdir(const char *path, char *out, u32 cap);
int  catin_stat(const char *path, u32 *size, int *is_dir);
void *catin_brk(void *top);
u64  catin_uptime(void);
int  catin_getchar(void);
void catin_putchar(char c);
int  catin_socket(int type);
int  catin_connect(int s, u32 ip, u16 port);
int  catin_send(int s, const void *buf, int n);
int  catin_recv(int s, void *buf, int n);
int  catin_close_sock(int s);
u32  catin_dns_lookup(const char *name);
int  catin_priv(int cmd, long arg);
#endif
