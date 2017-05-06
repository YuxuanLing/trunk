//---------------------------------------------------------------------------
//  Description:    Data Dumping Utilities
//  Author:         Hongjie Guan
//---------------------------------------------------------------------------

#include "stdio.h"
#include "malloc.h"

#define xdbg        printf
#define xmalloc     malloc
#define xfree       free

#include "xdump.h"

#define XDUMP_GROUPS(groups_)       enum { groups_ xd_num_groups };
#define XDUMP_GRP(g_)               xd_grp_##g_,
#define XDUMP_TABLE(name_,table_)   static xdump_entry_t g_xdw_tbl_##name_[] =  { table_ }; \
                                    void *xdw_tbl_##name_ = g_xdw_tbl_##name_; \
                                    static xdump_entry_t g_xdr_tbl_##name_[] =  { table_ }; \
                                    void *xdr_tbl_##name_ = g_xdr_tbl_##name_; \
                                    void *xd_tbl_##name_ = g_xdw_tbl_##name_;
#define XDUMP_ENTRY(g_,x_,s_,m_)    { (xd_grp_##g_), -1, \
                                      { 0L, (#g_ "." #x_), (s_), (&g_xfile_mode_##m_), 0, 0, 0 } },

#include "xdump_cfg.h"

#undef XDUMP_GROUPS
#undef XDUMP_GRP
#undef XDUMP_TABLE
#undef XDUMP_ENTRY


xdump_t *xdump_init(void *table, int num_entries, char *dir)
{
    xdump_t *xd_ctx;
    
    xd_ctx = malloc(sizeof(xdump_t));
    if (!xd_ctx)
    {
        xdbg("XDUMP>> Unable to allocate memory for xdump context!\n");
        return 0L;
    }

    xd_ctx->table       = (xdump_entry_t *)table;
    xd_ctx->dir         = dir;
    xd_ctx->num_entries = num_entries;

    return xd_ctx;
}

int xdump_close(xdump_t *ctx)
{
    int i;
    xdump_t *xd_ctx = (xdump_t *)ctx;

    for (i = 0; i < xd_ctx->num_entries; i++)
    {
        if (xd_ctx->table[i].state >= 0)
        {
            xfile_close(&xd_ctx->table[i].xfile);
            xd_ctx->table[i].state = 0;
        }
    }

    free(xd_ctx);

    return 0;
}

int xdump_open(xdump_t *ctx, int rw)
{
    int i, n;
    xdump_t *xd_ctx = (xdump_t *)ctx;
    xdump_entry_t *xi;

    for (i = 0; i < xd_ctx->num_entries; i++)
    {
        xi = &xd_ctx->table[i];
        if (xi->state)
        {
            if (!xfile_open(&xi->xfile, xd_ctx->dir, xi->xfile.name, xi->xfile.xfile_mode, rw, xi->xfile.rec_size))
            {
                break;
            }
            xi->state = 1;
        }
    }
    n = i;
    for (; i < xd_ctx->num_entries; i++)
    {
        xd_ctx->table[i].state = 0;
    }

    if (n < xd_ctx->num_entries)
    {
        for (i = 0; i < n; i++)
        {
            if (xd_ctx->table[i].state)
            {
                xfile_close(&xd_ctx->table[i].xfile);
            }
        }
        return -1;
    }

    return 0;
}

int xdump_next(xdump_t *ctx)
{
    int i, r;
    xdump_t *xd_ctx = (xdump_t *)ctx;

    for (i = 0; i < xd_ctx->num_entries; i++)
    {
        if (xd_ctx->table[i].state > 0)
        {
            r = xfile_next(&xd_ctx->table[i].xfile); if (r < 0) return r;
        }
    }

    return 0;
}

int xdump_read(xdump_t *ctx, int id, void *buf, int n)
{
    xdump_t *xd_ctx = (xdump_t *)ctx;
    if (xd_ctx->table[id].state > 0)
    {
        return xfile_read(&xd_ctx->table[id].xfile, buf, n);
    }
    else
    {
        return 0;
    }
}

int xdump_write(xdump_t *ctx, int id, void *buf, int n)
{
    xdump_t *xd_ctx = (xdump_t *)ctx;
    if (xd_ctx->table[id].state > 0)
    {
        return xfile_write(&xd_ctx->table[id].xfile, buf, n);
    }
    else
    {
        return n;
    }
}
