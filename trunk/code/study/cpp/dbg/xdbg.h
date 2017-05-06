//---------------------------------------------------------------------------
//  Copyright 2008 Marvell Semiconductor Inc
//---------------------------------------------------------------------------
//! \file
//! \brief      An all-purpose debug utility (super printf).
//! \author     Hongjie Guan
//---------------------------------------------------------------------------

#ifndef XDBG_H_
#define XDBG_H_

void xdbg_sample_panic(int code);
int xdbg_printf(const char *format, ...);
int xdbg_bind(const char *key_str, void *func);
int xdbg_init(void);
void xdbg_dump(void);

#ifdef  xdbg
#undef  xdbg
#endif
#define xdbg xdbg_printf
//#define xdbg 1?0:

#ifdef  xmsg
#undef  xmsg
#endif
#define xmsg xdbg_printf

#ifdef  xerr
#undef  xerr
#endif
#define xerr xdbg_printf

#ifdef  xbug
#undef  xbug
#endif
#define xbug(c_) do{if(c_){xerr("@bug>>\npanic! (reason: %s)\n@ %s:%d\n\n",\
							(#c_),__FILE__,__LINE__); \
							xmsg("@vmeta>> finished (FAIL: exit abnormal)\n"); \
							exit(-1);}}while(0)


#endif

