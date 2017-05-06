//---------------------------------------------------------------------------
//  Description:    XFILE modes Implementation
//  Author:         Hongjie Guan
//---------------------------------------------------------------------------

#include "stdio.h"
#include "xfile.h"
#include "cm_util.h"


//---------------------------------------------------------------------------
//  xfile_mode_nul: do nothing.
//---------------------------------------------------------------------------

int xfile_open_nul(xfile_t *xfile, char *dir, char *name, int rw, int rec_size)
{
    return 0;
}

int xfile_close_nul(xfile_t *xfile)
{
    return 0;
}

int xfile_next_nul(xfile_t *xfile)
{
    return 0;
}

int xfile_read_nul(xfile_t *xfile, void *buf, int n)
{
    return n;
}

int xfile_write_nul(xfile_t *xfile, void *buf, int n)
{
    return n;
}


//---------------------------------------------------------------------------
//  xfile_mode_bin: raw binary file.
//---------------------------------------------------------------------------

int xfile_open_bin(xfile_t *xfile, char *dir, char *name, int rw, int rec_size)
{
    xfile->hdl = fopen(name, rw ? "wb" : "rb");
    return xfile->hdl ? 0 : -1;
}

int xfile_close_bin(xfile_t *xfile)
{
    if (xfile) if (xfile->hdl) fclose(xfile->hdl);
    return 0;
}

int xfile_next_bin(xfile_t *xfile)
{
    return 0;
}

int xfile_read_bin(xfile_t *xfile, void *buf, int n)
{
    return (int)fread(buf, xfile->rec_size, n, xfile->hdl);
}

int xfile_write_bin(xfile_t *xfile, void *buf, int n)
{
    return (int)fwrite(buf, xfile->rec_size, n, xfile->hdl);
}


//---------------------------------------------------------------------------
//  xfile_mode_txt: hexadecimal text file.
//---------------------------------------------------------------------------

int xfile_open_txt(xfile_t *xfile, char *dir, char *name, int rw, int rec_size)
{
    xfile->hdl = fopen(name, rw ? "w" : "r");
    return xfile->hdl ? 0 : -1;
}

int xfile_close_txt(xfile_t *xfile)
{
    if (xfile) if (xfile->hdl) fclose(xfile->hdl);
    return 0;
}

int xfile_next_txt(xfile_t *xfile)
{
    fprintf(xfile->hdl, "<END> #%d\n", xfile->cntr);
    return 0;
}

int xfile_read_txt(xfile_t *xfile, void *buf, int n)
{
    //TODO: not supported yet
    return 0;
}

int xfile_write_txt(xfile_t *xfile, void *buf, int n)
{
    int i;
    for (i = 0; i < xfile->rec_size; i++)
    {
        fprintf(xfile->hdl, "%02x ", ((unsigned char *)buf)[i]);
    }
    fprintf(xfile->hdl, "\n");
    return n;
}


//---------------------------------------------------------------------------
//  xfile_mode_ser: serializer file.
//---------------------------------------------------------------------------

int xfile_open_ser(xfile_t *xfile, char *dir, char *name, int rw, int rec_size)
{
    int r;
    //TODO: configurable history size?
    r = _SerializerOpen(dir, name, rec_size, rw ? -1 : 0);
    if (r < 0) return r;
    xfile->hdl = (void *)r;
    return 0;
}

int xfile_close_ser(xfile_t *xfile)
{
    return 0;
}

int xfile_next_ser(xfile_t *xfile)
{
    int r;
    r = SerializerWriteEOF((int)(xfile->hdl));
    if (r < 0) return r;
    return 0;
}

int xfile_read_ser(xfile_t *xfile, void *buf, int n)
{
    int i, r;
    char *p = buf;
    for (i = 0; i < n; i++)
    {
        do {
            r = SerializerRead((int)(xfile->hdl), p);
        } while (!r);
        if (r < 0) break;
        p += xfile->rec_size;
    }
    return i;
}

int xfile_write_ser(xfile_t *xfile, void *buf, int n)
{
    int i, r;
    char *p = buf;
    for (i = 0; i < n; i++)
    {
        r = SerializerWrite((int)(xfile->hdl), p, 1);
        if (r < 0) break;
        p += xfile->rec_size;
    }
    return i;
}


//---------------------------------------------------------------------------
//  THE END
//---------------------------------------------------------------------------
