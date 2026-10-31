#include "catin.h"
#include "security.h"

static inode_t g_inodes[INODE_MAX];
static file_t g_files[FD_MAX];
static int g_root;

static int alloc_inode(void){ for(int i=0;i<INODE_MAX;i++) if(!g_inodes[i].used) return i; return -1; }
static int alloc_file(void){ for(int i=0;i<FD_MAX;i++) if(!g_files[i].used) return i; return -1; }
static int find_child(int p,const char*n){
    inode_t*pi=&g_inodes[p];
    for(int i=0;i<pi->n_children;i++) if(!kstrcmp(g_inodes[pi->children[i]].name,n)) return pi->children[i];
    return -1;
}
static int path_lookup(const char*path){
    if(!path||path[0]!='/') return -1;
    int cur=g_root; const char*p=path+1; char nm[NAME_MAX];
    while(*p){ int n=0; while(*p&&*p!='/'&&n<NAME_MAX-1) nm[n++]=*p++; nm[n]=0; if(*p=='/')p++;
        if(!n)continue; cur=find_child(cur,nm); if(cur<0)return -1; }
    return cur;
}
static int path_parent(const char*path,char*name){
    if(!path||path[0]!='/') return -1;
    char tmp[PATH_MAX]; kstrncpy(tmp,path,PATH_MAX);
    usize l=kstrlen(tmp); while(l>1&&tmp[l-1]=='/')tmp[--l]=0;
    char*slash=0; for(char*s=tmp;*s;s++) if(*s=='/')slash=s;
    if(!slash)return -1;
    kstrncpy(name,slash+1,NAME_MAX);
    if(slash==tmp) return g_root;
    *slash=0; return path_lookup(tmp);
}

// 辅助函数：递归创建目录
void vfs_mkdir_p(const char *path) {
    char tmp[PATH_MAX];
    kstrncpy(tmp, path, PATH_MAX);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            vfs_mkdir(tmp); 
            *p = '/';
        }
    }
    vfs_mkdir(tmp);
}

void vfs_init(void){
    kmemset(g_inodes,0,sizeof(g_inodes)); kmemset(g_files,0,sizeof(g_files));
    g_root=alloc_inode();
    g_inodes[g_root].used=1; g_inodes[g_root].is_dir=1;
    g_inodes[g_root].name[0]='/'; g_inodes[g_root].parent=-1;
    vfs_mkdir("/dev"); vfs_mkdir("/home"); vfs_mkdir("/bin"); vfs_mkdir("/tmp"); vfs_mkdir("/apps");
    vfs_writefile("/readme.txt","Catin OS 2.2\n",13);
    
    // 自动生成你截图里的 Catin System 目录结构
    vfs_mkdir_p("/Catin System");
    vfs_mkdir_p("/Catin System/OS");
    vfs_mkdir_p("/Catin System/rootfs");
    vfs_mkdir_p("/Catin System/rootfs/apps");
    vfs_mkdir_p("/Catin System/rootfs/security");
    vfs_mkdir_p("/Catin System/System");
    vfs_mkdir_p("/Catin System/log");

    log_info("vfs: ready");
}

int vfs_open(const char*path,int flags){
    // [新增] 安全拦截：阻止普通应用写入受保护文件
    if ((flags & O_WRONLY) || (flags & O_RDWR) || (flags & O_CREATE)) {
        if (!security_check_write(path)) return -1;
    }

    int idx=path_lookup(path);
    if(idx<0){ if(!(flags&O_CREATE)) return -1;
        char name[NAME_MAX]; int par=path_parent(path,name);
        if(par<0||!g_inodes[par].is_dir) return -1;
        if(g_inodes[par].n_children>=DIR_CHILDREN) return -1;
        idx=alloc_inode(); if(idx<0) return -1;
        g_inodes[idx].used=1; g_inodes[idx].is_dir=0;
        kstrncpy(g_inodes[idx].name,name,NAME_MAX); g_inodes[idx].parent=par;
        g_inodes[par].children[g_inodes[par].n_children++]=idx;
    }
    if((flags&O_TRUNC)&&!g_inodes[idx].is_dir){
        if(g_inodes[idx].data) kfree(g_inodes[idx].data);
        g_inodes[idx].data=NULL; g_inodes[idx].size=0;
    }
    int fd=alloc_file(); if(fd<0) return -1;
    g_files[fd].used=1; g_files[fd].inode_idx=idx; g_files[fd].pos=0; g_files[fd].flags=flags;
    g_inodes[idx].ref++;
    return fd;
}

int vfs_close(int fd){ if(fd<0||fd>=FD_MAX||!g_files[fd].used) return -1;
    g_inodes[g_files[fd].inode_idx].ref--; g_files[fd].used=0; return 0; }

int vfs_read(int fd,void*buf,unsigned int n){
    if(fd<0||fd>=FD_MAX||!g_files[fd].used) return -1;
    inode_t*ino=&g_inodes[g_files[fd].inode_idx];
    if(ino->is_dir) return -1;
    unsigned int left=ino->size>g_files[fd].pos?ino->size-g_files[fd].pos:0;
    if(n>left)n=left;
    if(n&&ino->data) kmemcpy(buf,(u8*)ino->data+g_files[fd].pos,n);
    g_files[fd].pos+=n; return (int)n;
}

int vfs_write(int fd,const void*buf,unsigned int n){
    if(fd<0||fd>=FD_MAX||!g_files[fd].used) return -1;
    inode_t*ino=&g_inodes[g_files[fd].inode_idx];
    if(ino->is_dir) return -1;
    unsigned int end=g_files[fd].pos+n;
    if(end>ino->size){ void*nd=kmalloc(end); if(!nd) return -1;
        if(ino->data){ kmemcpy(nd,ino->data,ino->size); kfree(ino->data); }
        ino->data=nd; ino->size=end; }
    kmemcpy((u8*)ino->data+g_files[fd].pos,buf,n);
    g_files[fd].pos=end; return (int)n;
}

int vfs_lseek(int fd,int off,int whence){
    if(fd<0||fd>=FD_MAX||!g_files[fd].used) return -1;
    inode_t*ino=&g_inodes[g_files[fd].inode_idx];
    if(whence==0)g_files[fd].pos=off; else if(whence==1)g_files[fd].pos+=off;
    else if(whence==2)g_files[fd].pos=ino->size+off;
    return (int)g_files[fd].pos;
}

int vfs_mkdir(const char*path){
    if(path_lookup(path)>=0) return -1;
    char name[NAME_MAX]; int par=path_parent(path,name);
    if(par<0) return -1;
    if(g_inodes[par].n_children>=DIR_CHILDREN) return -1;
    int idx=alloc_inode(); if(idx<0) return -1;
    g_inodes[idx].used=1; g_inodes[idx].is_dir=1;
    kstrncpy(g_inodes[idx].name,name,NAME_MAX); g_inodes[idx].parent=par;
    g_inodes[par].children[g_inodes[par].n_children++]=idx;
    return 0;
}

int vfs_unlink(const char*path){
    // [新增] 安全拦截：阻止删除受保护文件
    if (!security_check_unlink(path)) return -1;

    int idx=path_lookup(path); if(idx<0||idx==g_root) return -1;
    inode_t*ino=&g_inodes[idx]; if(ino->is_dir&&ino->n_children) return -1;
    if(ino->data) kfree(ino->data);
    int par=ino->parent; inode_t*p=&g_inodes[par];
    for(int i=0;i<p->n_children;i++) if(p->children[i]==idx){
        for(int j=i;j<p->n_children-1;j++) p->children[j]=p->children[j+1];
        p->n_children--; break;
    }
    ino->used=0; return 0;
}

int vfs_readdir(const char*path,char*out,unsigned int cap){
    int idx=path_lookup(path); if(idx<0||!g_inodes[idx].is_dir) return -1;
    inode_t*d=&g_inodes[idx]; unsigned int p=0;
    for(int i=0;i<d->n_children;i++){
        inode_t*c=&g_inodes[d->children[i]];
        int n=ksnprintf(out+p,cap-p,"%s%s\n",c->name,c->is_dir?"/":"");
        if(n<0||(unsigned)(n)>=cap-p) break;
        p+=n;
    }
    return (int)p;
}

int vfs_stat(const char*path,unsigned int*size,int*is_dir){
    int idx=path_lookup(path); if(idx<0) return -1;
    if(size)*size=g_inodes[idx].size; if(is_dir)*is_dir=g_inodes[idx].is_dir;
    return 0;
}

int vfs_readfile(const char*path,void*buf,unsigned int cap){
    int fd=vfs_open(path,O_RDONLY); if(fd<0) return -1;
    int n=vfs_read(fd,buf,cap); vfs_close(fd); return n;
}

int vfs_writefile(const char*path,const void*buf,unsigned int n){
    int fd=vfs_open(path,O_CREATE|O_TRUNC|O_WRONLY); if(fd<0) return -1;
    int w=vfs_write(fd,buf,n); vfs_close(fd); return w;
}