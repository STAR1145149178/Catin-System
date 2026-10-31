#ifndef _CATIN_ZZC_H
#define _CATIN_ZZC_H
#include "catin_types.h"
typedef struct {
    char name[128];
    char logo[256];
    char activity[256];
    char orientation[16];
    char vs[32];
    char sign[128];
    int  is_system;
} catin_zzc_manifest_t;
typedef struct {
    char appid[64];
    char name[128];
    char logo[256];
    char orientation[16];
    char vs[32];
    char sign[128];
    char entry[256];
    int  is_system;
} catin_app_t;
int catin_zzc_install(const char *zzc_path, char *out_appid, int out_size);
int catin_zzc_uninstall(const char *appid);
int catin_zzc_launch(const char *appid);
int catin_zzc_list(catin_app_t *out, int max);
int catin_zzc_find(const char *appid, catin_app_t *out);
#endif
