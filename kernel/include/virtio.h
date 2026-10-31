#ifndef _V_H
#define _V_H
#include "types.h"
#define VIRTIO_MAX_DEV 8
#define VIRTIO_ID_INPUT 18
typedef struct { u64 addr; u32 len; u16 flags; u16 next; } virtq_desc_t;
typedef struct { u16 flags; u16 idx; u16 ring[]; } virtq_avail_t;
typedef struct { u16 flags; u16 idx; struct { u32 id; u32 len; } ring[]; } virtq_used_t;
typedef struct { virtq_desc_t *desc; virtq_avail_t *avail; virtq_used_t *used; u32 size; u16 last_used; } virtq_t;
typedef struct { u64 base; u32 device_id; u32 irq; } virtio_dev_t;
int virtio_init(void); int virtio_dev_count(void);
virtio_dev_t *virtio_dev_at(int i); virtio_dev_t *virtio_find(u32 id, int idx);
u64 virtio_alloc_vq_mem(usize bytes);
int virtio_setup_queue(virtio_dev_t *d, u32 qidx, virtq_t *q, u32 qsize);
void virtio_kick(virtio_dev_t *d, u32 qidx);
u32 virtio_ack_irq(virtio_dev_t *d);
void virtio_set_status(virtio_dev_t *d, u32 s);
void virtio_read_config(virtio_dev_t *d, u32 off, void *buf, u32 len);
#define VIRTQ_DESC_F_WRITE 2
#define VIRTIO_STATUS_ACKNOWLEDGE 1
#define VIRTIO_STATUS_DRIVER 2
#define VIRTIO_STATUS_DRIVER_OK 4
#define VIRTQ_AVAIL_HDR 4
#define VIRTQ_USED_HDR 4
#endif
