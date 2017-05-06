/**********************************************************************************************************************
*	$Log: vsys#comm#inc#src#api.c_1,v $
*	Revision 1.11  2007-09-20 00:12:16-07  lsha
*	Fix xdbg & _fLOG
*
*	Revision 1.10  2007-08-27 18:50:56-07  lsha
*	Re-partition include files
*
*
*	DESCRIPTION:
*	This file provides common APIs (optimized for different platforms).
*
**********************************************************************************************************************/
#include	"cutil.h"



/**	SECTION - global temopral buffers
 */
	UNSG32	  garru32[KILO];
	SIGN32	 *garrs32  = (SIGN32 *)garru32;
	char	 *garrs8   = (( char *)garru32)+KILO*3;
	UNSG32	**garrpu32 = (UNSG32**)garru32;
	SIGN32	**garrps32 = (SIGN32**)garru32;
	char	**garrps8  = (  char**)garru32;

	UNSG32	dbghfp = NULL, dbgmask = ~0;

/**	ENDOFSECTION
 */



/**	SECTION - memory operations
 */
	/******************************************************************************************************************
	*	Function: _api_PckCpy
	*	Description: Software optimized 32B-packed (required at 8B aligned boundaries) memory copy.
	******************************************************************************************************************/
	INLINE	void	_api_PckCpy(
						void	*p,						/*!	Destination pointer !*/
						void	*psrc,					/*!	Source pointer !*/
						SIGN32	s32packs				/*!	Count of 32B packages to copy !*/
						)
	{
#ifdef	MMX_WIN32
	__asm
	{
			mov			$CNT,				s32packs
			cmp			$CNT,				0
			je			$quit

			mov			eax,				psrc
			mov			ebx,				p
			movq		mm0,				$QPTR [eax +   0]
			movq		mm1,				$QPTR [eax +   8]
			movq		mm2,				$QPTR [eax +  16]
			movq		mm3,				$QPTR [eax +  24]
	$cp32B:
			add			eax,				32
			sub			$CNT,				1
			je			$flush

			movq		$QPTR [ebx +   0],	mm0
			movq		mm0,				$QPTR [eax +   0]
			movq		$QPTR [ebx +   8],	mm1
			movq		mm1,				$QPTR [eax +   8]
			movq		$QPTR [ebx +  16],	mm2
			movq		mm2,				$QPTR [eax +  16]
			movq		$QPTR [ebx +  24],	mm3
			movq		mm3,				$QPTR [eax +  24]

			add			ebx,				32
			jmp			$cp32B
	$flush:
			movq		$QPTR [ebx +   0],	mm0
			movq		$QPTR [ebx +   8],	mm1
			movq		$QPTR [ebx +  16],	mm2
			movq		$QPTR [ebx +  24],	mm3
	$quit:
			emms
	}
#else
	SIGN32	i, *p32 = (SIGN32*)p, *p32src = (SIGN32*)psrc;
	for(i = s32packs; i > 0; i --) {
		p32[0] = p32src[0]; p32[1] = p32src[1]; p32[2] = p32src[2]; p32[3] = p32src[3];
		p32[4] = p32src[4]; p32[5] = p32src[5]; p32[6] = p32src[6]; p32[7] = p32src[7];
		p32 += 8; p32src += 8;
	}
#endif
	/**	ENDOFFUNCTION: _api_PckCpy **/
	}



	/******************************************************************************************************************
	*	Function: _api_PckSet
	*	Description: Software optimized 32B-packed (required at 8B aligned boundaries) memory set.
	******************************************************************************************************************/
	INLINE	void	_api_PckSet(
						void	*p,						/*!	Memory pointer (8B aligned) !*/
						SIGN32	s32v,					/*!	32b Value to be set !*/
						SIGN32	s32packs				/*!	Count (at least 1) of 32B packages to set !*/
						)
	{
#ifdef	MMX_WIN32
	SIGN32	arr32[2] = { s32v, s32v }, *p32 = (SIGN32 *)arr32;			/* Initiate 8B buffer */
	__asm
	{
			mov			$CNT,				s32packs
			cmp			$CNT,				0
			je			$quit

			mov			eax,				p32
			mov			ebx,				p
			movq		mm0,				$QPTR [eax]
	$wr32B:
			movq		$QPTR [ebx +   0],	mm0
			movq		$QPTR [ebx +   8],	mm0
			movq		$QPTR [ebx +  16],	mm0
			movq		$QPTR [ebx +  24],	mm0

			add			ebx,				32
			sub			$CNT,				1
			jne			$wr32B
	$quit:
			emms
	}
#else
	SIGN32	i, *p32 = (SIGN32*)p;
	for(i = s32packs; i > 0; i --) {
		p32[0] = p32[1] = p32[2] = p32[3] = p32[4] = p32[5] = p32[6] = p32[7] = s32v; p32 += 8;
	}
#endif
	/**	ENDOFFUNCTION: _api_PckSet **/
	}



	/******************************************************************************************************************
	*	DEFINITION - macro to distribute a trunk of memory to head bytes, tail bytes, well-aligned words and packet
	******************************************************************************************************************/
	#define	_api_MemPck(wordscl, packscl)				do{	SIGN32 mask = bSETMASK(wordscl) - 1;					\
															s32head = mask - ((ptr32u(p) - 1) & mask);				\
															if((s32 = s32bytes - s32head) <= mask) {				\
																s32head = s32bytes; s32tail = 0;					\
																s32word = s32pack = s32 = 0;						\
																	}												\
															else {													\
																s32word = s32 >> wordscl;							\
																s32pack = s32word >> packscl;						\
																s32word &= bSETMASK(packscl) - 1;					\
																s32tail = s32 & mask; s32 = wordscl;				\
																	}												\
																		}while(0)

	/******************************************************************************************************************
	*	Function: _api_MemCpy
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	void	_api_MemCpy(void	*p,
						void	*psrc,
						SIGN32	s32bytes
						)
	{
	SIGN32	s32head, s32word, s32pack, s32tail, s32, i;
	UNSG8	*p8 = (UNSG8*)p, *p8src = (UNSG8*)psrc;

#ifdef	MMX_WIN32
	_api_MemPck(3, 2);									/* Doing 32B packed MMX copy at destination 8-byte boundary */
#else
	s32 = ptr32u(p) - ptr32u(psrc);
	if(!(s32 & 3)) _api_MemPck(2, 3);									/* 4B x 8 packed copy */
	else if(!(s32 & 1)) _api_MemPck(1, 3);								/* 2B x 8 packed copy */
	else { s32head = s32bytes; s32word = s32pack = s32tail = s32 = 0; }	/* No packed copy */
#endif

	/* Copy bytes before aligned boundary */
	for(i = s32head; i > 0; i --) *p8 ++ = *p8src ++;

	if(s32) {
#ifdef	MMX_WIN32
		__asm
		{
				mov			ecx,				s32word
				cmp			ecx,				0
				je			$pack

				mov			eax,				p8src
				mov			ebx,				p8
		$cp8B:
				movq		mm0,				$QPTR [eax]
				movq		$QPTR [ebx],		mm0
				add			eax,				8
				add			ebx,				8
				sub			ecx,				1
				jne			$cp8B

				mov			p8src,				eax
				mov			p8,					ebx
		$pack:
				emms
		}

		_api_PckCpy(p8, p8src, s32pack); if(s32pack) { i = s32pack << 5; p8 += i; p8src += i; }
#else
		if(s32 == 2) {									/* 4B packed copy */
			UNSG32	*p32 = (UNSG32*)p8, *p32src = (UNSG32*)p8src;
			for(i = s32word; i > 0; i --) *p32 ++ = *p32src ++;

			_api_PckCpy(p32, p32src, s32pack); if(s32pack) { i = s32pack << 3; p32 += i; p32src += i; }
			p8 = (UNSG8*)p32; p8src = (UNSG8*)p32src;
		}
		else {											/* 2B packed copy */
			UNSG16	*p16 = (UNSG16*)p8, *p16src = (UNSG16*)p8src;
			for(i = s32word; i > 0; i --) *p16 ++ = *p16src ++;
			for(i = s32pack; i > 0; i --) {
				p16[0] = p16src[0]; p16[1] = p16src[1]; p16[2] = p16src[2]; p16[3] = p16src[3];
				p16[4] = p16src[4]; p16[5] = p16src[5]; p16[6] = p16src[6]; p16[7] = p16src[7];
				p16 += 8; p16src += 8;
			}

			p8 = (UNSG8*)p16; p8src = (UNSG8*)p16src;
		}
#endif
		/* Copy bytes after aligned boundary */
		for(i = s32tail; i > 0; i --) *p8 ++ = *p8src ++;
	}

	/**	ENDOFFUNCTION: _api_MemCpy **/
	}



	/******************************************************************************************************************
	*	Function: _api_MemSet
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	void	_api_MemSet(void	*p,
						SIGN32	s32v,
						SIGN32	s32unit,
						SIGN32	s32bytes
						)
	{
	UNSG32	*pu32, u32 = 0;
	UNSG16	*pu16, u16 = 0;
	UNSG8	*pu8, u8 = 0, *p8 = (UNSG8*)p;
	SIGN32	s32head, s32word, s32pack, s32tail, s32, i;

#ifdef	MMX_WIN32
	UNSG32	arr32[2];
#endif

	/* Initiate 8B buffer and move to starting point of packed set */
	if(s32v)
		if(s32unit == 4) u32 = (UNSG32)s32v;
		else {
			if(s32unit == 2) u16 = (UNSG16)s32v;
			else { u8 = (UNSG8)s32v; u16 = (u8 << 8) | u8; }
			u32 = (u16 << 16) | u16;
		}

#ifdef	MMX_WIN32
	_api_MemPck(3, 2);									/* Doing 32B packed MMX set at 8-byte boundary */
	arr32[0] = arr32[1] = u32;							/* Initiate starting 8B buffer of packed set */
#else
	_api_MemPck(2, 3);									/* Doing 32B packed set at 4-byte boundary */
#endif
	p8 += s32head;										/* Move to starting point of packed set */

	/* Set bytes before aligned boundary */
	i = 0;
	if(s32unit == 4) { s32head >>= 2; s32tail >>= 2; for(pu32 = (UNSG32*)p; i < s32head; i ++) pu32[i] = u32; }
	else if(s32unit == 2) { s32head >>= 1; s32tail >>= 1; for(pu16 = (UNSG16*)p; i < s32head; i ++) pu16[i] = u16; }
	else for(pu8 = (UNSG8*)p; i < s32head; i ++) pu8[i] = u8;

	if(s32) {
#ifdef	MMX_WIN32
		pu32 = (UNSG32 *)arr32;
		__asm
		{
				mov			ecx,				s32word
				mov			ebx,				p8
				cmp			ecx,				0
				je			$pack

				mov			eax,				pu32
				movq		mm0,				$QPTR [eax]
		$wr8B:
				movq		$QPTR [ebx],		mm0
				add			ebx,				8
				sub			ecx,				1
				jne			$wr8B
		$pack:
				mov			pu32,				ebx
				emms
		}
#else
		pu32 = (UNSG32*)p8;
		for(i = 0; i < s32word; i ++) *pu32 ++ = u32;
#endif
		_api_PckSet(pu32, u32, s32pack);

		/* Set bytes after aligned boundary */
		if(s32tail) {
			i = 0; pu32 += s32pack << 3;
			if(s32unit == 4) { do pu32[i ++] = u32; while(i < s32tail); }
			else if(s32unit == 2) { pu16 = (UNSG16*)pu32; do pu16[i ++] = u16; while(i < s32tail); }
			else { pu8 = (UNSG8*)pu32; do pu8[i ++] = u8; while(i < s32tail); }
		}
	}

	/**	ENDOFFUNCTION: _api_MemSet **/
	}

/**	ENDOFSECTION
 */



#ifndef	__CODE_LINK__
/**	__CODE_LINK__: all
 */

/**	SECTION - miscellanea utilities
 */
	/******************************************************************************************************************
	*	Function: _api_curSec
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	REAL64	_api_curSec(char	*ps8time
						)
	{
	REAL64 curSec = 0.;
	char *arrps8day[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	char *arrps8mon[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	struct tm *ptm; time_t curtm;

#if(defined(WIN32))
	const REAL64 CONSThih = 0.0000001 * 65536 * 65536, CONSTlow = 0.0000001;
	SYSTEMTIME curstime; FILETIME curftime; GetSystemTime(&curstime); SystemTimeToFileTime(&curstime, &curftime);
	curSec = CONSThih * curftime.dwHighDateTime + CONSTlow * curftime.dwLowDateTime;
#elif(defined(__ARMCC_VERSION)) //xingguo wang RVDS	

#elif(defined(__GNUC__))
	const REAL64 CONSThih = 1., CONSTlow = 0.000001;
	struct timeval tv; struct timezone tz; gettimeofday(&tv, &tz);
	curSec = CONSThih * tv.tv_sec + CONSTlow * tv.tv_usec;
#endif

	if(ps8time) {
		time(&curtm); ptm = localtime(&curtm);
		sprintf(ps8time, "%s %02d, %4d (%s) - %02d:%02d:%02d",
			arrps8mon[ptm->tm_mon], ptm->tm_mday, ptm->tm_year + 1900, arrps8day[ptm->tm_wday],
				ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
		}
	return curSec;
	/**	ENDOFFUNCTION: _api_curSec **/
	}



	/******************************************************************************************************************
	*	Function: _api_random
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	SIGN32	_api_random(UNSG32	*pu32s,
						SIGN32	s32bound
						)
	{
	static	UNSG32	u32s = 1;
	SIGN32	bNeg, s32rtn, s32abs = ABS(s32bound) + 1;
	if(!pu32s) pu32s = &u32s; if(s32bound == 0) *pu32s = (UNSG32)(_api_curSec(NULL) * kilo);

	*pu32s = *pu32s * 1103515245 + 12345; s32rtn = *pu32s & 0x7FFF0000; bNeg = bTST(*pu32s, 31);
	*pu32s = *pu32s * 1103515245 + 12345; s32rtn |= *pu32s >> 16; s32rtn %= s32abs;
	if((s32bound < 0) && bNeg) s32rtn = - s32rtn;

	u32s = *pu32s; return s32rtn;
	/**	ENDOFFUNCTION: _api_random **/
	}



	/******************************************************************************************************************
	*	Function: _api_rdmIdx
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	SIGN32	_api_rdmIdx(UNSG32	*pu32s,
						SIGN32	s32num,
						SIGN32	*ps32rate
						)
	{
	SIGN32	i, s32 = 0;
	for(i = 0; i < s32num; i ++) s32 += ps32rate[i];
	if(s32 > 0) {
		s32 = _api_random(pu32s, s32 - 1);
		for(i = 0; i < s32num; i ++) if((s32 -= ps32rate[i]) < 0) return i;
			}
	return - 1;
	/**	ENDOFFUNCTION: _api_rdmIdx **/
	}



	/******************************************************************************************************************
	*	Function: _api_rdmExp
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	SIGN32	_api_rdmExp(UNSG32	*pu32s,
						REAL64	r64scl,
						REAL64	r64base,
						SIGN32	s32expMax
						)
	{
	SIGN32	s32exp, s32b;
	REAL64	r64s = 0, r64abs = ABS(r64scl);
	if(!s32expMax) return _api_random(pu32s, ROUND(r64scl));
	else if(s32expMax < 0) { s32expMax = - s32expMax; r64base = 1. / r64base; }

	s32exp = _api_random(pu32s, s32expMax);
	while(s32exp -- > 0) { r64s += r64abs; r64abs *= r64base; }
	s32exp = ROUND(r64s); s32b = ROUND(r64s + r64abs) - s32exp - 1;
	if(s32b > 0) s32exp += _api_random(pu32s, s32b);
	if((r64scl < 0) && _api_random(pu32s, 1))
		s32exp = - s32exp;
	return s32exp;
	/**	ENDOFFUNCTION: _api_rdmExp **/
	}



	/******************************************************************************************************************
	*	Function: _api_rdmGma
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	SIGN32	_api_rdmGma(UNSG32	*pu32s,
						SIGN32	s32x,
						SIGN32	s32y,
						SIGN32	s32bound
						)
	{
	SIGN32	i, b = 0;
	REAL64	r64, r64base;
	if(s32x == s32y)
		return _api_random(pu32s, s32bound);
	else if(s32x > s32y) {
		b = 1; s32x = 100 - s32x; s32y = 100 - s32y;
			}
	r64base = pow(100. / s32x, 1. / (100 - s32y));
	if(ABS(r64base - 1) < 0.0001)
		return _api_random(pu32s, s32bound);

	for(i = 1, r64 = r64base; i < 100; i ++) r64 *= r64base; r64 = 1.01 * s32bound * (r64base - 1) / (r64 - 1);
	do i = _api_rdmExp(pu32s, r64, r64base, 99); while(ABS(i) > ABS(s32bound));
	if(b)
		if(i > 0) i = ABS(s32bound) - i;
		else if(i < 0) i = - ABS(s32bound) - i;
		else if((s32bound > 0) || _api_random(pu32s, 1)) i = s32bound;
		else i = - s32bound;
	return i;
	/**	ENDOFFUNCTION: _api_rdmGma **/
	}

/**	ENDOFSECTION
 */



/**	SECTION - string/file operations
 */
	/******************************************************************************************************************
	*	Function: StringPath
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	void	StringPath(	char	*ps8path,
						char	*ps8str
						)
	{
	SIGN32 i = strlen(ps8path) - 1;
	if(!ps8str) ps8str = ps8path; else if(ps8str != ps8path) strcpy(ps8str, ps8path);
	for(; i >= 0; i --)
		if((ps8str[i] == ' ') || (ps8str[i] == '.') || (ps8str[i] == '/') || (ps8str[i] == '\\'))
				ps8str[i] = '_';
	/**	ENDOFFUNCTION: StringPath **/
	}



	/******************************************************************************************************************
	*	Function: ModifyFileSuffix
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	void	ModifyFileSuffix(
						char	*ps8path,
						char	*ps8sfx
						)
	{
	SIGN32 i = strlen(ps8path) - 1;
	while((i >= 0) && (ps8path[i] != '.')) i --;

	if(i >= 0) ps8path[i] = 0; if(ps8sfx) strcat(ps8path, ps8sfx);
	/**	ENDOFFUNCTION: ModifyFileSuffix **/
	}



	/******************************************************************************************************************
	*	Function: MergePath
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	void	MergePath(	char	*ps8path,
						char	*ps8dir,
						char	*ps8filename
						)
	{
	SIGN32 i = strlen(ps8dir) - 1; strcpy(ps8path, ps8dir);
	if(!ps8filename) {
		if((i >= 0) && ((ps8dir[i] == '/') || (ps8dir[i] == '\\')))
			ps8path[i] = 0;
				}
	else {
		if((i >= 0) && ((ps8dir[i] != '/') && (ps8dir[i] != '\\')))
			strcat(ps8path, "/");
		strcat(ps8path, ps8filename);
				}
	/**	ENDOFFUNCTION: MergePath **/
	}



	/******************************************************************************************************************
	*	Function: SplitPath
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	void	SplitPath(	char	*ps8path,
						char	*ps8dir,
						char	*ps8filename
						)
	{
	SIGN32 i = strlen(ps8path) - 1;
	while((i >= 0) && (ps8path[i] != '/') && (ps8path[i] != '\\')) i --;

	if(ps8dir) { strcpy(ps8dir, ps8path); ps8dir[i + 1] = 0; }
	if(ps8filename) strcpy(ps8filename, ps8path + i + 1);
	/**	ENDOFFUNCTION: SplitPath **/
	}



	/******************************************************************************************************************
	*	Function: StringHeading
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	void	StringHeading(
						char	*ps8str,
						char	cpadding,
						SIGN32	s32alignment
						)
	{	
	char	arrs8[2], arrs8pad[256], *ps8;
	SIGN32	c, n = 0, i = 0;
	arrs8[0] = cpadding;
	arrs8[1] = NULL;
	c = (cpadding == '\t') ? 4 : 1;

	while(ps8str[i] != NULL)
		if(ps8str[i ++] == '\t') n = (n + 4) & ~3;
		else n ++;
	n = ABS32(s32alignment) * c - n;

	if(s32alignment >= 0) { i = CEILDIV(n, c); ps8 = ps8str; }
	else { i = n / c; ps8 = arrs8pad; *ps8 = NULL; }

	for(; i > 0; i --) strcat(ps8, arrs8);
	if(ps8 != ps8str) {
		for(i = n % c; i > 0; i --) strcat(ps8, " ");
		strcat(ps8, ps8str); strcpy(ps8str, ps8);
			}
	/**	ENDOFFUNCTION: StringHeading **/
	}



	/******************************************************************************************************************
	*	Function: _dirmk
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	void	_dirmk(		char	*ps8dir
						)
	{
	char arrs8dir[KILO]; MergePath(arrs8dir, ps8dir, NULL);

#if(defined(WIN32))
	if(_access(arrs8dir, 0)) _mkdir(arrs8dir);
#else
	sprintf(arrs8dir + 512, "mkdir -p %s", arrs8dir); system(arrs8dir + 512);
#endif
	/**	ENDOFFUNCTION: _dirmk **/
	}



	/******************************************************************************************************************
	*	Function: _filemk
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	FILE*	_filemk(	char	*ps8path,
						char	*ps8openmode
						)
	{
	FILE *fp = fopen(ps8path, ps8openmode);
	if(fp) return fp;
	else {
		char arrs8dir[KILO];
		SplitPath(ps8path, arrs8dir, NULL); _dirmk(arrs8dir); return fopen(ps8path, ps8openmode);
				}
	/**	ENDOFFUNCTION: _filemk **/
	}



	/******************************************************************************************************************
	*	Function: _comment
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	void	_comment(	char	*mark,
						FILE	*fp,
						SIGN32	s32tab,
						SIGN32	bHeadLine,
						char	*ps8comment
						)
	{
	char	c[2], s[2];
	SIGN32	i;
	for(i = 0; i < s32tab; i ++) fprintf(fp, "\t");
	c[0] = mark[1]; s[0] = mark[0]; c[1] = s[1] = NULL;

	if(ps8comment) {
		i = strlen(ps8comment) - 1;
		while((ps8comment[i] == '\n') || (ps8comment[i] == '\r'))
			ps8comment[i -- ] = 0;
		fprintf(fp, "%s\t%s\n", mark + 1, ps8comment);
	}
	else {
		if(bHeadLine) fprintf(fp, "%s%s%s", s, c, c);
		for(i = 29 - s32tab; i > 0; i --) fprintf(fp, "%s%s%s%s", c, c, c, c);
		if(!bHeadLine) fprintf(fp, "%s%s%s", c, c, s);
		fprintf(fp, "\n");
	}

	/**	ENDOFFUNCTION: _comment **/
	}

/**	ENDOFSECTION
 */

/**	__CODE_LINK__: all
 */
#endif



/**	ENDOFFILE: api.c **************************************************************************************************
 */

