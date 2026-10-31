#include "catin.h"
#include "xcui.h"
#define MAXL 22
#define MAXW 52
static char lines[MAXL][MAXW];
static int n_lines;
static char cur[MAXW];
static int n_cur;
static void push(const char*s){
    if(n_lines>=MAXL){ for(int i=0;i<MAXL-1;i++) kmemcpy(lines[i],lines[i+1],MAXW); n_lines=MAXL-1; }
    kstrncpy(lines[n_lines],s,MAXW); n_lines++;
}
static void run(char*c){
    while(*c==' ') c++;
    if(!*c) return;
    char*a=c; while(*a&&*a!=' ') a++;
    if(*a){ *a++=0; while(*a==' ') a++; }
    if(!kstrcmp(c,"help")) push("help ps ls cat ver mem uptime");
    else if(!kstrcmp(c,"ps")) proc_dump();
    else if(!kstrcmp(c,"ls")){
        char buf[512]; int n=vfs_readdir("/",buf,sizeof(buf));
        if(n>0){ char*p=buf; while(*p){ char*nl=p; while(*nl&&*nl!='\n')nl++;
            char save=*nl; *nl=0; push(p); *nl=save; p=(*nl)?nl+1:nl; } }
        else push("(empty)");
    }
    else if(!kstrcmp(c,"cat")){
        if(!*a){ push("usage: cat <file>"); return; }
        char path[64]="/"; kstrncpy(path+1,a,60);
        static char buf[256];
        int n=vfs_readfile(path,buf,sizeof(buf)-1);
        if(n<=0){ push("no file"); return; }
        buf[n]=0; push(buf);
    }
    else if(!kstrcmp(c,"ver")) push(CATIN_NAME " " CATIN_VERSION " (" ARCH_NAME ")");
    else if(!kstrcmp(c,"mem")){ char b[64];
        ksnprintf(b,sizeof(b),"used %lu KB free %lu KB",
            (unsigned long)(mm_used()>>10),(unsigned long)(mm_free()>>10)); push(b); }
    else if(!kstrcmp(c,"uptime")){ char b[64];
        ksnprintf(b,sizeof(b),"ticks = %llu",(unsigned long long)arch_timer_ticks()); push(b); }
    else push("unknown");
}
void shell_init(void){ n_lines=0; n_cur=0; push("Catin Console 2.2"); push("type 'help'"); }
void shell_tick(void){
    while(arch_uart_rx_ready()){
        char c=(char)arch_uart_getc();
        if(c=='\r'||c=='\n'){ cur[n_cur]=0; run(cur); n_cur=0; }
        else if(c==127||c==8){ if(n_cur>0) n_cur--; }
        else if(c>=32&&c<127&&n_cur<MAXW-1) cur[n_cur++]=c;
    }
}
void shell_frame(void){
    int W=fb_w(),H=fb_h();
    xc_fill(XC_RECT(0,0,W,H),(xc_color_t){8,10,16,255});
    xc_fill(XC_RECT(0,0,W,80),(xc_color_t){20,26,38,255});
    xc_text(24,26,"$ catin-console",xc_theme.text,2);
    xc_fill(XC_RECT(0,80,W,1),xc_theme.divider);
    char ab[48]; ksnprintf(ab,sizeof(ab),ARCH_NAME " / " CATIN_VERSION);
    xc_text(W-260,30,ab,xc_theme.text_dim,1);
    int y=110;
    for(int i=0;i<n_lines;i++){
        xc_text(20,y,lines[i],(i==0)?xc_theme.text_dim:xc_theme.text,1);
        y+=22; if(y>H-200) break;
    }
    xc_text(20,H-160,"$",xc_theme.success,2);
    xc_text(48,H-160,cur,xc_theme.text,2);
    if(((arch_timer_ticks()/400)&1)==0){
        int cx=48+xc_text_w(cur,2);
        xc_fill(XC_RECT(cx,H-160,12,24),xc_theme.primary);
    }
    xc_text(W/2-120,H-100,"串口输入",xc_theme.text_dim,1);
    xc_text(W/2-90,H-60,"顶部下拉返回桌面",xc_theme.text_dim,1);
}
