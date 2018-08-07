//---------------------------------------------------------------------------
//  Description:    Data Dumping Utilities
//  Author:         Hongjie Guan
//---------------------------------------------------------------------------

#ifndef XDUMP_H_
#define XDUMP_H_

#include "xfile.h"


//---------------------------------------------------------------------------
//  XDUMP internal data structures
//---------------------------------------------------------------------------

typedef struct xdump_entry_t
{
    int             group_id;
    int             state;
    xfile_t         xfile;
} xdump_entry_t;

typedef struct xdump_t
{
    xdump_entry_t   *table;
    char *          dir;
    int             num_entries;
} xdump_t;


//---------------------------------------------------------------------------
//  Define XDUMP tables and data IDs
//---------------------------------------------------------------------------

#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif
#endif

#define XDUMP_GROUPS(groups_)
#define XDUMP_TABLE(name_, table_)      EXTERN_C void *xdw_tbl_##name_; \
                                        EXTERN_C void *xdr_tbl_##name_; \
                                        EXTERN_C void *xd_tbl_##name_; \
                                        enum { table_ xd_tbl_##name_##_entries };
#define XDUMP_ENTRY(g_,x_,s_,m_)        pkt_id_##g_##_##x_,

#include "xdump_cfg.h"

#undef XDUMP_GROUPS
#undef XDUMP_TABLE
#undef XDUMP_ENTRY


#ifdef __cplusplus
extern "C"
{
#endif

//---------------------------------------------------------------------------
//  XDUMP API
//---------------------------------------------------------------------------

xdump_t *   xdump_init      (void *table, int num_entries, char *dir);
int         xdump_open      (xdump_t *ctx, int rw);
int         xdump_close     (xdump_t *ctx);
int         xdump_next      (xdump_t *ctx);
int         xdump_read      (xdump_t *ctx, int id, void *buf, int n);
int         xdump_write     (xdump_t *ctx, int id, void *buf, int n);


//---------------------------------------------------------------------------
//  THE END
//---------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
