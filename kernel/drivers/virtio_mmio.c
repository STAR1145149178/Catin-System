#include "catin.h"
#include "virtio.h"
#if ARCH_ID == 1 || ARCH_ID == 2
#define VMAGIC 0x74726976
extern u64 g_virtio_base;
extern u32 g_virtio_irq;
static virtio_dev_t devs[VIRTIO_MAX_DEV];
static int ndev;
static u8 vq_pool[128*1024] __attribute__((aligned(4096)));
static usize vq_head;
int virtio_init(void){
    ndev=0; vq_head=0;
    for(int i=0;i<32;i++){
        u64 base=g_virtio_base+(u64)i*0x200;
        if(mmio_r32(base+0x000)!=VMAGIC) continue;
        if(mmio_r32(base+0x004)!=2) continue;
        u32 id=mmio_r32(base+0x008); if(id==0) continue;
        if(ndev>=VIRTIO_MAX_DEV) break;
        devs[ndev].base=base; devs[ndev].device_id=id; devs[ndev].irq=g_virtio_irq;
        ndev++;
    }
    log_info("virtio: %d dev, irq=%u",ndev,g_virtio_irq);
    return 0;
}
int virtio_dev_count(void){ return ndev; }
virtio_dev_t *virtio_dev_at(int i){ return (i>=0&&i<ndev)?&devs[i]:NULL; }
virtio_dev_t *virtio_find(u32 id,int idx){
    int seen=0;
    for(int i=0;i<ndev;i++) if(devs[i].device_id==id){ if(seen++==idx) return &devs[i]; }
    return NULL;
}
u64 virtio_alloc_vq_mem(usize bytes){
    bytes=(bytes+4095)&~4095UL;
    if(vq_head+bytes>sizeof(vq_pool)){ log_err("vq full"); return 0; }
    u64 p=(u64)(uintptr_t)(vq_pool+vq_head);
    vq_head+=bytes; kmemset((void*)(uintptr_t)p,0,bytes); return p;
}
int virtio_setup_queue(virtio_dev_t*d,u32 qidx,virtq_t*q,u32 qsize){
    mmio_w32(d->base+0x028,qidx);
    u32 mx=mmio_r32(d->base+0x02c);
    if(mx==0){ log_err("vq: qmax=0"); return -1; }
    if(qsize==0||qsize>mx) qsize=mx;
    u32 p=1; while(p*2<=qsize) p*=2; qsize=p; if(qsize<1) qsize=1;
    q->size=qsize; q->last_used=0;
    q->desc=(virtq_desc_t*)virtio_alloc_vq_mem(sizeof(virtq_desc_t)*qsize);
    q->avail=(virtq_avail_t*)virtio_alloc_vq_mem(VIRTQ_AVAIL_HDR+2*qsize+2);
    q->used=(virtq_used_t*)virtio_alloc_vq_mem(VIRTQ_USED_HDR+8*qsize+2);
    if(!q->desc||!q->avail||!q->used) return -1;
    mmio_w32(d->base+0x030,qsize);
    mmio_w32(d->base+0x080,(u32)(u64)q->desc);
    mmio_w32(d->base+0x084,(u32)((u64)q->desc>>32));
    mmio_w32(d->base+0x090,(u32)(u64)q->avail);
    mmio_w32(d->base+0x094,(u32)((u64)q->avail>>32));
    mmio_w32(d->base+0x0a0,(u32)(u64)q->used);
    mmio_w32(d->base+0x0a4,(u32)((u64)q->used>>32));
    mmio_w32(d->base+0x044,1); return 0;
}
void virtio_kick(virtio_dev_t*d,u32 q){ mmio_w32(d->base+0x050,q); }
u32 virtio_ack_irq(virtio_dev_t*d){ u32 st=mmio_r32(d->base+0x060); mmio_w32(d->base+0x064,st); return st; }
void virtio_set_status(virtio_dev_t*d,u32 s){ mmio_w32(d->base+0x070,s); }
void virtio_read_config(virtio_dev_t*d,u32 off,void*buf,u32 len){
    u8*b=(u8*)buf; for(u32 i=0;i<len;i++) b[i]=*(volatile u8*)(d->base+0x100+off+i);
}
#else
int virtio_init(void){ return 0; }
int virtio_dev_count(void){ return 0; }
virtio_dev_t *virtio_dev_at(int i){ (void)i; return NULL; }
virtio_dev_t *virtio_find(u32 id,int idx){ (void)id;(void)idx; return NULL; }
u64 virtio_alloc_vq_mem(usize bytes){ (void)bytes; return 0; }
int virtio_setup_queue(virtio_dev_t*d,u32 q,virtq_t*qq,u32 s){ (void)d;(void)q;(void)qq;(void)s; return -1; }
void virtio_kick(virtio_dev_t*d,u32 q){ (void)d;(void)q; }
u32 virtio_ack_irq(virtio_dev_t*d){ (void)d; return 0; }
void virtio_set_status(virtio_dev_t*d,u32 s){ (void)d;(void)s; }
void virtio_read_config(virtio_dev_t*d,u32 o,void*b,u32 l){ (void)d;(void)o;(void)b;(void)l; }
#endif
