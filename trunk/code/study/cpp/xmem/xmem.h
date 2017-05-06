//---------------------------------------------------------------------------
//  Copyright 2008 Marvell Semiconductor Inc
//---------------------------------------------------------------------------
//! \file
//! \brief      Simple memory allocators
//! \author     Hongjie Guan
//---------------------------------------------------------------------------

#ifndef XMEM_H_
#define XMEM_H_

#ifdef __cplusplus
extern "C"
{
#endif


#define MEM_POOL_MAX_BLOCKS     (16*1024)
#define MEM_POOL_MAX_CALLERS    (0x80)

typedef struct mem_pool_t mem_pool_t; struct mem_pool_t
{
    unsigned char pool_tag;
    size_t  base;
    size_t  size;
    size_t  blk_size;
    int     blks;
    unsigned char tags  [MEM_POOL_MAX_BLOCKS][2];
    int     level       [MEM_POOL_MAX_CALLERS];
    int     peak        [MEM_POOL_MAX_CALLERS];
    int     level_all;
    int     peak_all;
};

#define XMEM_MAX_VMEM_POOLS     2
#define XMEM_MAX_PMEM_POOLS     8

typedef struct xmem_t xmem_t; struct xmem_t
{
    unsigned int vmem_pool_cnt;
    void *vmem_vbase;
    size_t vmem_pbase;
    size_t vmem_size;
    size_t vmem_ptr;

    unsigned int pmem_pool_cnt;
    void *pmem_vbase;
    size_t pmem_pbase;
    size_t pmem_size;
    size_t pmem_ptr;

    mem_pool_t vmem_pools[XMEM_MAX_VMEM_POOLS];
    mem_pool_t pmem_pools[XMEM_MAX_PMEM_POOLS];
};


void *      aligned_malloc          (size_t size, size_t align_size);
void        aligned_free            (void *ptr);

void        xmem_init               (struct xmem_t *xmem,
                                     void *vmem_vbase, size_t vmem_pbase, size_t vmem_size,
                                     void *pmem_vbase, size_t pmem_pbase, size_t pmem_size);
void        xmem_create_vmem_pool   (struct xmem_t *xmem, unsigned char pool_tag, size_t size, size_t blk_size);
void        xmem_create_pmem_pool   (struct xmem_t *xmem, unsigned char pool_tag, size_t size, size_t blk_size);

int         xmem_check              (struct xmem_t *xmem, unsigned char caller_tag, int reset);
size_t      xmem_alloc              (struct xmem_t *xmem, unsigned int tags, size_t size, size_t align_size);
int         xmem_free               (struct xmem_t *xmem, unsigned int tags, size_t addr);


#ifdef __cplusplus
}
#endif

#endif

