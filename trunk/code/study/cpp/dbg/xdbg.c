//---------------------------------------------------------------------------
//  Copyright 2008 Marvell Semiconductor Inc
//---------------------------------------------------------------------------
//! \file
//! \brief      An all-purpose debug utility (super printf).
//! \author     Hongjie Guan
//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "xdbg.h"

#ifdef xdbg
#undef xdbg
#endif
//#define xdbg printf
#define xdbg 1?0:

#ifdef xmsg
#undef xmsg
#endif
//#define xmsg printf
#define xmsg 1?0:

#ifdef xerr
#undef xerr
#endif
//#define xerr printf
#define xerr 1?0:

#ifdef  xbug
#undef  xbug
#endif
#define xbug(c_) do{if(c_){xerr("@bug>>\npanic! (reason: %s)\n@ %s:%d\n\n",\
							(#c_),__FILE__,__LINE__); \
							xmsg("@vmeta>> finished (FAIL: exit abnormal)\n\n"); \
							exit(-1);}}while(0)

//---------------------------------------------------------------------------

#define XDBG_FTBL_SIZE      1024
#define XDBG_HASH_LEN       16
#define XDBG_HASH_COLS      4

#define XDBG_HASH_ROWS      256 //do not change
#define XDBG_CTBL_SIZE      256 //do not change

typedef int (* xdbg_func_t)(const char *format, va_list ap);
typedef struct xdbg_k_f_t { const char *keyword; xdbg_func_t func; } xdbg_k_f_t;
static int g_xdbg_keys = 0;
static int g_xdbg_hashes = 0;
static int g_xdbg_hash[XDBG_HASH_ROWS][2];
static char g_xdbg_cflag[XDBG_CTBL_SIZE];
static xdbg_func_t g_xdbg_ctbl[XDBG_CTBL_SIZE]; //for single-character keywords
static xdbg_k_f_t g_xdbg_ftbl[XDBG_FTBL_SIZE];  //for multi-character keywords
static int xdbg_func_null(const char *format, va_list ap) { return 0; }

//---------------------------------------------------------------------------
//! \brief      Calculate hash and length of keyword.
//---------------------------------------------------------------------------
static int xdbg_hash(const char *keyword, int *hash)
{
    int i;
    *hash = 0;
    for (i = 1; i <= XDBG_HASH_LEN; i++)
    {
        if (keyword[i] == 0 || keyword[i] == '>') break;
        xdbg("%c", keyword[i]);
        //*hash ^= (keyword[i] & 31) << ((i & 1) << 2);
        *hash ^= (keyword[i] & 15) << ((i & 1) << 2);
        //*hash ^= (keyword[i] & 3) << ((i & 3) << 1);
    }
    xdbg(" => hash: 0x%02x len: %d\n", *hash, i-1);
    return i;
}

//---------------------------------------------------------------------------
//! \brief      Look up function pointer based on keyword.
//---------------------------------------------------------------------------
static xdbg_func_t xdbg_func(const char *keyword, int *len)
{
    int i, n0, n1;
    *len = xdbg_hash(keyword, &i) + 2;
    n0 = g_xdbg_hash[i][0];
    n1 = g_xdbg_hash[i][1];
    if (n0 != n1)
    {
        for (i = n0; i < n1; i++)
        {
            xdbg("hash cmp\n");
            if (!strncmp(keyword+1, g_xdbg_ftbl[i].keyword+1, (*len)-1))
                return g_xdbg_ftbl[i].func;
        }
        for (i = g_xdbg_hashes; i < g_xdbg_keys; i++)
        {
            xdbg("wild cmp\n");
            if (!strncmp(keyword+1, g_xdbg_ftbl[i].keyword+1, (*len)-1))
                return g_xdbg_ftbl[i].func;
        }
    }
    return g_xdbg_ftbl[0].func;
}


void xdbg_sample_panic(int code)
{
	xmsg("@vmeta>> finished (FAIL: exit abnormal)\n");
	exit(code);
}

//---------------------------------------------------------------------------
//! \brief      Super printf
//---------------------------------------------------------------------------
int xdbg_printf(const char *format, ...)
{
    va_list ap;
    xdbg_func_t func;
    int len, r = 0;
    va_start(ap, format);
    if (format[0] == '@')
    {
        if (format[2] == '>' && format[3] == '>')
        {//fast lookup for single-character keywords
            (*g_xdbg_ctbl[(unsigned char)(format[1])])(format, ap);
        }
        else if (format[1] == '+')
        {
            func = xdbg_func(format+1, &len);
            r = (*func)(format+len+1, ap);
        }
        else r = (*xdbg_func(format, &len))(format, ap);
    }
    else r = (*g_xdbg_ftbl[0].func)(format, ap);
    va_end(ap);
    return r;
}

//---------------------------------------------------------------------------
//! \brief      Associate keyword with function pointer
//---------------------------------------------------------------------------
int xdbg_bind(const char *keyword, void *func)
{
    int i, h, n0, n1;
    if (!func) func = (void *)xdbg_func_null;
    if (keyword)
    {
        if (keyword[0] != '@') return 0;
        if (keyword[2] == '>' && keyword[3] == '>')
        {//single-character commands for fast processing
            g_xdbg_ctbl[(unsigned char)(keyword[1])] = (xdbg_func_t)func;
            g_xdbg_cflag[(unsigned char)(keyword[1])] = 1;
            return 0;
        }
        n0 = xdbg_hash(keyword, &h);
        if (n0 > XDBG_HASH_LEN)
        {
            xerr("\nERROR: keyword is too long!\n");
            xbug("keyword is too long");
            return -1;
        }
        xdbg("find '%s', len=%d hash=0x%02x\n", keyword, n0, h);
        for (i = 1; i < g_xdbg_keys; i++)
        {   //check for duplicate keywords
            if (!strncmp(keyword, g_xdbg_ftbl[i].keyword, n0+2))
            {
                xdbg("found idx=%d 0%08x <- 0x%08x\n", i, g_xdbg_ftbl[i].func, func);
                g_xdbg_ftbl[i].func = (xdbg_func_t)func;
                return 0;
            }
        }
        if (g_xdbg_keys >= XDBG_FTBL_SIZE-1)
        {
            xerr("\nERROR: too many (%d) xdbg keywords!\n\n", g_xdbg_keys+1);
            xbug("too many xdbg keywords");
            return -1;
        }
        n0 = g_xdbg_hash[h][0];
        n1 = g_xdbg_hash[h][1];
        if (n1 - n0 >= XDBG_HASH_COLS)
        {//too many conflicts, demote to unhashed keyword
            g_xdbg_ftbl[g_xdbg_keys].keyword = keyword;
            g_xdbg_ftbl[g_xdbg_keys].func = (xdbg_func_t)func;
        }
        else
        {//insert new hashed keyword
            for (i = g_xdbg_keys; i > n1; i--)
            {
                g_xdbg_ftbl[i].keyword = g_xdbg_ftbl[i-1].keyword;
                g_xdbg_ftbl[i].func = g_xdbg_ftbl[i-1].func;
            }
            g_xdbg_ftbl[n1].keyword = keyword;
            g_xdbg_ftbl[n1].func = (xdbg_func_t)func;
            g_xdbg_hashes++;
            g_xdbg_hash[h][1]++;
            for (i = h+1; i < XDBG_HASH_ROWS; i++)
            {
                g_xdbg_hash[i][0]++;
                g_xdbg_hash[i][1]++;
            }
        }
        g_xdbg_keys++;
    }
    else
    {//default function pointer for unrecognized keywords
        for (i = 0; i < XDBG_CTBL_SIZE; i++)
            if (!g_xdbg_cflag[i]) g_xdbg_ctbl[i] = (xdbg_func_t)func;
        g_xdbg_ftbl[0].func = (xdbg_func_t)func;
    }
    return 0;
}

//---------------------------------------------------------------------------
//! \brief      Initialize xdbg utility
//---------------------------------------------------------------------------
int xdbg_init(void)
{
    int i;
    memset(g_xdbg_cflag, 0, sizeof(g_xdbg_cflag));
    g_xdbg_keys = 1;
    g_xdbg_hashes = 1;
    for (i = 0; i < XDBG_HASH_ROWS; i++)
        g_xdbg_hash[i][0] = g_xdbg_hash[i][1] = g_xdbg_hashes;
    xdbg_bind(0L, 0L);
    return 0;
}

//---------------------------------------------------------------------------
//! \brief      Dump internal info for xdbg utility
//---------------------------------------------------------------------------
void xdbg_dump(void)
{
    int i, n;
    xmsg("g_xdbg_ctbl=0x%08x, g_xdbg_ftbl=0x%08x\n", &g_xdbg_ctbl, &g_xdbg_ftbl);
    for (i = 0; i < XDBG_HASH_ROWS; i++)
    {
        n = g_xdbg_hash[i][1]-g_xdbg_hash[i][0];
        if (n) xmsg("hash 0x%02x n=%d\n", i, n);
    }
    xmsg("keywords=%d, hashes=%d\n", g_xdbg_keys, g_xdbg_hashes);

    xmsg("\t0x%08x - (default)\n", g_xdbg_ftbl[0].func);
    for (i = 1; i < g_xdbg_keys; i++)
    {
        xmsg("\t0x%08x - '%s'\n", g_xdbg_ftbl[i].func, g_xdbg_ftbl[i].keyword);
    }
}

