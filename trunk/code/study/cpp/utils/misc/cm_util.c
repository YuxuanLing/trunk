/**********************************************************************************************************************
*	$Log: vsys#comm#cmodel#inc#src#cm_util.c_1,v $
*	Revision 1.8  2007-10-29 10:47:49-07  georgew
*	data dumping update
*
*	Revision 1.7  2007-05-25 16:19:00-07  yluo
*	...No comments entered during checkin...
*
*	Revision 1.6  2006-08-14 14:28:52-07  hguan
*	use dftLOG() instead of xdbg() in cm_util.c.
*
*	Revision 1.5  2006-08-01 21:58:51-07  hguan
*	Serializer: always create a new file for write even if the data size is 0
*
*	Revision 1.4  2006-05-31 14:41:49-07  lsha
*	...No comments entered during checkin...
*
*
*	DESCRIPTION:
*	C-model miscellanea utility functions.
*
**********************************************************************************************************************/

#include	"cm_util.h"



/**	SECTION - miscellaneous functions declared in "../cm_util.h" & "../mathops.h"
 */
	/******************************************************************************************************************
	*	Function: GreatCommDiv
	*	Prototype description: ../mathops.h
	******************************************************************************************************************/
	UNSG32	GreatCommDiv(
						SIGN32	s32a,
						SIGN32	s32b
						)
	{
	UNSG32	u32ab[2] = { ABS(s32a), ABS(s32b) }, i = (u32ab[0] < u32ab[1]), u32gcd;
	while(u32ab[i = 1 - i]) u32ab[1 - i] %= u32ab[i];

	u32gcd = u32ab[1 - i]; return  MAX(u32gcd, 1);
	/**	ENDOFFUNCTION: GreatCommDiv **/
	}



	/******************************************************************************************************************
	*	Function: LeastCommMtp
	*	Prototype description: ../mathops.h
	******************************************************************************************************************/
	UNSG32	LeastCommMtp(
						SIGN32	s32a,
						SIGN32	s32b
						)
	{
	UNSG32	u32gcd = GreatCommDiv(s32a, s32b);

	return ABS(s32a) / u32gcd * ABS(s32b);
	/**	ENDOFFUNCTION: LeastCommMtp **/
	}



	/******************************************************************************************************************
	*	Function: CoPrime
	*	Prototype description: ../mathops.h
	******************************************************************************************************************/
	void	CoPrime(	SIGN32	*ps32a,
						SIGN32	*ps32b
						)
	{
	UNSG32	u32gcd = GreatCommDiv(*ps32a, *ps32b);

	(*ps32a) /= u32gcd; (*ps32b) /= u32gcd;
	/**	ENDOFFUNCTION: CoPrime **/
	}

/**	ENDOFSECTION
 */



/**	SECTION - histogram functions
 */
#if(defined(__CODE_LINK__) && (__CODE_LINK__ == 0))
	#define	__histogram	0
#elif(!defined(__histogram_on__))
	#define	__histogram	256
#else
	#define	__histogram	(16 * KILO)
#endif

#if	__histogram > 0
	static	SIGN32	*arrps32tbl[__histogram];			/* Histogram tables */
#endif

	/******************************************************************************************************************
	*	Function: _HistogramNew
	*	Prototype description: ../cm_util.h
	******************************************************************************************************************/
	SIGN32*	_HistogramNew(
						SIGN32	s32entries
						)
	{
#if	__histogram > 0
	SIGN32	i;
	for(i = 0; i < __histogram; i ++)
		if(!arrps32tbl[i])
			return arrps32tbl[i] = (SIGN32*)calloc(s32entries, 4);
	xdbg(CM_UTIL "Create histogram table failed: no sufficient memory!\n");
#endif
	return NULL;
	/**	ENDOFFUNCTION: _HistogramNew **/
	}

/**	ENDOFSECTION
 */



#ifndef	__CODE_LINK__
/**	!__CODE_LINK__
 */

/**	SECTION - serialization functions
 */
	/******************************************************************************************************************
	*	DEFINITION - STRUCTURE SSerializer
	******************************************************************************************************************/
	typedef struct {

	SIGN32	s32size;									/* Object size in bytes */
	SIGN32	s32hist;									/* >=0 for read file history, -1 for write */	
	SIGN32	s32fidx;									/* Current file index */
	SIGN32	s32fpos;									/* Current file position */
	SIGN32	s32rest;									/* Rest object count in current group */
	SIGN32	s32path;									/* Index of object name (with path) */
	FILE	*fp;										/* File handle */

	} SSerializer;

	static	SSerializer	*arrhdl[KILO];					/* Maximum 1024 serializers supported */
	static	char	*arrps8path[KILO];					/* Maximum 1024 object-paths */
	static	SIGN32	arrs32dfidx[KILO];					/* Object-paths' delete-file-index */

	/******************************************************************************************************************
	*	Function: _SerializerOpen
	*	Prototype description: ../cm_util.h
	******************************************************************************************************************/
	SIGN32	_SerializerOpen(
						char	*ps8dir,
						char	*ps8obj,
						SIGN32	s32objSize,
						SIGN32	s32rdHistory
						)
	{
	SSerializer	*p;
	SIGN32	i, j, k;
	if(!ps8dir) return ERR_UNKNOWN;
	MergePath(garrs8, ps8dir, ps8obj);

	for(j = - 1, i = KILO - 1; i >= 0; i --)
		if(!arrps8path[i]) j = i;
		else if(!_stricmp(arrps8path[i], garrs8)) break;

	if(i >= 0)
		j = i;
	else if(j < 0) {
		xdbg(CM_UTIL "Serializer open failed: all object-paths are occupied!\n");
		return ERR_MEMORY;
	}
	else {
		arrps8path[j] = (char*)malloc(strlen(garrs8) + 1); strcpy(arrps8path[j], garrs8); arrs32dfidx[j] = 0;
	}

	for(k = - 1, i = KILO - 1; i >= 0; i --)
		if(!arrhdl[i]) k = i;
		else if((s32rdHistory < 0) && (arrhdl[i]->s32hist < 0) && (arrhdl[i]->s32path == j)) {
			xdbg(CM_UTIL "Serializer open failed: \"%s\" has been already opened for write!\n", garrs8);
			return ERR_FILE;
				}
	if(k < 0) {
		xdbg(CM_UTIL "Serializer open failed: all handles are occupied!\n");
		return ERR_MEMORY;
			}
	p = arrhdl[k] = (SSerializer*)malloc(sizeof(SSerializer)); p->s32path = j;
	p->s32size = s32objSize; p->s32hist = s32rdHistory; p->s32fidx = 0; p->s32fpos = 0; p->s32rest = 0; p->fp = NULL;

	return k;
	/**	ENDOFFUNCTION: _SerializerOpen **/
	}



	/******************************************************************************************************************
	*	Function: _SerializerWrite
	*	Prototype description: ../cm_util.h
	******************************************************************************************************************/
	SIGN32	_SerializerWrite(
						SIGN32	s32id,
						void	*pobj,
						SIGN32	s32num
						)
	{
	SSerializer	*p; if(s32id < 0) return s32id;
	p = arrhdl[s32id];
	if(p->s32hist >= 0) {
		xdbg(CM_UTIL "Serializer \"%s\" is not opened for write!\n", arrps8path[p->s32path]);
		return ERR_MISMATCH;
	}
	if(!p->fp) {
		sprintf(garrs8, "%s.%04d.hex", arrps8path[p->s32path], p->s32fidx % 10000);
		_dftLOG(0, 31, (CM_UTIL "Serializer \"%s\" opened for write...\n", garrs8));
		if(!(p->fp = _filemk(garrs8, "wb"))) {
			xdbg(CM_UTIL "Serializer \"%s\" opened for write failed!\n", garrs8);
			return ERR_FILE;
		}
	}
	if(s32num < 0) { if(p->fp) fclose(p->fp); p->fp = NULL; p->s32fpos = 0; p->s32fidx ++; }
	else {
		s32num *= p->s32size; fwrite(&s32num, 4, 1, p->fp); if(s32num) fwrite(pobj, s32num, 1, p->fp);
    fflush(p->fp);
	}
	return SUCCESS;
	/**	ENDOFFUNCTION: _SerializerWrite **/
	}



	/******************************************************************************************************************
	*	Function: _SerializerRead
	*	Prototype description: ../cm_util.h
	******************************************************************************************************************/
	SIGN32	_SerializerRead(
						SIGN32	s32id,
						void	*pobj,
						SIGN32	*pidx,
						SIGN32	*peof,
						SIGN32	s32num
						)
	{
	SSerializer	*p; if(s32id < 0) return s32id;
	p = arrhdl[s32id];
	if(p->s32hist < 0) {
		xdbg(CM_UTIL "Serializer \"%s\" is not opened for read!\n", arrps8path[p->s32path]);
		return ERR_PARAMETER;
			}
	if(!pobj) s32num = 0;
	else {
		SIGN32	i, j, k;
		if(!p->fp) {
			for(i = KILO - 1; i >= 0; i --)
				if(arrhdl[i] && (arrhdl[i]->s32hist >= 0) && arrhdl[i]->fp &&
					(arrhdl[i]->s32fidx == p->s32fidx) && (arrhdl[i]->s32path == p->s32path))
						break;
			sprintf(garrs8, "%s.%04d.hex", arrps8path[p->s32path], p->s32fidx % 10000);
			if(i >= 0) {
				xdbg(CM_UTIL "Serializer \"%s\" opend for read by #%d already...\n", garrs8, i);
				p->fp = arrhdl[i]->fp;
			}
			else {
				_dftLOG(0, 31, (CM_UTIL "Serializer \"%s\" opened for read...\n", garrs8));
				if(!(p->fp = _filemk(garrs8, "rb"))) {
					_dftLOG(0, 31, (CM_UTIL "Serializer \"%s\" opened for read failed!\n", garrs8));
					return ERR_FILE;
				}
			}
		}

		fseek(p->fp, 0, SEEK_END); j = ftell(p->fp);
		if(p->s32fpos >= j) s32num = 0;
		else {
			UNSG8	*pu8 = (UNSG8*)pobj; k = 0;
			fseek(p->fp, p->s32fpos, SEEK_SET);
			do {
			if(!p->s32rest) {
				fread(&i, 4, 1, p->fp);
				p->s32rest = i / p->s32size;
					}
			i = (s32num != 1) ? p->s32rest : MIN(1, p->s32rest);
			if(i) {
				fread(pu8, p->s32size, i, p->fp); p->s32rest -= i; pu8 += p->s32size * i; k += i;
					}
			if((p->s32fpos = ftell(p->fp)) >= j)
				break;
			} while(s32num < 0);
			s32num = k;
				}
		if(p->s32fpos >= j) {
			for(i = KILO - 1; i >= 0; i --)
				if((i != s32id) && arrhdl[i] && (arrhdl[i]->fp == p->fp))
					break;
			if(i < 0) fclose(p->fp); p->fp = NULL; p->s32fpos = 0; p->s32fidx ++; p->s32rest = 0;

			for(i = KILO - 1; i >= 0; i --) {
				if(i == 1)
					i = 1;
				if(arrhdl[i] && (arrhdl[i]->s32path == p->s32path) &&
					(!arrhdl[i]->s32hist ||
						((arrhdl[i]->s32hist > 0) &&
							(arrhdl[i]->s32fidx - arrhdl[i]->s32hist <= arrs32dfidx[p->s32path]))))
								break;
			}

			if(i < 0) {
				sprintf(garrs8, "%s.%04d.hex", arrps8path[p->s32path], arrs32dfidx[p->s32path] % 10000);
				_unlink(garrs8); arrs32dfidx[p->s32path] ++;
					}
				}
			}
	if(pidx) *pidx = (p->fp || !pobj) ? p->s32fidx : p->s32fidx - 1;
	if(peof) *peof = !p->fp;

	return s32num;
	/**	ENDOFFUNCTION: _SerializerRead **/
	}

/**	ENDOFSECTION
 */

/**	!__CODE_LINK__
 */
#endif



/**	SECTION - global init/free functions
 */
	UNSG8	*Clp255, *Clp127Add128;
	SIGN8	*Clp127, *Clp255Sub128;

	/* Clip table to 0~255 */
	UNSG8	Clp255u[512];

	/* Clip table to -128~127 */
	SIGN8	Clp127s[512];

	/******************************************************************************************************************
	*	Function: CmodelInit
	*	Prototype description: ../cm_util.h
	******************************************************************************************************************/
	SIGN32	CmodelInit()
	{
	SIGN32	i = 0;
	for(; i < 128; i ++) { Clp255u[i] = 0; Clp127s[i] = - 128; }
	for(; i < 384; i ++) { Clp255u[i] = (UNSG8)(i - 128); Clp127s[i] = (SIGN8)(i - 128); }
	for(; i < 512; i ++) { Clp255u[i] = 255; Clp127s[i] = 127; }
	Clp255 = Clp255u + 128; Clp127Add128 = Clp255u + 256; Clp127 = Clp127s + 256; Clp255Sub128 = Clp127s + 128;

#if	__histogram > 0
	for(i = 0; i < __histogram; i ++) arrps32tbl[i] = NULL;
#endif

#ifndef	__CODE_LINK__
	for(i = 0; i < KILO; i ++) {
		arrhdl[i] = NULL; arrps8path[i] = NULL; arrs32dfidx[i] = 0;
			}
	randSeed((UNSG32*)&i, 0);
#endif
	return i;
	/**	ENDOFFUNCTION: CmodelInit **/
	}



	/******************************************************************************************************************
	*	Function: CmodelFree
	*	Prototype description: ../cm_util.h
	******************************************************************************************************************/
	void	CmodelFree()
	{
	SIGN32	i = 0;
#if	__histogram > 0
	for(i = 0; i < __histogram; i ++) if(arrps32tbl[i]) free(arrps32tbl[i]);
#endif

#ifndef	__CODE_LINK__
	for(i = 0; i < KILO; i ++) {
		if(arrps8path[i]) free(arrps8path[i]); arrs32dfidx[i] = 0;
		if(arrhdl[i]) { if(arrhdl[i]->fp) fclose(arrhdl[i]->fp); free(arrhdl[i]); }
			}
#endif
	/**	ENDOFFUNCTION: CmodelFree **/
	}

/**	ENDOFSECTION
 */



/**	ENDOFFILE: cm_util.c **********************************************************************************************
 */

