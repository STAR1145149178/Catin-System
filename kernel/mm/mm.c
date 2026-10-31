#include "catin.h"
extern char __kernel_end[];
#define HEAP_START ((u64)(uintptr_t)__kernel_end)
#if ARCH_ID == 1 || ARCH_ID == 2
  #define HEAP_END 0x60000000ULL   /* ARM: RAM 到 0x60000000 */
#else
  #define HEAP_END 0x20000000ULL   /* x86: RAM 到 0x20000000 (512MB) */
#endif
typedef struct __attribute__((aligned(16))) blk { usize size; struct blk *next; int free; } blk_t;
static u8 *heap; static usize heap_bytes; static blk_t *freelist;
void mm_init(void) {
    heap = (u8*)(uintptr_t)HEAP_START;
    heap_bytes = HEAP_END - HEAP_START;
    freelist = (blk_t*)heap;
    freelist->size = heap_bytes - sizeof(blk_t);
    freelist->free = 1; freelist->next = NULL;
    log_info("mm: %p + %lu KB", heap, (unsigned long)(heap_bytes>>10));
}
static void split(blk_t *b, usize n) {
    if (b->size <= n + sizeof(blk_t) + 32) return;
    blk_t *nb = (blk_t*)((u8*)b + sizeof(blk_t) + n);
    nb->size = b->size - n - sizeof(blk_t);
    nb->free = 1; nb->next = b->next;
    b->size = n; b->next = nb;
}
void *kmalloc(usize n) {
    if (!n) return NULL;
    n = (n + 15) & ~15UL;
    for (blk_t *b = freelist; b; b = b->next)
        if (b->free && b->size >= n) { split(b,n); b->free=0; return (void*)((u8*)b + sizeof(blk_t)); }
    log_err("kmalloc: OOM %lu", (unsigned long)n); return NULL;
}
void *kzalloc(usize n) { void *p=kmalloc(n); if(p) kmemset(p,0,n); return p; }
static void merge(void) {
    for (blk_t *b = freelist; b && b->next; )
        if (b->free && b->next->free) { b->size += sizeof(blk_t) + b->next->size; b->next = b->next->next; }
        else b = b->next;
}
void kfree(void *p) { if(!p) return; blk_t *b=(blk_t*)((u8*)p-sizeof(blk_t)); if(b->free){log_err("kfree double");return;} b->free=1; merge(); }
usize mm_used(void){ usize u=0; for(blk_t*b=freelist;b;b=b->next) if(!b->free) u+=b->size; return u; }
usize mm_free(void){ usize f=0; for(blk_t*b=freelist;b;b=b->next) if(b->free) f+=b->size; return f; }
