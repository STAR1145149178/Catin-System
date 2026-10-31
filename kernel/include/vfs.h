#ifndef _VFS_H
#define _VFS_H
#include "types.h"
#define PATH_MAX 128
#define NAME_MAX 32
#define FD_MAX 32
#define INODE_MAX 128
#define DIR_CHILDREN 24
#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREATE 0x100
#define O_TRUNC 0x200
typedef struct inode {
    int used, is_dir;
    char name[NAME_MAX];
    unsigned int size;
    void *data;
    int parent;
    int children[DIR_CHILDREN];
    int n_children;
    int ref;
} inode_t;
typedef struct file { int used, inode_idx; unsigned int pos; int flags; } file_t;
void vfs_init(void);
int vfs_open(const char *path, int flags);
int vfs_close(int fd);
int vfs_read(int fd, void *buf, unsigned int n);
int vfs_write(int fd, const void *buf, unsigned int n);
int vfs_lseek(int fd, int off, int whence);
int vfs_mkdir(const char *path);
int vfs_unlink(const char *path);
int vfs_readdir(const char *path, char *out, unsigned int cap);
int vfs_stat(const char *path, unsigned int *size, int *is_dir);
int vfs_readfile(const char *path, void *buf, unsigned int cap);
int vfs_writefile(const char *path, const void *buf, unsigned int n);
#endif
