/**********************************************************************************************************************
*	$Log: vsys#comm#cmodel#inc#cm_util.h_1,v $
*	Revision 1.6  2007-07-19 15:28:22-07  guanming
*	replace IO32RD and IO32WR by direct memory copy
*
*	Revision 1.5  2006-07-12 21:49:23-07  lsha
*	serveice handle changed to void*
*
*	Revision 1.4  2006-07-11 23:07:15-07  lsha
*	serveice handle added
*
*	Revision 1.3  2006-05-31 14:41:49-07  lsha
*	...No comments entered during checkin...
*
*
*	DESCRIPTION:
*	It includes macros and function declarations of useful c-model utilities.
*
*	Pre-definition build options:
*	__histogram_on__	Turn on histogram statistics for verification coverage purpose only.
*	__serializer_off__	Turn off serializer operations.
*
**********************************************************************************************************************/

#ifndef	CM_UTIL
#define	CM_UTIL						"         CM_UTIL >>>    "
/**	CM_UTIL
 */

#include	"cutil.h"



#ifdef	__cplusplus
	extern	"C"
	{
#endif

/**	SECTION - histogram, serialization, and other functions in "src/cm_util.c"
 */
	/******************************************************************************************************************
	*	Function: CmodelInit
	*	Description: Initiate global value, tables and random seed.
	*	Return:			SIGN32						-	Initialized random seed
	******************************************************************************************************************/
	SIGN32	CmodelInit();

	/******************************************************************************************************************
	*	Function: CmodelFree
	*	Description: Free allocated global resource.
	******************************************************************************************************************/
	void	CmodelFree();



	/******************************************************************************************************************
	*	Function: DftDma
	*	Description: Default simulator to perform a DMA transaction.
	*	Return:			SIGN32						-	Number of cycles used
	******************************************************************************************************************/
	INLINE	SIGN32	DftDma(
						void	*hdl,					/*!	Associated service handle !*/
						UNSG32	u32ep,					/*!	External 32b-word address !*/
						UNSG32	*pu32,					/*!	Internal 32b-word pointer !*/
						SIGN32	s32num					/*!	Absolute value as number of 32b-word to transfer,
															>0 to write from internal to external, or
															<0 to read from external to internal
															!*/
						)
	{
	SIGN32	rtn = ABS32(s32num);
	if(s32num > 0) for(; s32num > 0; s32num --, u32ep += 4, pu32 ++) *(UNSG32 *)u32ep = *pu32;
	else for(; s32num < 0; s32num ++, u32ep += 4, pu32 ++) *pu32 = *(UNSG32 *)u32ep;

	return rtn;
	/**	ENDOFFUNCTION: DftDma **/
	}



	/******************************************************************************************************************
	*	Histogram API: functional coverage meters
	******************************************************************************************************************/

	#define	_HistogramAdd(tbl, idx, n)					do{	if(tbl) ((SIGN32*)(tbl))[idx] += (n); }while(0)
	#define	_HistogramMin(tbl, idx, x)					do{	if(tbl) ((SIGN32*)(tbl))[idx]							\
																	= MIN(((SIGN32*)(tbl))[idx], x);				\
																		}while(0)
	#define	_HistogramMax(tbl, idx, x)					do{	if(tbl) ((SIGN32*)(tbl))[idx]							\
																	= MAX(((SIGN32*)(tbl))[idx], x);				\
																		}while(0)
#ifdef	__histogram_on__
	#define	HistogramNew								_HistogramNew
	#define	HistogramAdd								_HistogramAdd
	#define	HistogramMin								_HistogramMin
	#define	HistogramMax								_HistogramMax
#else
	#define	HistogramNew								1 ? 0 :
	#define	HistogramAdd								1 ? 0 :
	#define	HistogramMin								1 ? 0 :
	#define	HistogramMax								1 ? 0 :
#endif
	#define	HistogramTbl(tbl, idx)						do{	if(tbl) HistogramAdd(tbl, idx, 1); }while(0)
	#define	HistogramSat(tbl, x, min, max)				HistogramTbl(tbl, (x) < (min) ? 1 : ((x) > (max) ? 2 : 0))
	#define	HistogramInc(tbl)							HistogramTbl(tbl, 0)

	/******************************************************************************************************************
	*	Function: _HistogramNew
	*	Description: Create a histogram table.
	*	Return:			SIGN32*						-	>=0 for created histogram table, or
	*													NULL if not enough memory
	******************************************************************************************************************/
	SIGN32*	_HistogramNew(
						SIGN32	s32entries				/*!	>=1, entries of histogram table to create !*/
						);



	/******************************************************************************************************************
	*	Serializer API: signle-write multi-read probe channel for data packets
	******************************************************************************************************************/

#if(defined(__CODE_LINK__) || defined(__serializer_off__))
	#define	SerializerOpenWrite							1 ? 0 :
	#define	SerializerOpenRead							1 ? 0 :
	#define	SerializerWrite								1 ? 0 :
	#define	SerializerWriteEOF							1 ? 0 :
	#define	SerializerRead								1 ? 0 :
	#define	SerializerReadOne							1 ? 0 :
	#define	SerializerReadToEOF							1 ? 0 :
	#define	SerializerReadStatus						1 ? 0 :
#else
/**	!__CODE_LINK__ && !__serializer_off__
 */
	#define	SerializerOpenWrite(dir, obj, T)			_SerializerOpen(dir, obj, sizeof(T), - 1)
	#define	SerializerOpenRead(dir, obj, T, fn)			_SerializerOpen(dir, obj, sizeof(T), fn)
	#define	SerializerWrite(hdl, p, num)				_SerializerWrite(hdl, p, num)
	#define	SerializerWriteEOF(hdl)						_SerializerWrite(hdl, NULL, - 1)
	#define	SerializerRead(hdl, p)						_SerializerRead(hdl, p, NULL, NULL, 0)
	#define	SerializerReadOne(hdl, p)					_SerializerRead(hdl, p, NULL, NULL, 1)
	#define	SerializerReadToEOF(hdl, p)					_SerializerRead(hdl, p, NULL, NULL, - 1)
	#define	SerializerReadStatus(hdl, idx, eof)			_SerializerRead(hdl, NULL, idx, eof, - 1)

	/******************************************************************************************************************
	*	Function: _SerializerOpen
	*	Description: Open a serializer for read/write.
	*	Return:			SIGN32						-	>=0 for opened serializer handle ID, or
	*													ERR_FILE if trying to open for write twice, or
	*													ERR_MEMORY if not enough memory, or
	*													ERR_UNKNOWN if no serialization path (ps8dir == NULL) given
	******************************************************************************************************************/
	SIGN32	_SerializerOpen(
						char	*ps8dir,				/*!	Serialization path !*/
						char	*ps8obj,				/*!	Object name !*/
						SIGN32	s32objSize,				/*!	Object size in bytes for write !*/
						SIGN32	s32rdHistory			/*!	-1 to open for write, or
															>0 for number of recent read history files to keep, or
															0 to keep all history files for read
															!*/
						);

	/******************************************************************************************************************
	*	Function: _SerializerWrite
	*	Description: Serializer to write a group of objects.
	*	Return:			SIGN32						-	SUCCESS, or
	*													ERR_FILE if file access failed, or
	*													ERR_MISMATCH if serializer is not opend for write
	******************************************************************************************************************/
	SIGN32	_SerializerWrite(
						SIGN32	s32id,					/*!	Serializer handle ID !*/
						void	*pobj,					/*!	Objects to write !*/
						SIGN32	s32num					/*!	Number objects to write in group,
															-1 to write nothing but EOF
															!*/
						);

	/******************************************************************************************************************
	*	Function: _SerializerRead
	*	Description: Serializer to read a object or a group of objects or all remaining in current file.
	*	Return:			SIGN32						-	>= 0 for number of objects read, or
	*													ERR_FILE if file access failed or under-flow happened, or
	*													ERR_PARAMETER if serializer is not opend for read
	******************************************************************************************************************/
	SIGN32	_SerializerRead(
						SIGN32	s32id,					/*!	Serializer handle ID !*/
						void	*pobj,					/*!	Non-zero buffer to received objects !*/
						SIGN32	*pidx,					/*!	Non-zero to receive current file index !*/
						SIGN32	*peof,					/*!	Non-zero to receive if EOF hit !*/
						SIGN32	s32num					/*!	0 to read out current group, or
															1 to read one object, or
															-1 to read out all remaining in current file
															!*/
						);
/**	!__CODE_LINK__ && !__serializer_off__
 */
#endif

/**	ENDOFSECTION
 */

#ifdef	__cplusplus
	}
#endif



#include	"multimedia.h"



#ifdef	__cplusplus
/**	__cplusplus
 */

/**	SECTION - template utility functions
 */
	/******************************************************************************************************************
	*	Function: _MismatchCheck
	*	Description: Log and return first mismatch (type word) position of 2 pointers of any type.
	*	Return:			SIGN32						-	>= 0 for first mismatch word index, or
	*													-1 if identical
	******************************************************************************************************************/
	template <class T>
	SIGN32	_MismatchCheck(
						void	*pa,					/*!	1st pointer to compare !*/
						void	*pb,					/*!	2nd pointer to compare !*/
						SIGN32	s32w,					/*!	Number of items to compare, 0 indicates non-array !*/
						char	*ps8pfx,				/*!	Message prefix to log, "" to disable prefix output !*/
						UNSG32	hfpErrLOG				/*!	Error log file handle !*/
						)
	{
	if(ps8pfx) {
		SIGN32	i, w = MAX(s32w, 1);
		T *pta = (T*)pa, *ptb = (T*)pb;
		for(i = 0; i < w; i ++)
			if(pta[i] != ptb[i]) {
				_fLOG(hfpErrLOG, ("%s### mismatch found ###    ", ps8pfx));
				_iout<T>(pta[i], hfpErrLOG); _fLOG(hfpErrLOG, (" != ")); _iout<T>(ptb[i], hfpErrLOG);
				if(s32w > 0) _fLOG(hfpErrLOG, (" @ [%d]", i));
				_fLOG(hfpErrLOG, ("\n"));
				return i;
			}
		}
	return - 1;
	/**	ENDOFFUNCTION: _MismatchCheck **/
	}



	/******************************************************************************************************************
	*	Function: _EnTileScan
	*	Description: Convert raster scan buffer to tiling buffer.
	******************************************************************************************************************/
	template <class T>
	void	_EnTileScan(SIGN32	s32h,					/*!	Input horizontal size !*/
						SIGN32	s32v,					/*!	Input vertical size !*/
						SIGN32	s32pitch,				/*!	Input horizontal pitch !*/
						SIGN32	b8x8,					/*!	1 to turn on 8x8 tiling !*/
						SIGN32	b4x4,					/*!	1 to turn on 4x4 tiling !*/
						T		*pa,					/*!	Input buffer !*/
						T		*pb,					/*!	Output buffer !*/
						T		*p						/*!	Temporary buffer !*/
								= NULL
						)
	{
	SIGN32	i, j, t, h, v, m, n, k = 1;
	if(b8x8) {			/* pa->pb/p */
		h = (s32h + 7) >> 3; v = (s32v + 7) >> 3; n = h * 64;
		if(!b4x4) p = pb;
		for(i = 0; i < s32v; i ++, pa += s32pitch) {
			m = (i >> 3) * n + (i & 7) * 8;
			for(j = 0; j < s32h; j ++) p[m + (j >> 3) * 64 + (j & 7)] = pa[j];
				}
		k = h * v; s32h = s32v = s32pitch = 8;
		}
	if(b4x4) {			/* pa/p->pb */
		h = (s32h + 3) >> 2; v = (s32v + 3) >> 2; n = h * 16;
		if(!b8x8) p = pa;
		for(t = 0; t < k; t ++, pb += n * v)
			for(i = 0; i < s32v; i ++, p += s32pitch) {
				m = (i >> 2) * n + (i & 3) * 4;
				for(j = 0; j < s32h; j ++) pb[m + (j >> 2) * 16 + (j & 3)] = p[j];
					}
		}
	/**	ENDOFFUNCTION: _EnTileScan **/
	}

	/******************************************************************************************************************
	*	Function: _DeTileScan
	*	Description: Convert tiling buffer to raster scan buffer.
	******************************************************************************************************************/
	template <class T>
	void	_DeTileScan(SIGN32	s32h,					/*!	Output horizontal size !*/
						SIGN32	s32v,					/*!	Output vertical size !*/
						SIGN32	s32pitch,				/*!	Output horizontal pitch !*/
						SIGN32	b8x8,					/*!	1 to turn on 8x8 de-tiling !*/
						SIGN32	b4x4,					/*!	1 to turn on 4x4 de-tiling !*/
						T		*pa,					/*!	Output buffer !*/
						T		*pb,					/*!	Input buffer !*/
						T		*p						/*!	Temporary buffer !*/
								= NULL
						)
	{
	SIGN32	i, j, t, h, v, m, n, k = 1, x = s32h, y = s32v, s = s32pitch;
	T *pt = p;
	if(b4x4) {			/* pb->pa/p */
		if(!b8x8) {
			pt = pa; h = (s32h + 3) >> 2; v = (s32v + 3) >> 2;
				}
		else {
			h = v = 2; x = y = s = 8; k = ((s32h + 7) >> 3) * ((s32v + 7) >> 3);
				}
		n = h * 16;
		for(t = 0; t < k; t ++, pb += n * v)
			for(i = 0; i < y; i ++, pt += s) {
				m = (i >> 2) * n + (i & 3) * 4;
				for(j = 0; j < x; j ++) pt[j] = pb[m + (j >> 2) * 16 + (j & 3)];
					}
		}
	if(b8x8) {			/* pb/p->pa */
		h = (s32h + 7) >> 3; v = (s32v + 7) >> 3; n = h * 64;
		if(!b4x4) p = pb;
		for(i = 0; i < s32v; i ++, pa += s32pitch) {
			m = (i >> 3) * n + (i & 7) * 8;
			for(j = 0; j < s32h; j ++) pa[j] = p[m + (j >> 3) * 64 + (j & 7)];
				}
		}
	/**	ENDOFFUNCTION: _DeTileScan **/
	}

/**	ENDOFSECTION
 */



#if(!defined(__CODE_LINK__))
/**	!__CODE_LINK__
 */

/**	SECTION - WIN32 functions in "src/mfio_win.cpp"
 */
	/******************************************************************************************************************
	*	Common used macros
	******************************************************************************************************************/
	#define	dibPitch(grey, cols)						((((grey) ? (cols) : (cols) * 3) + 3) & ~3)
	#define	dibImageSize(grey, cols, rows)				(dibPitch(grey, cols) * (rows))

	/******************************************************************************************************************
	*	DEFINITION - INTERFACE IMediaFileIO
	******************************************************************************************************************/
	class IMediaFileIO {

	public:

	/******************************************************************************************************************
	*	Function: IMediaFileIO::LoadBMP
	*	Description: Load a BMP file.
	*	Return:			SIGN32						-	SUCCESS or
	*													ERR_FILE if file open failed or
	*													ERR_UNKNOWN if file content ruined
	******************************************************************************************************************/
	static	SIGN32	LoadBMP(
						char	*ps8FileName,			/*!	BMP file name !*/
						SIGN32	*ps32cols,				/*!	Non-zero to get picture horizontal size !*/
						SIGN32	*ps32rows,				/*!	Non-zero to get picture vertical size !*/
						UNSG8	**ppu8BMP,				/*!	For picture data !*/
						SIGN32	*ps32ClrBitCount		/*!	Non-zero to get picture color bit count (8/24) !*/
						);

	/******************************************************************************************************************
	*	Function: IMediaFileIO::SaveBMP
	*	Description: Save a BMP file.
	*	Return:			SIGN32						-	SUCCESS or
	*													ERR_FILE if file open failed or
	*													ERR_POINTER if return data pointer 'pu8BMP' is NULL
	******************************************************************************************************************/
	static	SIGN32	SaveBMP(
						char	*ps8FileName,			/*!	BMP file name !*/
						SIGN32	s32cols,				/*!	Picture horizontal size !*/
						SIGN32	s32rows,				/*!	Picture vertical size !*/
						UNSG8	*pu8BMP,				/*!	Picture data !*/
						SIGN32	s32ClrBitCount			/*!	Picture color bit count (8/24) !*/
								= 24
						);

	/******************************************************************************************************************
	*	Function: IMediaFileIO::CreateInstance
	*	Description: Create CMediaFileIO instance based on media file name.
	*	Return:			SIGN32						-	SUCCESS or
	*													ERR_POINTER if 'pp' is NULL or
	*													ERR_UNKNOWN if file type is not supported or
	*													ERR_PARAMETER if media file open/initialization failed
	******************************************************************************************************************/
	static	SIGN32	CreateInstance(
						IMediaFileIO	**pp,			/*!	For returning the instance of interface IMediaFileIO !*/
						char	*ps8FileName,			/*!	Name of media file (*.bmp/avi) !*/
						REAL64	*pf64fps,				/*!	To get/set picture rate (frame/field per second) !*/
						SIGN32	*ps32cols,				/*!	To get/set picture horizontal size
															Must not be NULL for write mode
															!*/
						SIGN32	*ps32rows,				/*!	To get/set picture vertical size
															Must not be NULL for write mode
															!*/
						SIGN32	**ppVideoInfo,			/*!	To get/set video stream information !*/
						SIGN32	**ppAudioInfo,			/*!	To get/set audio stream information
															NULL in write mode means no audio
															!*/
						SIGN32	*ps32compression,		/*!	To get/set video compression
															NULL in write mode means no video
															!*/
						SIGN32	*ps32ReadPictureCount	/*!	NULL to set write mode or
															Non-zero pointer to set read mode and for input/output
																input: count of pictures to read
																return: maximum pictures available in this file to read
															!*/
								= NULL
						);

	/******************************************************************************************************************
	*	Function: IMediaFileIO::CreateInstance
	*	Description: Create CMediaFileIO instance based on configuration file.
	*	Return:			SIGN32						-	SUCCESS or
	*													ERR_POINTER if 'pp' is NULL or
	*													ERR_FILE if configuration file error or
	*													ERR_UNKNOWN if file type is not supported or
	*													ERR_PARAMETER if media file open/initialization failed
	******************************************************************************************************************/
	static	SIGN32	CreateInstance(
						IMediaFileIO	**pp,			/*!	For returning the instance of interface IMediaFileIO !*/
						FILE	*fpCFG,					/*!	Configuration file handle !*/
						char	*ps8FileName,			/*!	Return the name of media file (*.bmp/avi)
															Null is allowed
															!*/
						REAL64	*pf64fps,				/*!	To get/set picture rate (frame/field per second) !*/
						SIGN32	*ps32cols,				/*!	To get/set picture horizontal size
															Must not be NULL for write mode
															!*/
						SIGN32	*ps32rows,				/*!	To get/set picture vertical size
															Must not be NULL for write mode
															!*/
						SIGN32	**ppVideoInfo,			/*!	To get/set video stream information !*/
						SIGN32	**ppAudioInfo,			/*!	To get/set audio stream information
															NULL in write mode means no audio
															!*/
						SIGN32	*ps32compression,		/*!	To get/set video compression
															NULL in write mode means no video
															!*/
						SIGN32	*ps32ReadPictureCount	/*!	NULL to set write mode or
															Non-zero pointer to set read mode and for input/output
																input: count of pictures to read
																return: maximum pictures available in this file to read
															!*/
								= NULL
						);

	/******************************************************************************************************************
	*	Function: IMediaFileIO::Release
	*	Description: Release a CMediaFile instance.
	******************************************************************************************************************/
	virtual	void	Release(
						) = 0;

	/******************************************************************************************************************
	*	Function: IMediaFileIO::Load
	*	Description: Load an audio frame or a video picture from an opened media file.
	*	Return:			SIGN32						-	Number of bytes read
	*													0 means reading failed
	******************************************************************************************************************/
	virtual	SIGN32	Load(
						SIGN8	s8v,					/*!	0 for audio or 1 for video !*/
						UNSG8	*pu8Data,				/*!	For returning audio/video data !*/
						SIGN32	s32index,				/*!	Frame index !*/
						char	*ps8SaveBMP				/*!	For read raw RGB24 video only
															Non-zero BMP file name to store current picture
															!*/
								= NULL
						) = 0;

	/******************************************************************************************************************
	*	Function: IMediaFileIO::Save
	*	Description: Save an audio frame or a video picture from an opened media file.
	*	Return:			SIGN32						-	SUCCESS or
	*													Negative error code if writing failed
	******************************************************************************************************************/
	virtual	SIGN32	Save(
						SIGN8	s8v,					/*!	0 for audio or 1 for video !*/
						UNSG8	*pu8Data,				/*!	Pointer to audio/video data to be written !*/
						SIGN32	s32index,				/*!	Frame index !*/
						SIGN32	s32DataSize,			/*!	Number of bytes to write !*/
						SIGN32	dwFlags					/*!	Please refer 'AVIStreamWrite'
															'ARG_AUTO' will set it to AVIIF_KEYFRAME
															!*/
								= ARG_AUTO
						) = 0;

	/**	ENDOFDEF: CLASS IMediaFileIO **/
	};

/**	ENDOFSECTION
 */

/**	!__CODE_LINK__
 */
#endif

/**	__cplusplus
 */
#endif



/**	CM_UTIL
 */
#endif

/**	ENDOFFILE: cm_util.h **********************************************************************************************
 */

