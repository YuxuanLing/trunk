/**********************************************************************************************************************
*	$Log: vsys#comm#inc#cbase.h_1,v $
*	Revision 1.21  2007-09-20 01:35:39-07  lsha
*	Fix xdbg & _fLOG
*
*	Revision 1.20  2007-09-20 00:12:15-07  lsha
*	Fix xdbg & _fLOG
*
*	Revision 1.19  2007-08-28 21:09:16-07  lsha
*	...No comments entered during checkin...
*
*	Revision 1.18  2007-08-27 18:50:54-07  lsha
*	Re-partition include files
*
*
*	DESCRIPTION:
*	Unified definitions of commonly used types, symbols, constants, and shared functions.
*
**********************************************************************************************************************/

#ifndef	CBASE
#define	CBASE						"           CBASE >>>    "
/**	CBASE
 */



/**	SECTION - symbols
 */
	#ifdef	INLINE
	#undef	INLINE
	#endif
		#if(defined(WIN32))
		#define	INLINE				static __forceinline

		#elif(defined(__ARMCC_VERSION))
	    #define INLINE				static __inline

		#else
		#define INLINE				static inline
		#endif

	#ifdef	NULL
	#undef	NULL
	#endif
		#define	NULL				(0)

	#ifndef	ptr32u
		#if(defined(WIN32))
		#ifndef	__OS_MSVC_CE__
		#define	$QPTR				qword ptr
		#define	$DPTR				dword ptr
		#define	$CNT				edx
		#define	MMX_WIN32
		#endif
		#define	ptr32u(ptr)			((UNSG32)PtrToUlong((char*)ptr))

		#elif(defined(__ARMCC_VERSION))
	    #define	ptr32u(ptr)			((UNSG32)(ptr))
	    #define	_unlink				remove
	    #define	_stricmp            strcmp
	    #define	_unlink				remove	

		#else
		#define	ptr32u(ptr)			((UNSG32)(ptr))
		#define	_stricmp			strcasecmp
		#define	_unlink				unlink
		#define	_open				open
		#define	_close				close
		#define	_read				read
		#define	_write				write
		#define	_tell				tell
		#define	_lseek				lseek
		#define	_lseeki64			lseeki64
		#endif
	#endif

	#ifndef	MemCpy
		#define	MemCpy				memcpy
	#endif

	#ifndef	MemSet
		#define	MemSet				memset
	#endif

/**	ENDOFSECTION
 */



#ifdef	__cplusplus
	extern	"C"
	{
#endif

/**	SECTION - pre-defined function arguments and returns
 */
	#ifndef	EFunc__
		#define	EFunc__
		typedef enum {

		ARG_KEEP					= - 32768,			/* No update, keep old value */
		ARG_AUTO					= - 32767,			/* Function chooses the value for it */

		} EFuncArgument;

		typedef enum {

		ERR_UNKNOWN					= - 1,				/* Error type not defined */
		ERR_POINTER					= - 2,				/* Invalid pointer */
		ERR_FILE					= - 3,				/* File access failure */
		ERR_FIFO					= - 4,				/* FIFO overflow or underflow */
		ERR_MEMORY					= - 5,				/* Not enough memory or allocation failure */
		ERR_MISMATCH				= - 6,				/* Mis-matches found in hand-shaking */
		ERR_PARAMETER 				= - 7,				/* False function parameter */
		ERR_HARDWARE				= - 8,				/* Hardware error */
		ERR_TIMING					= - 9,				/* Sychronize-related violation */

		SUCCESS						= 0

		} EFuncReturn;
		#ifndef	__CODE_LINK__
		static	char	*eperr[]	= {	"SUCCESS",
										"ERR_UNKNOWN",
										"ERR_POINTER",
										"ERR_FILE",
										"ERR_FIFO",
										"ERR_MEMORY",
										"ERR_MISMATCH",
										"ERR_PARAMETER",
										"ERR_HARDWARE",
										"ERR_TIMING",
											};
		#else
		#define	eperr				((char**)NULL)
		#endif
	#endif

/**	ENDOFSECTION
 */



/**	SECTION - unified 'xdbg(fmt, ...)' and '_LOG'/'_dftLOG'/'_fLOG' (masking enabled) functions
 */
	#ifndef	xdbg
	#ifdef	__CODE_LINK__
		#define	xdbg						1 ? 0 :
	#else
		#if(defined(__OS_GNUC_KERNEL__))
		#define	xdbg						printk

		#elif(defined(__OS_MSVC_KERNEL__))
		#define	xdbg						DbgPrint

		#elif(defined(__OS_MSVC_CE__))
		#define	xdbg						KITLOutputDebugString

		#elif(defined(__OS_MSVC_USER__))
		INLINE void _dbg(const char *fmt, va_list args)
											{	char str[1024]; vsprintf(str, fmt, args); OutputDebugString((LPCWSTR)str);
														}
		#else
		#define	_dbg						vprintf
		#endif
	#endif

		#ifdef	xdbg
		#define	fdbg						1 ? 0 :
		#define	_dflush(fp)					do{}while(0)

		/* Usage: _LOG(log_file, log_mask, item_bit, ("item = %d\n", item_value)) */
		#define	_LOG(fp, mask, b, fmt_args)	do{	xdbg fmt_args; }while(0)

		#else
		#define	DFTHFP						(errno)

		INLINE void _xdbg(const char *fmt, ...)
											{	va_list args;
												va_start(args, fmt);
												if(DFTHFP < 0x100) _dbg(fmt, args);
												else vfprintf((FILE*)DFTHFP, fmt, args);
												va_end(args);
														}
		INLINE void _fdbg(UNSG32 hfp, const char *fmt, ...)
											{	va_list args; if(!hfp) return;
												va_start(args, fmt); vfprintf((FILE*)hfp, fmt, args); va_end(args);
														}
		#define	xdbg						_xdbg
		#define	fdbg						_fdbg
		#define	_dflush(fp)					do{	if(fp) fflush((FILE*)fp); }while(0)

		/* Usage: _LOG(log_file, log_mask, item_bit, ("item = %d\n", item_value)) */
		#define	_LOG(fp, mask, b, fmt_args)	do{ if(((b) < 0) || (((mask) >> (b)) & 1)) {							\
												UNSG32 ctx = DFTHFP;												\
												DFTHFP = ptr32u((FILE*)fp); xdbg fmt_args; DFTHFP = ctx; }			\
														}while(0)
		#endif

		/* Usage: _fLOG(log_file, ("item = %d\n", item_value)) */
		#define	_fLOG(fp, fmt_args)			do{	_LOG(fp, 1, 0, fmt_args); _dflush(fp); }while(0)
	#endif



	#ifndef	fprintf
	#ifdef	__CODE_LINK__
	#define	fprintf					1 ? 0 :
	#else
	#define	fprintf					fprintf_
	static SIGN32 fprintf_(void *fp, const char *fmt, ...) {
			SIGN32 r = 0;
			if(fp != NULL) {
				va_list args; va_start(args, fmt);
				if(fp == (void*)stdout) r = vprintf(fmt, args);
				else r = vfprintf((FILE*)fp, fmt, args);
				va_end(args);
							}
							return r;
									}
													#endif
														#endif
	#ifndef	fflush
	#ifdef	__CODE_LINK__
	#define	fflush					1 ? 0 :
	#else
	#define	fflush(fp)				\
			(((fp != NULL) && ((void*)fp != (void*)stdout)) ? fflush((FILE*)fp) : 0)
													#endif
														#endif
/**	ENDOFSECTION
 */



/**	SECTION - unified 'IO32RD' and 'IO32WR' functions
 */
	#ifdef	__CUSTOMIZED_IO__
		void io32wr(UNSG32  d, UNSG32 a);
		void io32rd(UNSG32 *d, UNSG32 a);
		#define	IO32WR(d, a)				io32wr( (d), (a))
		#define	IO32RD(d, a)				io32rd(&(d), (a))
	#else
		#define	IO32WR(d, a)				do{ *(volatile UNSG32*)(a) = (d); } while(0)
		#define	IO32RD(d, a)				do{ (d) = *(volatile UNSG32*)(a); } while(0)
	#endif

		/* Directly write a 32b register or append to 'T64b cfgQ[]', in (adr,data) pairs */
		#define	IO32CFG(cfgQ, i, a, d)		do{	if(cfgQ) { (cfgQ)[i][0] = (a); (cfgQ)[i][1] = (d); }				\
												else IO32WR(d, a);													\
												(i) ++;																\
														}while(0)
/**	ENDOFSECTION
 */

#ifdef	__cplusplus
	}
#endif



/**	CBASE
 */
#endif

/**	ENDOFFILE: cbase.h ************************************************************************************************
 */

