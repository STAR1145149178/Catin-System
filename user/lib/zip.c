#include "catin.h"
/* 极简 ZIP: 只支持 stored (method=0), 无压缩, 但可解压和创建 */
/* 完整 deflate 需要 600+ 行, 可后续替换为 miniz */

typedef struct {
    u8  *buf;
    u32  size, pos;
} zbuf_t;

static u16 rd16(const u8*p){ return ((u16)p[1]<<8)|p[0]; }
static u32 rd32(const u8*p){ return ((u32)p[3]<<24)|((u32)p[2]<<16)|((u32)p[1]<<8)|p[0]; }
static void wr16(u8*p,u16 v){ p[0]=v&0xff; p[1]=(v>>8)&0xff; }
static void wr32(u8*p,u32 v){ p[0]=v&0xff; p[1]=(v>>8)&0xff; p[2]=(v>>16)&0xff; p[3]=(v>>24)&0xff; }

int catin_zip_extract(const char *zip_path, const char *dir){
    u32 zsz = 0;
    u8 *z = (u8*)catin_fs_readfile(zip_path, &zsz);
    if(!z) return -1;

    /* 找 End of Central Directory: 0x06054b50 */
    int eocd = -1;
    for(int i=(int)zsz-22; i>=0 && i>= (int)zsz-65557; i--){
        if(z[i]==0x50 && z[i+1]==0x4b && z[i+2]==0x05 && z[i+3]==0x06){ eocd=i; break; }
    }
    if(eocd < 0){ catin_free(z); return -1; }

    u16 total = rd16(z+eocd+10);
    u32 cdoff = rd32(z+eocd+16);

    u32 p = cdoff;
    for(u16 i=0; i<total; i++){
        if(p+46 > zsz) break;
        if(!(z[p]==0x50 && z[p+1]==0x4b && z[p+2]==0x01 && z[p+3]==0x02)) break;
        u16 method = rd16(z+p+10);
        u32 csize = rd32(z+p+20);
        u16 nlen = rd16(z+p+28);
        u16 elen = rd16(z+p+30);
        u16 clen = rd16(z+p+32);
        u32 lho = rd32(z+p+42);

        char fname[256];
        int fn = nlen > 255 ? 255 : nlen;
        catin_memcpy(fname, z+p+46, fn);
        fname[fn] = 0;

        if(method != 0){ p += 46+nlen+elen+clen; continue; }

        u16 lf_nlen = rd16(z+lho+26);
        u16 lf_elen = rd16(z+lho+28);
        u32 dataoff = lho + 30 + lf_nlen + lf_elen;
        if(dataoff + csize > zsz) break;

        char full[512];
        catin_snprintf(full, sizeof(full), "%s/%s", dir, fname);
        if(csize == 0){
            /* 目录项 */
            catin_fs_mkdir(full);
        } else {
            /* 确保父目录存在 */
            char parent[512];
            catin_strncpy(parent, full, sizeof(parent)-1);
            char *sl = catin_strrchr(parent, '/');
            if(sl){ *sl = 0; catin_fs_mkdir(parent); }
            catin_fs_writefile(full, z+dataoff, csize);
        }
        p += 46+nlen+elen+clen;
    }
    catin_free(z);
    return 0;
}

int catin_zip_create(const char *zip_path, const char *const *files, int n){
    /* 计算总大小 */
    u32 total = 22;   /* EOCD */
    u32 *fsz = catin_malloc(sizeof(u32) * n);
    for(int i=0;i<n;i++){
        u32 s = 0;
        char *d = catin_fs_readfile(files[i], &s);
        if(d) catin_free(d);
        fsz[i] = s;
        const char *fn = catin_strrchr(files[i], '/');
        fn = fn ? fn+1 : files[i];
        total += 30 + catin_strlen(fn) + s + 46 + catin_strlen(fn);
    }
    u8 *z = catin_calloc(1, total);
    u32 p = 0;
    u32 *lo = catin_malloc(sizeof(u32) * n);
    for(int i=0;i<n;i++){
        lo[i] = p;
        const char *fn = catin_strrchr(files[i], '/');
        fn = fn ? fn+1 : files[i];
        u16 nl = catin_strlen(fn);
        wr32(z+p, 0x04034b50); p+=4;
        wr16(z+p, 20); p+=2;
        wr16(z+p, 0); p+=2;
        wr16(z+p, 0); p+=2;
        wr16(z+p, 0); p+=2;
        wr16(z+p, 0); p+=2;
        wr32(z+p, 0); p+=4;
        wr32(z+p, fsz[i]); p+=4;
        wr32(z+p, fsz[i]); p+=4;
        wr16(z+p, nl); p+=2;
        wr16(z+p, 0); p+=2;
        catin_memcpy(z+p, fn, nl); p += nl;
        u32 s = 0;
        char *d = catin_fs_readfile(files[i], &s);
        if(d){ catin_memcpy(z+p, d, s); p += s; catin_free(d); }
    }
    u32 cd_off = p;
    for(int i=0;i<n;i++){
        const char *fn = catin_strrchr(files[i], '/');
        fn = fn ? fn+1 : files[i];
        u16 nl = catin_strlen(fn);
        wr32(z+p, 0x02014b50); p+=4;
        wr16(z+p, 20); p+=2;
        wr16(z+p, 20); p+=2;
        wr16(z+p, 0); p+=2;
        wr16(z+p, 0); p+=2;
        wr16(z+p, 0); p+=2;
        wr16(z+p, 0); p+=2;
        wr32(z+p, 0); p+=4;
        wr32(z+p, fsz[i]); p+=4;
        wr32(z+p, fsz[i]); p+=4;
        wr16(z+p, nl); p+=2;
        wr16(z+p, 0); p+=2;
        wr16(z+p, 0); p+=2;
        wr16(z+p, 0); p+=2;
        wr16(z+p, 0); p+=2;
        wr32(z+p, 0); p+=4;
        wr32(z+p, lo[i]); p+=4;
        catin_memcpy(z+p, fn, nl); p += nl;
    }
    wr32(z+p, 0x06054b50); p+=4;
    wr16(z+p, 0); p+=2;
    wr16(z+p, 0); p+=2;
    wr16(z+p, n); p+=2;
    wr16(z+p, n); p+=2;
    wr32(z+p, p - 22 - cd_off + cd_off); p+=4;
    wr32(z+p, cd_off); p+=4;
    wr16(z+p, 0); p+=2;

    int w = catin_fs_writefile(zip_path, z, p);
    catin_free(z);
    catin_free(lo);
    catin_free(fsz);
    catin_free(lo);
    return w > 0 ? 0 : -1;
}
