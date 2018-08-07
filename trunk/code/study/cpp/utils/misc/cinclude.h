/**********************************************************************************************************************
*	$Log: vsys#comm#inc#cinclude.h_1,v $
*	Revision 1.16  2007-10-09 20:18:41-07  lsha
*	...No comments entered during checkin...
*
*	Revision 1.15  2007-09-20 01:35:38-07  lsha
*	Fix xdbg & _fLOG
*
*	Revision 1.14  2007-08-27 18:50:53-07  lsha
*	Re-partition include files
*
*
*	DESCRIPTION:
*	First header file to include by all C/C++ files.
*	Unified definitions of commonly used types, symbols, constants, and macros/functions.
*
*	Pre-definition build options:
*	__MAIN__			Must and only be defined at the beginning of the .c/.cpp file that contains main() function.
*	__CODE_LINK__		Optionally defined in make file:
*						"#define __CODE_LINK__ 0"
*							- Link for deep-embedded firmware. Will not link any library except customized "string.h".
*						"#define __CODE_LINK__ 1"
*							- Link for firmware on embedded OS. Will link customized "malloc.h" in addition.
*						"#ifndef __CODE_LINK__"
*							- Link for application software. Most standard libraries will be linked.
*	__big_endian__		Optionally defined in "_proj.h" for big-endian system only.
*
**********************************************************************************************************************/

#ifndef	CINCLUDE
#define	CINCLUDE					"        CINCLUDE >>>    "
/**	CINCLUDE
 */



/**	SECTION - platform dependent includes
 */

#ifndef	__CODE_LINK__
	#ifndef	_CRT_SECURE_NO_DEPRECATE
	#define	_CRT_SECURE_NO_DEPRECATE	/* To avoid CRT warnings */
	#endif

	#include	<stdio.h>
	#include	<stdlib.h>
	#include	<stdarg.h>
	#include	<string.h>
	#include	<math.h>
	#include	<time.h>
	#include	<assert.h>
	#include	<errno.h>

#else
	#if	__CODE_LINK__ > 0
	#include	"string.h"			/* Customized: memcpy, memset, and str... */

	#else
	#if	__CODE_LINK__ > 1
	#include	"malloc.h"			/* Customized: malloc, free */

	#endif
	#endif
#endif

#if(defined(WIN32))
	#include	<io.h>
	#include	<direct.h>

	#if(!defined(__XPLATFORM__))
	#include	<windows.h>
	#endif
		#define	__reversed_bit_fields__

	#if(defined(_TGTOS) && (_TGTOS == CE))
		#define	__OS_MSVC_CE__

	#elif(defined(DRIVER))
		#define	__OS_MSVC_KERNEL__

	#elif(defined(_CONSOLE) || defined(__XPLATFORM__))
		#define	__OS_MSVC_CONSOLE__

	#else
		#define	__OS_MSVC_USER__
	#endif

#elif(defined(__ARMCC_VERSION))

#elif(defined(__GNUC__))
	#include	<unistd.h>
	#include	<sys/time.h>
	#include	<sys/types.h>

	#if(defined(__KERNEL__))
		#define	__OS_GNUC_KERNEL__

	#else
		#define	__OS_GNUC_USER__
	#endif
#else
		#define	__OS_UNKNOWN__
#endif

	#include	"ctypes.h"
	#include	"cbase.h"
	#include	"cmacros.h"

/**	ENDOFSECTION
 */



/**	CINCLUDE
 */
#endif

/**	ENDOFFILE: cinclude.h *********************************************************************************************
 */

