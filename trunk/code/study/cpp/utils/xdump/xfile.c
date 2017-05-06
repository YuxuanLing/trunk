//---------------------------------------------------------------------------
//  Description:    A Generic File I/O Wrapper
//  Author:         Hongjie Guan
//---------------------------------------------------------------------------

#include "stdio.h"
#include "malloc.h"

#define xdbg        printf
#define xmalloc     malloc
#define xfree       free

#include "xfile.h"


//---------------------------------------------------------------------------
//  XFILE mode definitions
//---------------------------------------------------------------------------

#define XFILE_MODES(modes_) modes_
#define XFILE_MODE(mode_) \
    \
    int    xfile_open_##mode_  (xfile_t *xfile, char *dir, char *name, \
                                int rw, int rec_size); \
    int    xfile_close_##mode_ (xfile_t *xfile); \
    int    xfile_next_##mode_  (xfile_t *xfile); \
    int    xfile_read_##mode_  (xfile_t *xfile, void *buf, int n); \
    int    xfile_write_##mode_ (xfile_t *xfile, void *buf, int n); \
    \
    xfile_mode_t g_xfile_mode_##mode_ = \
    { \
        xfile_open_##mode_,  \
        xfile_close_##mode_, \
        xfile_next_##mode_,  \
        xfile_read_##mode_,  \
        xfile_write_##mode_  \
    }; \
    xfile_mode_t *xfile_mode_##mode_ = &g_xfile_mode_##mode_;

#include "xfile_cfg.h"

#undef XFILE_MODES
#undef XFILE_MODE


//---------------------------------------------------------------------------
//  XFILE API
//---------------------------------------------------------------------------

xfile_t *xfile_open(xfile_t *xfile, char *dir, char *name, xfile_mode_t *mode, int rw, int rec_size)
{
    if (!xfile)
    {
        xfile = xmalloc(sizeof(xfile_t));
        if (!xfile)
        {
            xdbg("XDUMP>> Unable to allocate memory for xfile context!\n");
            return 0L;
        }
        xfile->flags = 1;
    }
    else
    {
        xfile->flags = 0;
    }

    if ((*(xf_open_fnp)(mode->open))(xfile, dir, name, rw, rec_size) < 0)
    {
        xdbg("XFILE>> unable to open '%s'!\n", name);
        if (xfile->flags)
        {
            xfree(xfile);
        }
        return 0L;
    }

    xfile->xfile_mode = mode;
    xfile->rec_size = rec_size;
    xfile->cntr = 0;
    xfile->dir = dir;
    xfile->name = name;

    //xdbg("XFILE>> '%s' opened successfully.\n", name);
    return xfile;
}

int xfile_close(xfile_t *xfile)
{
    (*(xf_close_fnp)(xfile->xfile_mode->close))(xfile);
    if (xfile->flags)
    {
        xfree(xfile);
    }
    //xdbg("XFILE>> closed.\n");
    return 0;
}

int xfile_next(xfile_t *xfile)
{
    int r;
    r = (*(xf_next_fnp)(xfile->xfile_mode->next))(xfile);
    xfile->cntr++;
    return r;
}

int xfile_read(xfile_t *xfile, void *buf, int n)
{
    return (*(xf_read_fnp)(xfile->xfile_mode->read))(xfile, buf, n);
}

int xfile_write(xfile_t *xfile, void *buf, int n)
{
    return (*(xf_write_fnp)(xfile->xfile_mode->write))(xfile, buf, n);
}

//---------------------------------------------------------------------------
//  THE END
//---------------------------------------------------------------------------
