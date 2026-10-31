#include "catin.h"
int catin_fs_open(const char*p,int f){ return catin_open(p,f); }
int catin_fs_close(int fd){ return catin_close(fd); }
int catin_fs_read(int fd,void*b,u32 n){ return catin_read(fd,b,n); }
int catin_fs_write(int fd,const void*b,u32 n){ return catin_write(fd,b,n); }
int catin_fs_seek(int fd,int o,int w){ return catin_lseek(fd,o,w); }
int catin_fs_mkdir(const char*p){ return catin_mkdir(p); }
int catin_fs_rm(const char*p){ return catin_unlink(p); }
int catin_fs_stat(const char*p,u32*s,int*d){ return catin_stat(p,s,d); }
int catin_fs_rename(const char*o,const char*n){ return catin_sys(21,(long)o,(long)n,0,0,0,0); }
int catin_fs_exists(const char*p){ u32 s; int d; return catin_stat(p,&s,&d)==0; }
int catin_fs_list(const char*path,catin_dirent_t*out,int max){
    static char buf[8192];
    int n = catin_readdir(path,buf,sizeof(buf));
    if(n<=0) return 0;
    int cnt = 0;
    char *p = buf;
    while(*p && cnt < max){
        char *nl = p;
        while(*nl && *nl != '\n') nl++;
        char sv = *nl; *nl = 0;
        int is_dir = 0;
        int len = catin_strlen(p);
        if(len > 0 && p[len-1] == '/'){ is_dir = 1; p[len-1] = 0; }
        catin_strncpy(out[cnt].name, p, sizeof(out[cnt].name)-1);
        out[cnt].is_dir = is_dir;
        char full[512];
        catin_snprintf(full, sizeof(full), "%s/%s", path, p);
        u32 sz = 0; int dd = 0;
        catin_fs_stat(full, &sz, &dd);
        out[cnt].size = sz;
        cnt++;
        *nl = sv;
        p = (*nl) ? nl+1 : nl;
    }
    return cnt;
}
char *catin_fs_readfile(const char*path,u32*out_size){
    int fd = catin_fs_open(path, CATIN_O_RDONLY);
    if(fd < 0) return NULL;
    u32 cap = 4096, len = 0;
    char *buf = catin_malloc(cap+1);
    for(;;){
        if(len == cap){ cap *= 2; buf = catin_realloc(buf, cap+1); }
        int r = catin_fs_read(fd, buf+len, cap-len);
        if(r <= 0) break;
        len += r;
    }
    buf[len] = 0;
    catin_fs_close(fd);
    if(out_size) *out_size = len;
    return buf;
}
int catin_fs_writefile(const char*path,const void*data,u32 size){
    int fd = catin_fs_open(path, CATIN_O_WRONLY|CATIN_O_CREATE|CATIN_O_TRUNC);
    if(fd < 0) return -1;
    int w = catin_fs_write(fd, data, size);
    catin_fs_close(fd);
    return w;
}
