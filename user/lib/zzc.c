#include "catin.h"

static int read_conf(const char *conf, catin_zzc_manifest_t *m){
    u32 sz = 0;
    char *c = catin_fs_readfile(conf, &sz);
    if(!c) return -1;
    if(catin_strncmp(c, "Catin OS", 8) != 0){ catin_free(c); return -1; }
    catin_memset(m, 0, sizeof(*m));
    catin_strcpy(m->orientation, "portrait");

    char *p = c;
    int first = 1;
    while(*p){
        char *nl = catin_strchr(p, '\n');
        if(nl) *nl = 0;
        char *line = p;
        while(*line == ' ' || *line == '\t') line++;
        if(*line && *line != '#' && !(line[0]=='/' && line[1]=='/')){
            if(first){ first = 0; }
            else {
                char *eq = catin_strchr(line, '=');
                if(eq){
                    *eq = 0;
                    char *k = line, *v = eq+1;
                    while(*k && (k[catin_strlen(k)-1] == ' ')) k[catin_strlen(k)-1] = 0;
                    while(*v == ' ') v++;
                    if      (!catin_strcmp(k,"name")) catin_strncpy(m->name, v, 127);
                    else if (!catin_strcmp(k,"logo")) catin_strncpy(m->logo, v, 255);
                    else if (!catin_strcmp(k,"Activity")) catin_strncpy(m->activity, v, 255);
                    else if (!catin_strcmp(k,"entry")) catin_strncpy(m->activity, v, 255);
                    else if (!catin_strcmp(k,"orientation")) catin_strncpy(m->orientation, v, 15);
                    else if (!catin_strcmp(k,"vs")) catin_strncpy(m->vs, v, 31);
                    else if (!catin_strcmp(k,"version")) catin_strncpy(m->vs, v, 31);
                    else if (!catin_strcmp(k,"zzc.sign")) catin_strncpy(m->sign, v, 127);
                    else if (!catin_strcmp(k,"system")) m->is_system = catin_atoi(v);
                }
            }
        }
        if(!nl) break;
        p = nl+1;
    }
    catin_free(c);
    if(!m->name[0] || !m->logo[0] || !m->activity[0]) return -1;
    if(!m->vs[0]) catin_strcpy(m->vs, "null");
    return 0;
}

static void make_appid(const char *name, char *out, int size){
    u32 h = 5381;
    for(const char *p = name; *p; p++) h = ((h<<5)+h) + (u8)*p;
    catin_snprintf(out, size, "%s_%08x", name, h);
    for(char *p=out; *p; p++)
        if(!((*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z')||(*p>='0'&&*p<='9')||*p=='_'||*p=='-')) *p='_';
}

int catin_zzc_install(const char *zzc_path, char *out_appid, int out_size){
    char tmp[256];
    catin_snprintf(tmp, sizeof(tmp), "/tmp/install_%d", catin_getpid());
    catin_fs_mkdir("/tmp");
    catin_fs_mkdir(tmp);

    if(catin_zip_extract(zzc_path, tmp) != 0) return -1;

    char conf[512];
    catin_snprintf(conf, sizeof(conf), "%s/app.conf", tmp);

    catin_zzc_manifest_t m;
    if(read_conf(conf, &m) != 0){
        catin_fs_rm(tmp);
        return -1;
    }

    make_appid(m.name, out_appid, out_size);

    char app_dir[512];
    catin_snprintf(app_dir, sizeof(app_dir), "/apps/%s", out_appid);
    catin_fs_mkdir("/apps");

    /* 只做简单移动: 拷贝每个文件, 然后删 tmp */
    /* 简化: 用 readdir 遍历, 逐个 copy */
    catin_dirent_t ents[128];
    int n = catin_fs_list(tmp, ents, 128);
    for(int i=0;i<n;i++){
        char from[1024], to[1024];
        catin_snprintf(from, sizeof(from), "%s/%s", tmp, ents[i].name);
        catin_snprintf(to, sizeof(to), "%s/%s", app_dir, ents[i].name);
        if(ents[i].is_dir){
            catin_fs_mkdir(to);
            /* 简单处理: 一层子目录 */
            catin_dirent_t sub[64];
            int sn = catin_fs_list(from, sub, 64);
            for(int j=0;j<sn;j++){
                char f2[1024], t2[1024];
                catin_snprintf(f2, sizeof(f2), "%s/%s", from, sub[j].name);
                catin_snprintf(t2, sizeof(t2), "%s/%s", to, sub[j].name);
                u32 ss = 0;
                char *d = catin_fs_readfile(f2, &ss);
                if(d){ catin_fs_writefile(t2, d, ss); catin_free(d); }
            }
        } else {
            u32 ss = 0;
            char *d = catin_fs_readfile(from, &ss);
            if(d){ catin_fs_writefile(to, d, ss); catin_free(d); }
        }
    }

    /* 追加注册表 */
    char ent_line[1024];
    char app_conf_final[512];
    catin_snprintf(app_conf_final, sizeof(app_conf_final), "%s/app.conf", app_dir);
    catin_snprintf(ent_line, sizeof(ent_line),
        "%s|%s|%s/%s|%s|%s|%s|%s/%s|%s/%s\n",
        out_appid, m.name, app_dir, m.logo, m.orientation, m.vs, m.sign,
        app_dir, m.activity, app_dir, m.activity);
    u32 old_sz = 0;
    char *old = catin_fs_readfile("/apps/registry.db", &old_sz);
    char *all = catin_malloc((old_sz ? old_sz : 0) + catin_strlen(ent_line) + 1);
    if(old_sz) catin_memcpy(all, old, old_sz);
    catin_memcpy(all + old_sz, ent_line, catin_strlen(ent_line));
    all[old_sz + catin_strlen(ent_line)] = 0;
    catin_fs_writefile("/apps/registry.db", all, old_sz + catin_strlen(ent_line));
    catin_free(all);
    if(old) catin_free(old);

    /* 清理 tmp */
    for(int i=0;i<n;i++){
        char f[1024];
        catin_snprintf(f, sizeof(f), "%s/%s", tmp, ents[i].name);
        catin_fs_rm(f);
    }
    catin_fs_rm(tmp);

    return 0;
}

int catin_zzc_uninstall(const char *appid){
    char dir[512];
    catin_snprintf(dir, sizeof(dir), "/apps/%s", appid);
    catin_fs_rm(dir);
    return 0;
}

int catin_zzc_launch(const char *appid){
    /* 简化: 直接 exec */
    char path[512];
    catin_snprintf(path, sizeof(path), "/apps/%s/bin/app", appid);
    if(!catin_fs_exists(path)) return -1;
    int pid = catin_fork();
    if(pid == 0){
        catin_exec(path);
        catin_exit(127);
    }
    int st;
    catin_wait(&st);
    return 0;
}

int catin_zzc_list(catin_app_t *out, int max){
    u32 sz = 0;
    char *db = catin_fs_readfile("/apps/registry.db", &sz);
    if(!db) return 0;
    int n = 0;
    char *p = db;
    while(*p && n < max){
        char *nl = catin_strchr(p, '\n');
        if(nl) *nl = 0;
        /* 格式: appid|name|logo|orientation|vs|sign|entry|entry */
        catin_memset(&out[n], 0, sizeof(out[n]));
        char *fields[8] = {0};
        int fi = 0;
        char *q = p;
        fields[fi++] = q;
        while(*q && fi < 8){
            if(*q == '|'){ *q = 0; fields[fi++] = q+1; }
            q++;
        }
        if(fi >= 7){
            catin_strncpy(out[n].appid, fields[0], 63);
            catin_strncpy(out[n].name, fields[1], 127);
            catin_strncpy(out[n].logo, fields[2], 255);
            catin_strncpy(out[n].orientation, fields[3], 15);
            catin_strncpy(out[n].vs, fields[4], 31);
            catin_strncpy(out[n].sign, fields[5], 127);
            catin_strncpy(out[n].entry, fields[6], 255);
            n++;
        }
        if(!nl) break;
        p = nl+1;
    }
    catin_free(db);
    return n;
}

int catin_zzc_find(const char *appid, catin_app_t *out){
    catin_app_t arr[64];
    int n = catin_zzc_list(arr, 64);
    for(int i=0;i<n;i++) if(!catin_strcmp(arr[i].appid, appid)){ *out = arr[i]; return 0; }
    return -1;
}
