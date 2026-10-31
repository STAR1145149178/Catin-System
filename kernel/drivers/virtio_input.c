#include "catin.h"
#include "virtio.h"
#if ARCH_ID == 1 || ARCH_ID == 2
typedef struct { u16 type; u16 code; u32 value; } vin_ev_t;
#define SYN 0
#define KEY 1
#define ABS 3
#define AX 0
#define AY 1
#define BTN_TOUCH 0x14a
#define EVQS 16
static virtio_dev_t *g_dev;
static virtq_t g_evq;
static vin_ev_t g_slots[EVQS] __attribute__((aligned(16)));
static u16 g_used_head;
static int g_down,g_rx,g_ry;
static int g_minx,g_maxx,g_miny,g_maxy;
static int g_fbw,g_fbh,g_inited;
static void read_abs(void){
    struct { u8 sel,sub,sz,rsv[5]; u8 data[128]; } c;
    kmemset(&c,0,sizeof(c));
    c.sel=1; c.sub=AX;
    virtio_read_config(g_dev,0,&c,8+20);
    g_minx=(int)(*(u32*)(c.data+0)); g_maxx=(int)(*(u32*)(c.data+4));
    kmemset(&c,0,sizeof(c));
    c.sel=1; c.sub=AY;
    virtio_read_config(g_dev,0,&c,8+20);
    g_miny=(int)(*(u32*)(c.data+0)); g_maxy=(int)(*(u32*)(c.data+4));
    if(g_maxx<=g_minx){ g_minx=0; g_maxx=32767; }
    if(g_maxy<=g_miny){ g_miny=0; g_maxy=32767; }
}
static void refill(void){
    for(u32 i=0;i<EVQS;i++){
        g_evq.desc[i].addr=(u64)(uintptr_t)&g_slots[i];
        g_evq.desc[i].len=sizeof(vin_ev_t);
        g_evq.desc[i].flags=VIRTQ_DESC_F_WRITE;
        g_evq.desc[i].next=0;
        g_evq.avail->ring[i]=(u16)i;
    }
    __asm__ volatile("dsb sy":::"memory");
    g_evq.avail->idx=EVQS;
    __asm__ volatile("dsb sy":::"memory");
}
static void emit(int dn,int up){
    if(g_fbw<=0) return;
    int x=(g_rx-g_minx)*g_fbw/(g_maxx-g_minx);
    int y=(g_ry-g_miny)*g_fbh/(g_maxy-g_miny);
    if(x<0)x=0; if(x>=g_fbw)x=g_fbw-1;
    if(y<0)y=0; if(y>=g_fbh)y=g_fbh-1;
    if(dn) touch_feed(TOUCH_DOWN,x,y);
    else if(up) touch_feed(TOUCH_UP,x,y);
    else touch_feed(TOUCH_MOVE,x,y);
}
void touch_tick(void){
    if(!g_inited||!g_dev) return;
    virtio_ack_irq(g_dev);
    while(g_used_head!=g_evq.used->idx){
        u32 id=g_evq.used->ring[g_used_head%EVQS].id;
        vin_ev_t ev=g_slots[id];
        int dn=0,up=0,mv=0;
        if(ev.type==ABS){ if(ev.code==AX){ g_rx=(int)ev.value; mv=1; }
                         if(ev.code==AY){ g_ry=(int)ev.value; mv=1; } }
        else if(ev.type==KEY){ if(ev.code==BTN_TOUCH){
            if(ev.value&&!g_down){ g_down=1; dn=1; }
            else if(!ev.value&&g_down){ g_down=0; up=1; } } }
        else if(ev.type==SYN){ if(ev.code==0&&(dn||up||mv)) emit(dn,up); }
        g_evq.desc[id].addr=(u64)(uintptr_t)&g_slots[id];
        g_evq.desc[id].len=sizeof(vin_ev_t);
        g_evq.desc[id].flags=VIRTQ_DESC_F_WRITE;
        g_evq.desc[id].next=0;
        g_evq.avail->ring[g_evq.avail->idx%EVQS]=(u16)id;
        __asm__ volatile("dsb sy":::"memory");
        g_evq.avail->idx++;
        __asm__ volatile("dsb sy":::"memory");
        g_used_head++;
    }
    virtio_kick(g_dev,0);
}
int touch_init(void){
    g_inited=0;
    g_fbw=fb_w(); g_fbh=fb_h();
    g_dev=virtio_find(VIRTIO_ID_INPUT,0);
    if(!g_dev){ log_err("touch: no dev"); return -1; }
    u32 st=0;
    st|=VIRTIO_STATUS_ACKNOWLEDGE; virtio_set_status(g_dev,st);
    st|=VIRTIO_STATUS_DRIVER; virtio_set_status(g_dev,st);
    if(virtio_setup_queue(g_dev,0,&g_evq,EVQS)!=0){ log_err("touch: q"); return -1; }
    st|=VIRTIO_STATUS_DRIVER_OK; virtio_set_status(g_dev,st);
    read_abs(); refill(); virtio_kick(g_dev,0);
    g_used_head=g_evq.used->idx;
    g_inited=1;
    log_info("touch: ready");
    return 0;
}
int touch_online(void){ return g_inited; }
#else
int touch_init(void){ return 0; }
int touch_online(void){ return 0; }
void touch_tick(void){}
#endif
