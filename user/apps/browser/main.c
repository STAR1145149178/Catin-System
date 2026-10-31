#include "catin.h"

// 定义当前状态
static int is_home_page = 1; // 1: 百度首页, 0: 网页内容页
static catin_textbox_t tb_search;
static catin_button_t btn_baidu, btn_back, btn_dl;
static catin_keyboard_t kb;
static char g_content[65536];
static char g_url[1024];
static int g_is_zzc = 0;
static char g_status[256];

// 真正的网络请求逻辑
static void load_url(const char *url){
    catin_strncpy(g_url, url, sizeof(g_url)-1);
    int len = catin_strlen(url);
    is_home_page = 0;
    
    // 如果是安装包
    if(len>4 && !catin_strcmp(url+len-4, ".zzc")){
        g_is_zzc = 1;
        catin_snprintf(g_content, sizeof(g_content),
            "检测到应用安装包\n\n%s\n\n点击下方 [下载并安装] 开始本地安装。", url);
        return;
    }
    
    g_is_zzc = 0;
    static char buf[65536];
    u32 n = 0;
    
    // 请求网络（如果没有底层网卡和协议栈，会返回失败）
    catin_snprintf(g_content, sizeof(g_content), "正在连接服务器解析...\n\n%s", url);
    if(catin_http_get(url, buf, sizeof(buf), &n) != 0){
        catin_strcpy(g_content, 
            "【真实网络连接失败】\n\n"
            "原因：\n"
            "1. RJ45 网卡尚未插入。\n"
            "2. 内核的 TCP/IP 协议栈尚未启动。\n\n"
            "如果你只是想测试应用安装，请输入：\n"
            "http://local/settings.zzc");
        return;
    }
    
    // 如果成功拿到了数据，把 HTML 标签简单剥离一下
    int j=0, in_tag=0;
    for(int i=0; buf[i] && j<sizeof(g_content)-1; i++){
        if(buf[i]=='<'){ in_tag=1; continue; }
        if(buf[i]=='>'){ in_tag=0; continue; }
        if(in_tag) continue;
        g_content[j++] = buf[i];
    }
    g_content[j]=0;
}

static void download_and_install(void){
    const char *slash = catin_strrchr(g_url, '/');
    const char *fname = slash ? slash+1 : "settings.zzc";
    char path[512];
    catin_snprintf(path, sizeof(path), "/Catin System/rootfs/apps/%s", fname);
    catin_snprintf(g_status, sizeof(g_status), "正在安装 %s...", fname);
    
    char appid[64];
    int rc = catin_zzc_install(path, appid, sizeof(appid));
    if(rc == 0){
        catin_snprintf(g_status, sizeof(g_status), "安装成功! ID: %s", appid);
        g_is_zzc = 0;
    } else {
        catin_snprintf(g_status, sizeof(g_status), "安装失败 (rc=%d)", rc);
    }
}

// 初始化：直接进入百度页面
static void home_enter(void){
    int W = catin_canvas_get()->width;
    
    tb_search = (catin_textbox_t){ .r=RECT(40, 250, W-80, 64), .max_len=1000, .focused=0 };
    catin_strcpy(tb_search.text, ""); 
    tb_search.len = 0;
    
    // 百度一下按钮
    btn_baidu = (catin_button_t){ .r=RECT(W/2-100, 340, 200, 64), .text="百度一下", .bg=C_BLUE,.fg=C_WHITE,.border=C_BLUE };
    // 退出按钮
    btn_back = (catin_button_t){ .r=RECT(W-150,20,130,64), .text="退出", .bg=C_RED,.fg=C_WHITE,.border=C_WHITE };
    
    catin_keyboard_init(&kb);
    g_status[0] = 0;
    is_home_page = 1; // 核心：初始就在首页
}

static void home_update(catin_canvas_t *cv, catin_event_t *e){
    cv->clear(cv, C_BG); // 清空背景

    if (is_home_page) {
        // ================= 百度首页渲染 =================
        
        // 画百度的假 Logo (用文字模拟)
        cv->text_center(cv, cv->width/2, 120, cv->width, "Baidu", C_BLUE);
        cv->text_center(cv, cv->width/2, 170, cv->width, "Catin Browser", C_GRAY);
        
        // 画搜索框
        catin_textbox_draw(cv,&tb_search); 
        catin_textbox_event(&tb_search,e);
        
        // 点击搜索框，弹出键盘
        if (e->type == CEV_PEN_DOWN && catin_rect_contains(tb_search.r, e->x, e->y)) {
            kb.visible = 1;
            tb_search.focused = 1;
        }

        // 画百度一下按钮
        catin_button_draw(cv,&btn_baidu); catin_button_event(&btn_baidu,e);
        if(btn_baidu.clicked){ 
            btn_baidu.clicked=0; 
            if (tb_search.len > 0) {
                char url[512];
                catin_snprintf(url, sizeof(url), "https://www.baidu.com/s?wd=%s", tb_search.text);
                load_url(url); 
                kb.visible = 0; 
            }
        }
        
        // 画退出按钮
        catin_button_draw(cv,&btn_back); catin_button_event(&btn_back,e);
        if(btn_back.clicked){ btn_back.clicked=0; catin_exit(0); }

    } else {
        // ================= 网页/结果展示页 =================
        
        // 顶部返回按钮
        btn_back = (catin_button_t){ .r=RECT(20,20,150,64), .text="< 返回首页", .bg=C_GRAY,.fg=C_WHITE,.border=C_WHITE };
        catin_button_draw(cv,&btn_back); catin_button_event(&btn_back,e);
        if(btn_back.clicked){ btn_back.clicked=0; is_home_page=1; }

        // 内容展示区
        int y = 100;
        cv->rect_fill(cv, 20, y, cv->width-40, cv->height-y-100, C_BLACK);
        cv->text(cv, 40, y+20, g_content, C_WHITE);
        
        // 如果触发了 .zzc 安装
        if(g_is_zzc){
            btn_dl = (catin_button_t){ .r=RECT(20, cv->height-80, cv->width-40, 60), .text="下载并安装", .bg=C_GREEN,.fg=C_BLACK,.border=C_WHITE };
            catin_button_draw(cv,&btn_dl); catin_button_event(&btn_dl,e);
            if(btn_dl.clicked){ btn_dl.clicked=0; download_and_install(); }
        }
    }

    // 底部状态栏提示
    if(g_status[0]){
        cv->rect_fill(cv, 20, cv->height-60, cv->width-40, 40, C_DARK);
        cv->text(cv, 30, cv->height-48, g_status, C_GREEN);
    }

    // 绘制键盘
    if (kb.visible) {
        int hit_enter = catin_keyboard_draw_and_event(cv, &kb, e, tb_search.text, &tb_search.len, tb_search.max_len);
        if (hit_enter) {
            if (tb_search.len > 0) {
                char url[512];
                catin_snprintf(url, sizeof(url), "https://www.baidu.com/s?wd=%s", tb_search.text);
                load_url(url);
                kb.visible = 0;
            }
        }
    }

    cv->present(cv);
}

CATIN_APP("Browser") {
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