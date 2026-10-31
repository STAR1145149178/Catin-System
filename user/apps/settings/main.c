#include "catin.h"

#define PAGE_MAIN 0
#define PAGE_DEV  1

static catin_button_t btn_back;
static catin_switch_t sw_dev, sw_usb, sw_mock, sw_bl;
static int dev_mode_on = 0;
static int is_dev_unlocked = 0;
static int current_page = PAGE_MAIN;

// 开发者模式解锁状态
static int click_count = 0;
static u64 press_start_time = 0;
static char g_toast[128] = {0};
static u64 toast_time = 0;
static catin_rect_t version_rect = {40, 200, 400, 40};

// ==== BL锁隐藏解锁状态 ====
static int bl_unlocked = 0;
static int bl_phase = 0; // 0: 无操作, 1: 长按成功, 2: 滑动成功
static u64 bl_press_time = 0;
static int is_sliding = 0;
static int slide_start_x = 0;

static void save_dev_state(void) {
    catin_fs_writefile("/Catin System/System/dev_mode.cfg", dev_mode_on ? "1" : "0", 1);
    catin_fs_writefile("/Catin System/System/dev_unlocked.cfg", is_dev_unlocked ? "1" : "0", 1);
    catin_fs_writefile("/Catin System/System/bl_unlocked.cfg", bl_unlocked ? "1" : "0", 1);
}

// 用户态读取文件判断
static int check_flag_file(const char *path) {
    u32 sz = 0;
    char *data = catin_fs_readfile(path, &sz);
    if (data && sz > 0 && data[0] == '1') {
        catin_free(data);
        return 1;
    }
    if (data) catin_free(data);
    return 0;
}

static void on_dev_mode_change(void *ud, int on) {
    (void)ud;
    dev_mode_on = on;
    save_dev_state();
    catin_snprintf(g_toast, sizeof(g_toast), on ? "开发者模式已开启 (仅提供调试)" : "开发者模式已关闭");
    toast_time = catin_uptime();
}

static void on_bl_change(void *ud, int on) {
    (void)ud;
    if (bl_unlocked && bl_phase == 2) {
        catin_snprintf(g_toast, sizeof(g_toast), on ? "BL 锁已成功解开！" : "BL 锁已重新上锁");
        toast_time = catin_uptime();
    } else if (on) {
        sw_bl.on = 0;
        sw_bl.anim = 0; 
    }
}

static void home_enter(void){
    int W = catin_canvas_get()->width;
    
    dev_mode_on = check_flag_file("/Catin System/System/dev_mode.cfg");
    is_dev_unlocked = check_flag_file("/Catin System/System/dev_unlocked.cfg");
    bl_unlocked = check_flag_file("/Catin System/System/bl_unlocked.cfg");
    
    btn_back = (catin_button_t){ .r=RECT(W-150,20,130,64), .text="返回", .bg=C_RED,.fg=C_WHITE,.border=C_WHITE };
    
    sw_dev = (catin_switch_t){
        .r = RECT(20, 200, W-40, 80), .title = "开发者模式", .subtitle = "允许修改系统核心文件",
        .on = dev_mode_on, .anim = dev_mode_on ? 100 : 0, .on_change = on_dev_mode_change
    };
    sw_usb = (catin_switch_t){ .r = RECT(20, 300, W-40, 80), .title = "USB 调试 (模拟)", .subtitle = "允许通过电脑调试", .on = 0, .anim = 0 };
    sw_mock = (catin_switch_t){ .r = RECT(20, 400, W-40, 80), .title = "模拟位置信息", .subtitle = "提供虚构 GPS", .on = 0, .anim = 0 };
    
    sw_bl = (catin_switch_t){
        .r = RECT(20, 550, W-40, 80), .title = "Bootloader 锁", .subtitle = "默认锁定，需特殊手段解锁",
        .on = bl_unlocked ? 1 : 0, .anim = bl_unlocked ? 100 : 0, .on_change = on_bl_change
    };
}

static void handle_events(catin_event_t *e) {
    catin_button_event(&btn_back, e);
    if(btn_back.clicked){ btn_back.clicked=0; current_page = (current_page == PAGE_DEV) ? PAGE_MAIN : PAGE_DEV; }

    if (current_page == PAGE_MAIN) {
        if (e->x >= version_rect.x && e->x <= version_rect.x + version_rect.w && e->y >= version_rect.y && e->y <= version_rect.y + version_rect.h) {
            if (e->type == CEV_PEN_DOWN) press_start_time = catin_uptime();
            else if (e->type == CEV_PEN_UP) {
                u64 dur = catin_uptime() - press_start_time;
                if (!is_dev_unlocked && dur < 500) {
                    click_count++;
                    if (click_count >= 7) { catin_snprintf(g_toast, sizeof(g_toast), "请长按 1 秒解锁开发者选项"); toast_time = catin_uptime(); }
                } else if (click_count >= 7 && !is_dev_unlocked && dur >= 1000) {
                    is_dev_unlocked = 1; save_dev_state();
                    catin_snprintf(g_toast, sizeof(g_toast), "开发者选项已解锁！"); toast_time = catin_uptime();
                }
            }
        }
        if (is_dev_unlocked && e->type == CEV_PEN_UP) {
            if (e->x >= version_rect.x && e->x <= version_rect.x + version_rect.w && e->y >= version_rect.y + 40 && e->y <= version_rect.y + 80) {
                current_page = PAGE_DEV;
            }
        }
    } 
    else if (current_page == PAGE_DEV) {
        catin_switch_event(&sw_dev, e);
        catin_switch_event(&sw_usb, e);
        catin_switch_event(&sw_mock, e);
        catin_switch_event(&sw_bl, e);

        int is_on_bl_text = (e->x >= 20 && e->x <= 250 && e->y >= sw_bl.r.y && e->y <= sw_bl.r.y + sw_bl.r.h);

        if (is_on_bl_text) {
            if (e->type == CEV_PEN_DOWN) {
                bl_press_time = catin_uptime();
                is_sliding = 0; 
            } 
            else if (e->type == CEV_PEN_UP) {
                u64 dur = catin_uptime() - bl_press_time;
                if (dur >= 1000 && bl_phase == 0) bl_phase = 1;
            }
            else if (e->type == CEV_PEN_MOVE) {
                if (bl_phase == 1) {
                    if (!is_sliding) {
                        is_sliding = 1;
                        slide_start_x = e->x;
                    } else {
                        if (slide_start_x - e->x > 150) {
                            bl_phase = 2; bl_unlocked = 1; save_dev_state();
                            is_sliding = 0;
                        }
                    }
                }
            }
        }
    }
}

static void home_update(catin_canvas_t *cv, catin_event_t *e){
    cv->clear(cv, C_DARK);
    handle_events(e);
    
    cv->text(cv, 40, 80, current_page == PAGE_MAIN ? "系统设置" : "开发者选项", C_WHITE);
    catin_button_draw(cv,&btn_back);

    if (current_page == PAGE_MAIN) {
        cv->text(cv, 40, 140, "Catin OS 2.2.0", C_GRAY);
        cv->text(cv, 40, 200, "内核版本: Cascade (点击此处)", C_GRAY);
        if (is_dev_unlocked) cv->text(cv, 40, 240, "> 开发者选项已解锁 (点击进入) <", C_GREEN);
    } 
    else if (current_page == PAGE_DEV) {
        catin_switch_draw(cv, &sw_dev, e);
        catin_switch_draw(cv, &sw_usb, e);
        catin_switch_draw(cv, &sw_mock, e);
        
        if (!bl_unlocked || bl_phase < 2) {
            sw_bl.on = 0;
            sw_bl.anim = 0;
        }
        catin_switch_draw(cv, &sw_bl, e);
    }
    
    if (g_toast[0] && (catin_uptime() - toast_time < 2000)) {
        cv->rect_fill(cv, 20, cv->height - 160, cv->width - 40, 60, C_BLUE);
        cv->text(cv, 40, cv->height - 145, g_toast, C_WHITE);
    }

    cv->present(cv);
}

CATIN_APP("Settings") {
    catin_screen_t s = { .name="home", .enter=home_enter, .update=home_update };
    catin_screen_register("home", s); catin_screen_goto(0);
    catin_canvas_t *cv = catin_canvas_get();
    for(;;){ 
        catin_event_t e = {0}; 
        catin_platform_poll(&e); 
        if(e.type == CEV_QUIT) break; 
        catin_screen_dispatch(cv, &e); 
    }
    return 0;
}