//---------------------------------------------------------------------------
//  Description:    A Generic File I/O Wrapper
//  Author:         Hongjie Guan
//---------------------------------------------------------------------------

#ifndef XFILE_H_
#define XFILE_H_

#ifdef __cplusplus
extern "C"
{
#endif


//---------------------------------------------------------------------------
//  XFILE internal data structures
//---------------------------------------------------------------------------

typedef struct xfile_mode_t
{
    void *open;
    void *close;
    void *next;
    void *read;
    void *write;
} xfile_mode_t;

typedef struct xfile_t
{
    char *          dir;
    char *          name;
    int             rec_size;
    xfile_mode_t *  xfile_mode;
    void *          hdl;
    int             cntr;
    int             flags;
} xfile_t;

typedef int (* xf_open_fnp  )(void *xfile, char *dir, char *name, int rw, int rec_size);
typedef int (* xf_close_fnp )(void *xfile);
typedef int (* xf_next_fnp  )(void *xfile);
typedef int (* xf_read_fnp  )(void *xfile, void *buf, int n);
typedef int (* xf_write_fnp )(void *xfile, void *buf, int n);


//---------------------------------------------------------------------------
//  XFILE mode definitions
//---------------------------------------------------------------------------

#define XFILE_MODES(modes_)         modes_
#define XFILE_MODE(mode_)           extern  xfile_mode_t *  xfile_mode_##mode_; \
                                    extern  xfile_mode_t g_xfile_mode_##mode_;

#include "xfile_cfg.h"

#undef XFILE_MODES
#undef XFILE_MODE


//---------------------------------------------------------------------------
//  XFILE API
//---------------------------------------------------------------------------

xfile_t *   xfile_open      (xfile_t *xfile, char *dir, char *name,
                             xfile_mode_t *mode, int rw, int rec_size);
int         xfile_close     (xfile_t *xfile);
int         xfile_next      (xfile_t *xfile);
int         xfile_read      (xfile_t *xfile, void *buf, int n);
int         xfile_write     (xfile_t *xfile, void *buf, int n);


//---------------------------------------------------------------------------
//  THE END
//---------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
