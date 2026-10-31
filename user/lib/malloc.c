#include "catin.h"
#define HEAP_CHUNK (64*1024)
typedef struct blk { usize size; int free; struct blk *next; } blk_t;
static u8 *heap_base;
static usize heap_size;
static usize heap_used;
static blk_t *freelist;
void *catin_malloc(usize n){
    if(!n) return NULL;
    n = (n + 15) & ~15UL;
    if(!heap_base){
        void *p = catin_brk((void*)0);
        if(!p) return NULL;
        heap_base = (u8*)p; heap_size = 0; heap_used = 0; freelist = NULL;
    }
    for(blk_t*b=freelist;b;b=b->next){
        if(b->free && b->size>=n){ b->free=0; return (void*)((u8*)b + sizeof(blk_t)); }
    }
    if(heap_used + sizeof(blk_t) + n > heap_size){
        void *newtop = (void*)(heap_base + heap_size + HEAP_CHUNK);
        void *got = catin_brk(newtop);
        if(!got || got != newtop) return NULL;
        heap_size += HEAP_CHUNK;
    }
    blk_t*b = (blk_t*)(heap_base + heap_used);
    b->size = n; b->free = 0; b->next = freelist;
    freelist = b;
    heap_used += sizeof(blk_t) + n;
    return (void*)((u8*)b + sizeof(blk_t));
}
void *catin_calloc(usize n,usize sz){ void*p=catin_malloc(n*sz); if(p)catin_memset(p,0,n*sz); return p; }
void catin_free(void*p){ if(!p) return; blk_t*b=(blk_t*)((u8*)p-sizeof(blk_t)); b->free=1; }
void *catin_realloc(void*p,usize n){
    if(!p) return catin_malloc(n);
    blk_t*b=(blk_t*)((u8*)p-sizeof(blk_t));
    if(b->size>=n) return p;
    void*q=catin_malloc(n);
    if(!q) return NULL;
    catin_memcpy(q,p,b->size);
    catin_free(p);
    return q;
}
