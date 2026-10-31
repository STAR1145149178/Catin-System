#include "catin.h"

#define MAXE 128
static char g_cwd[512] = "/";
static catin_dirent_t g_ent[MAXE];
static int g_n = 0;
static int g_sel = -1;

static catin_button_t btn_up, btn_newf, btn_newd, btn_del, btn_zip, btn_unzip;
static catin_textbox_t tb_name;
static char g_msg[128];

static void refresh(void){ g_n = catin_fs_list(g_cwd, g_ent, MAXE); g_sel=-1; }
static void join(char *o, int cap, const char *n){
    if(!catin_strcmp(g_cwd,"/")) catin_snprintf(o,cap,"/%s",n);
    else catin_snprintf(o,cap,"%s/%s",g_cwd,n);
}
static void do_zip(const char *path){
    char out[1024];
    catin_snprintf(out, sizeof(out), "%s.zip", path);
    if(catin_zip_create(out, &path, 1) == 0)
        catin_snprintf(g_msg, sizeof(g_msg), "已压缩: %s", out);
    else catin_snprintf(g_msg, sizeof(g_msg), "压缩失败");
}
static void do_unzip(const char *path){
    char out[1024];
    catin_snprintf(out, sizeof(out), "%s.d", path);
    catin_fs_mkdir(out);
    if(catin_zip_extract(path, out) == 0)
        catin_snprintf(g_msg, sizeof(g_msg), "已解压到: %s", out);
    else catin_snprintf(g_msg, sizeof(g_msg), "解压失败");
}
static void home_enter(void){
    refresh();
    int W = catin_canvas_get()->width;
    btn_up = (catin_button_t){ .r=RECT(20,20,100,48), .text="上级", .bg=C_DARK,.fg=C_WHITE,.border=C_GRAY };
    btn_newf = (catin_button_t){ .r=RECT(20,900,170,60), .text="新建文件", .bg=C_BLUE,.fg=C_WHITE,.border=C_WHITE };
    btn_newd = (catin_button_t){ .r=RECT(210,900,170,60), .text="新建目录", .bg=C_BLUE,.fg=C_WHITE,.border=C_WHITE };
    btn_del = (catin_button_t){ .r=RECT(400,900,150,60), .text="删除", .bg=C_RED,.fg=C_WHITE,.border=C_WHITE };
    btn_zip = (catin_button_t){ .r=RECT(20,980,170,60), .text="压缩", .bg=C_DARK,.fg=C_WHITE,.border=C_GRAY };
    btn_unzip = (catin_button_t){ .r=RECT(210,980,170,60), .text="解压", .bg=C_DARK,.fg=C_WHITE,.border=C_GRAY };
    tb_name = (catin_textbox_t){ .r=RECT(400,980,W-420,60), .max_len=200 };
    catin_strcpy(tb_name.text, "newfile.txt");
    tb_name.len = catin_strlen(tb_name.text);
}
static void home_update(catin_canvas_t *cv, catin_event_t *e){
    cv->clear(cv, C_BG);
    cv->text(cv, 140, 30, g_cwd, C_WHITE);
    catin_button_draw(cv,&btn_up); catin_button_event(&btn_up,e);
    if(btn_up.clicked){
        btn_up.clicked=0;
        char *sl = catin_strrchr(g_cwd, '/');
        if(sl && sl != g_cwd) *sl = 0;
        else catin_strcpy(g_cwd, "/");
        refresh();
    }
    int y = 90;
    for(int i=0;i<g_n && y<880;i++){
        int hover = (e->type==CEV_PEN_MOVE && e->x>=20 && e->x<cv->width-20 && e->y>=y && e->y<y+56);
        u32 bg = (i==g_sel)?C_BLUE:(hover?C_GRAY:C_DARK);
        cv->rect_fill(cv, 20, y, cv->width-40, 56, bg);
        char line[300];
        catin_snprintf(line, sizeof(line), "%s %s", g_ent[i].is_dir?"[D]":"   ", g_ent[i].name);
        cv->text(cv, 36, y+18, line, C_WHITE);
        if(e->type==CEV_PEN_DOWN && e->x>=20 && e->x<cv->width-20 && e->y>=y && e->y<y+56) g_sel = i;
        y += 62;
    }
    if(e->type==CEV_PEN_DOWN && e->y>=90 && e->y<880){
        int idx = (e->y - 90) / 62;
        if(idx>=0 && idx<g_n && g_ent[idx].is_dir){
            char np[1024]; join(np, sizeof(np), g_ent[idx].name);
            catin_strncpy(g_cwd, np, sizeof(g_cwd)-1);
            refresh();
        }
    }
    catin_button_draw(cv,&btn_newf); catin_button_event(&btn_newf,e);
    if(btn_newf.clicked){ btn_newf.clicked=0; char full[1024]; join(full,sizeof(full),tb_name.text);
        int fd = catin_fs_open(full, CATIN_O_WRONLY|CATIN_O_CREATE|CATIN_O_TRUNC);
        if(fd>=0) catin_fs_close(fd); refresh(); }
    catin_button_draw(cv,&btn_newd); catin_button_event(&btn_newd,e);
    if(btn_newd.clicked){ btn_newd.clicked=0; char full[1024]; join(full,sizeof(full),tb_name.text);
        catin_fs_mkdir(full); refresh(); }
    catin_button_draw(cv,&btn_del); catin_button_event(&btn_del,e);
    if(btn_del.clicked && g_sel>=0){ btn_del.clicked=0;
        char full[1024]; join(full,sizeof(full),g_ent[g_sel].name);
        catin_fs_rm(full); refresh(); }
    catin_button_draw(cv,&btn_zip); catin_button_event(&btn_zip,e);
    if(btn_zip.clicked && g_sel>=0){ btn_zip.clicked=0;
        char full[1024]; join(full,sizeof(full),g_ent[g_sel].name);
        do_zip(full); refresh(); }
    catin_button_draw(cv,&btn_unzip); catin_button_event(&btn_unzip,e);
    if(btn_unzip.clicked && g_sel>=0){ btn_unzip.clicked=0;
        char full[1024]; join(full,sizeof(full),g_ent[g_sel].name);
        do_unzip(full); refresh(); }
    catin_textbox_draw(cv,&tb_name); catin_textbox_event(&tb_name,e);
    if(g_msg[0]){
        cv->rect_fill(cv, 20, 1060, cv->width-40, 40, C_DARK);
        cv->text(cv, 30, 1072, g_msg, C_GREEN);
    }
    cv->present(cv);
}

CATIN_APP("FileManager") {
    catin_screen_t s = { .name="home", .enter=home_enter, .update=home_update };
    catin_screen_register("home", s);
    catin_screen_goto(0);
    catin_canvas_t *cv = catin_canvas_get();
    for(;;){
        catin_event_t e = {0};
        catin_platform_poll(&e);
        if(e.type == CEV_QUIT) break;
        catin_screen_dispatch(cv, &e);
    }
    return 0;
}
