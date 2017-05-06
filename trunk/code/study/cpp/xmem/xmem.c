//---------------------------------------------------------------------------
//  Copyright 2008 Marvell Semiconductor Inc
//---------------------------------------------------------------------------
//! \file
//! \brief      Simple memory allocators
//! \author     Hongjie Guan
//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xdbg.h"
#include "xmem.h"


//---------------------------------------------------------------------------
//  Memory allocator 1:
//      Aligned memory allocation based on malloc()/free().
//      align_size must be a power of 2.
//      Best for virtual memory allocation.
//      If used for physical memory, caller needs to take care of the cache
//      coherence issues. malloc()/free() may leave the cached memory dirty.
//---------------------------------------------------------------------------

void *aligned_malloc(size_t size, size_t align_size)
{
    char *ptr, *aligned_ptr;

    ptr = (char *)malloc(size + align_size + sizeof(size_t));
    if (!ptr) return ptr;

    aligned_ptr = ptr + sizeof(size_t);
    aligned_ptr += align_size - ((size_t)aligned_ptr & (align_size - 1));
    *((size_t *)(aligned_ptr - sizeof(size_t))) = (size_t)(aligned_ptr - ptr);

    return aligned_ptr;
}

void aligned_free(void *ptr)
{
    free((char *)ptr - *((size_t *)ptr - 1));
}


//---------------------------------------------------------------------------
//  Memory allocator 2:
//      A simple (and very inefficient) memory allocator used mainly for
//      physical memory allocation in test benches. Different types of
//      memories can be allocated from different memory pools.
//      There is no cache coherence issue since the allocator never touches
//      the memory it allocated.
//---------------------------------------------------------------------------

static int mem_pool_init(mem_pool_t *mem_pool, unsigned char pool_tag, size_t base, size_t size, size_t blk_size)
{
    xdbg("@mem>> mem_pool_init(pool_tag='%c' base=0x%08x size=0x%08x blk_size=0x%08x)\n",
        pool_tag, base, size, blk_size);

    xdbg("@sizeof>> mem_pool_t %d\n", sizeof(mem_pool_t));
    xdbg("@sizeof>> mem_pool_t.tags %d\n", sizeof(mem_pool->tags));
    xdbg("@sizeof>> mem_pool_t.level %d\n", sizeof(mem_pool->level));
    xdbg("@sizeof>> mem_pool_t.peak %d\n", sizeof(mem_pool->peak));

    memset(mem_pool, 0, sizeof(mem_pool_t));
    mem_pool->pool_tag = pool_tag;
    mem_pool->base = base;
    mem_pool->size = size;
    mem_pool->blk_size = blk_size;
    mem_pool->blks = size/blk_size;
    xbug((mem_pool->blks)*(mem_pool->blk_size) != (mem_pool->size));
    xbug((mem_pool->blks) > MEM_POOL_MAX_BLOCKS);
    return 0;
}

static int mem_pool_check(mem_pool_t *mem_pool, unsigned char caller_tag, int reset)
{
    int r = 0, i;
    xmsg("@mem>> mem_pool_check(pool_tag='%c' caller_tag='%c' reset=%d)\n",
        mem_pool->pool_tag, caller_tag, reset);

    if (caller_tag == '*')
    {
        xmsg("@mem>> \t total: %d/%d\n", mem_pool->level_all, mem_pool->peak_all);
        for (i = 0; i < MEM_POOL_MAX_CALLERS; i++)
        {
            if (mem_pool->peak[i])
            {
                xmsg("@mem>> \t caller '%c': %d/%d\n",
                    i, mem_pool->level[i], mem_pool->peak[i]);
            }
        }
    }
    else
    {
        xmsg("@mem>> \t caller '%c': %d/%d\n",
             caller_tag, mem_pool->level[caller_tag], mem_pool->peak[caller_tag]);
    }

    for (i = 0; i < MEM_POOL_MAX_BLOCKS; i++)
    {
        if (mem_pool->tags[i][0])
        {
            if (caller_tag == '*' || (mem_pool->tags[i][1] & ~0x80) == caller_tag)
            {
                r++;
                xmsg("@mem>> \t blk 0x%04x: 0x%08x '%c' 0x%02x\n", i,
                    mem_pool->base + i * mem_pool->blk_size,
                    mem_pool->tags[i][0], mem_pool->tags[i][1]
                    );
                if (reset)
                {
                    mem_pool->tags[i][0] = 0;
                    mem_pool->tags[i][1] = 0;
                    mem_pool->level[mem_pool->tags[i][1] & ~0x80]--;
                    mem_pool->level_all--;
                }
            }
        }
        else xbug(mem_pool->tags[i][1]);
    }
    xmsg("@mem>> \t blocks used: %d\n", r);

    return r;
}

static size_t mem_pool_alloc(mem_pool_t *mem_pool, size_t size, unsigned char mem_tag, unsigned char caller_tag)
{
    int i, sz, r0, r1, n;
    size_t addr;

    xdbg("@mem>> +++ mem_pool_alloc(pool_tag='%c' mem_tag='%c' caller_tag='%c' size=0x%08x) {pool: %d/%d %d/%d}\n",
        mem_pool->pool_tag, mem_tag, caller_tag, size,
        mem_pool->level[caller_tag], mem_pool->peak[caller_tag],
        mem_pool->level_all, mem_pool->peak_all
        );

    xbug(caller_tag >= MEM_POOL_MAX_CALLERS);

    sz = (size+(mem_pool->blk_size)-1)/(mem_pool->blk_size);
    r0 = 0;
    r1 = r0 + mem_pool->blks;

    n = 0;
    for (i = r0; i < r1; i++)
    {
        if (mem_pool->tags[i][0]) n = 0;
        else n++;
        if (n == sz) break;
    }

    if (sz && n == sz)
    {
        n = i - n + 1;
        addr = mem_pool->base + n * mem_pool->blk_size;
        for (i = n; i < n + sz; i++)
        {
            mem_pool->tags[i][0] = mem_tag;
            mem_pool->tags[i][1] = caller_tag;
        }
        mem_pool->tags[n][1] |= 0x80;

        mem_pool->level[caller_tag] += sz;
        if (mem_pool->level[caller_tag] > mem_pool->peak[caller_tag])
            mem_pool->peak[caller_tag] = mem_pool->level[caller_tag];
        mem_pool->level_all += sz;
        if (mem_pool->level_all > mem_pool->peak_all)
            mem_pool->peak_all = mem_pool->level_all;
    }
    else
    {
        addr = n = 0;
        mem_pool_check(mem_pool, '*', 0);
        //xbug("out of memory");
    }

    xdbg("@mem>> --- mem_pool_alloc(pool_tag='%c' mem_tag='%c' caller_tag='%c' size=0x%08x): 0x%08x {pool: +%d %d/%d %d/%d}\n",
        mem_pool->pool_tag, mem_tag, caller_tag, size, addr, sz,
        mem_pool->level[caller_tag], mem_pool->peak[caller_tag],
        mem_pool->level_all, mem_pool->peak_all
        );

    return addr;
}

static int mem_pool_free(mem_pool_t *mem_pool, size_t addr, unsigned char mem_tag, unsigned char caller_tag)
{
    int sz= 0, i;

    xdbg("@mem>> +++ mem_pool_free(pool_tag='%c' mem_tag='%c' caller_tag='%c' addr=0x%08x) {pool: %d/%d %d/%d}\n",
        mem_pool->pool_tag, mem_tag, caller_tag, addr,
        mem_pool->level[caller_tag], mem_pool->peak[caller_tag],
        mem_pool->level_all, mem_pool->peak_all
        );

    i = (addr - mem_pool->base)/mem_pool->blk_size;

    if (!(mem_pool->tags[i][1] & 0x80))
    {
        xbug("invalid start address!");
        return -1;
    }

    do
    {
        xbug((mem_pool->tags[i][0]) != mem_tag);
        xbug((mem_pool->tags[i][1] & ~0x80) != caller_tag);
        mem_pool->tags[i][0] = 0;
        mem_pool->tags[i][1] = 0;
        sz++;
        i++;
    } while (mem_pool->tags[i][0] && !(mem_pool->tags[i][1] & 0x80));

    mem_pool->level[caller_tag] -= sz;
    mem_pool->level_all -= sz;

    xdbg("@mem>> --- mem_pool_free(pool_tag='%c' mem_tag='%c' caller_tag='%c' addr=0x%08x) {pool: -%d %d/%d %d/%d}\n",
        mem_pool->pool_tag, mem_tag, caller_tag, addr, sz,
        mem_pool->level[caller_tag], mem_pool->peak[caller_tag],
        mem_pool->level_all, mem_pool->peak_all
        );

    return 0;
}

//---------------------------------------------------------------------------
//  Physical and virtual memory manager.
//---------------------------------------------------------------------------

void xmem_create_vmem_pool(xmem_t *xmem, unsigned char pool_tag, size_t size, size_t blk_size)
{
    int r;

    xdbg("@mem>> xmem_create_vmem_pool(pool_tag='%c' size=%08x blk_size=0x%08x base=0x%08x)\n",
        pool_tag, size, blk_size, xmem->vmem_ptr);

    xbug(xmem->vmem_pool_cnt >= XMEM_MAX_VMEM_POOLS);
    xbug(xmem->vmem_ptr + size > xmem->vmem_pbase + xmem->vmem_size);

    r = mem_pool_init(
            &xmem->vmem_pools[xmem->vmem_pool_cnt],
            pool_tag,
            xmem->vmem_ptr,
            size,
            blk_size
            ); xbug(r<0);
    xmem->vmem_pools[xmem->vmem_pool_cnt],

    xmem->vmem_ptr += size;
    xmem->vmem_pool_cnt++;
}

void xmem_create_pmem_pool(xmem_t *xmem, unsigned char pool_tag, size_t size, size_t blk_size)
{
    int r;

    xdbg("@mem>> xmem_create_pmem_pool(pool_tag='%c' size=%08x blk_size=0x%08x base=0x%08x)\n",
        pool_tag, size, blk_size, xmem->pmem_ptr);

    xbug(xmem->pmem_pool_cnt >= XMEM_MAX_PMEM_POOLS);
    xbug(xmem->pmem_ptr + size > xmem->pmem_pbase + xmem->pmem_size);

    r = mem_pool_init(
            &xmem->pmem_pools[xmem->pmem_pool_cnt],
            pool_tag,
            xmem->pmem_ptr,
            size,
            blk_size
            ); xbug(r<0);
    xmem->pmem_pools[xmem->pmem_pool_cnt],

    xmem->pmem_ptr += size;
    xmem->pmem_pool_cnt++;
}

void xmem_init(xmem_t *xmem,
               void *vmem_vbase, size_t vmem_pbase, size_t vmem_size,
               void *pmem_vbase, size_t pmem_pbase, size_t pmem_size)
{
    memset(xmem, 0, sizeof(xmem_t));

    xmem->vmem_vbase = vmem_vbase;
    xmem->vmem_pbase = vmem_pbase;
    xmem->vmem_size = vmem_size;
    xmem->vmem_ptr = xmem->vmem_pbase;

    xmem->pmem_vbase = pmem_vbase;
    xmem->pmem_pbase = pmem_pbase;
    xmem->pmem_size = pmem_size;
    xmem->pmem_ptr = xmem->pmem_pbase;
}

int xmem_check(xmem_t *xmem, unsigned char caller_tag, int reset)
{
    int r, rsum = 0;
    unsigned int i;
    mem_pool_t *pool;

    for (i = 0; i < xmem->vmem_pool_cnt; i++)
    {
        pool = &xmem->vmem_pools[i];
        r = mem_pool_check(pool, caller_tag, reset); xbug(r<0);
        rsum += r;
    }

    for (i = 0; i < xmem->pmem_pool_cnt; i++)
    {
        pool = &xmem->pmem_pools[i];
        r = mem_pool_check(pool, caller_tag, reset); xbug(r<0);
        rsum += r;
    }

    return rsum;
}

static mem_pool_t *xmem_get_pool_by_tag(xmem_t *xmem, unsigned char pool_tag)
{
    unsigned int i;
    mem_pool_t *pool;

    for (i = 0; i < xmem->vmem_pool_cnt; i++)
    {
        pool = &xmem->vmem_pools[i];
        if (pool->pool_tag == pool_tag) return pool;
    }

    for (i = 0; i < xmem->pmem_pool_cnt; i++)
    {
        pool = &xmem->pmem_pools[i];
        if (pool->pool_tag == pool_tag) return pool;
    }

    return 0L;
}

size_t xmem_alloc(xmem_t *xmem, unsigned int tags, size_t size, size_t align_size)
{
    unsigned char pool_tag   = (unsigned char)(tags>>16);
    unsigned char mem_tag    = (unsigned char)(tags>>8);
    unsigned char caller_tag = (unsigned char)(tags);
    mem_pool_t *pool = xmem_get_pool_by_tag(xmem, pool_tag); xbug(!pool);
    xbug(pool->blk_size / align_size * align_size != pool->blk_size);
    return mem_pool_alloc(pool, size, mem_tag, caller_tag);
}

int xmem_free(xmem_t *xmem, unsigned int tags, size_t addr)
{
    unsigned char pool_tag   = (unsigned char)(tags>>16);
    unsigned char mem_tag    = (unsigned char)(tags>>8);
    unsigned char caller_tag = (unsigned char)(tags);
    mem_pool_t *pool = xmem_get_pool_by_tag(xmem, pool_tag); xbug(!pool);
    return mem_pool_free(pool, addr, mem_tag, caller_tag);
}

