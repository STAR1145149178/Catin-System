#ifndef _CATIN_FS_H
#define _CATIN_FS_H
#include "catin_types.h"
#define CATIN_O_RDONLY  0
#define CATIN_O_WRONLY  1
#define CATIN_O_RDWR    2
#define CATIN_O_CREATE  0x100
#define CATIN_O_TRUNC   0x200
#define CATIN_O_APPEND  0x400
typedef struct { char name[256]; int is_dir; u32 size; } catin_dirent_t;
int  catin_fs_open (const char *path, int flags);
int  catin_fs_close(int fd);
int  catin_fs_read (int fd, void *buf, u32 n);
int  catin_fs_write(int fd, const void *buf, u32 n);
int  catin_fs_seek (int fd, int off, int whence);
int  catin_fs_mkdir(const char *path);
int  catin_fs_rm   (const char *path);
int  catin_fs_stat (const char *path, u32 *size, int *is_dir);
int  catin_fs_rename(const char *oldp, const char *newp);
int  catin_fs_list (const char *path, catin_dirent_t *out, int max);
char *catin_fs_readfile (const char *path, u32 *out_size);
int   catin_fs_writefile(const char *path, const void *data, u32 size);
int   catin_fs_exists   (const char *path);
#endif
