/**********************************************************************************************************************
*	$Log: vsys#comm#cmodel#inc#src#mfio.cpp_1,v $
*	Revision 1.4  2007-05-30 13:43:51-07  lsha
*	Initial revision.
*
*	Revision 1.3  2006-05-31 14:41:49-07  lsha
*	...No comments entered during checkin...
*
*
*	DESCRIPTION:
*	Media file (bmp/avi/yuv) access (r/w) APIs.
*
**********************************************************************************************************************/

#if(defined(__cplusplus) && !defined(__CODE_LINK__))
/**	__cplusplus && !__CODE_LINK__
 */

#include	"cm_util.h"



/**	SECTION - for internal use
 */
	#pragma	pack(1)

	/* Replacement of 'BITMAPINFOHEADER' */
	typedef struct {

	UNSG32	biSize;										/* DWORD */
	SIGN32	biWidth;									/* LONG */
	SIGN32	biHeight;									/* LONG */
	UNSG16	biPlanes;									/* WORD */
	UNSG16	biBitCount;									/* WORD */
	UNSG32	biCompression;								/* DWORD */
	UNSG32	biSizeImage;								/* DWORD */
	SIGN32	biXPelsPerMeter;							/* LONG */
	SIGN32	biYPelsPerMeter;							/* LONG */
	UNSG32	biClrUsed;									/* DWORD */
	UNSG32	biClrImportant;								/* DWORD */

	} SbmpInfoHeader;

	/* Replacement of 'BITMAPFILEHEADER' */
	typedef struct {

	UNSG16	bfType;										/* WORD */
	UNSG32	bfSize;										/* DWORD */
	UNSG16	bfReserved1;								/* WORD */
	UNSG16	bfReserved2;								/* WORD */
	UNSG32	bfOffBits;									/* DWORD */

	} SbmpFileHeader;

	/* Replacement of 'RGBQUAD' */
	typedef struct {

	UNSG8	rgbBlue;									/* BYTE */
	UNSG8	rgbGreen;									/* BYTE */
	UNSG8	rgbRed;										/* BYTE */
	UNSG8	rgbReserved;								/* BYTE */

	} SrgbQuad;

	#pragma	pack()

	/* Default media file formats */
	typedef enum {

	__media_bmp						= 0,				/* BMP */
	__media_avi						= 1,				/* AVI */
	__media_yuv						= 2,				/* YUV */

	__media_err						= - 1

	} EMediaFileType;

/**	ENDOFSECTION
 */



/**	SECTION - definition of CLASS CMediaFileIO
 */
	/******************************************************************************************************************
	*	DEFINITION - CLASS CMediaFileIO
	******************************************************************************************************************/
	class CMediaFileIO : public IMediaFileIO {

	public:

	SIGN32				s32mode;						/*!	See 'EMediaFileType' !*/

	/******************************************************************************************************************
	*	Function: CMediaFileIO::CreateInstance
	*	Prototype description: ../_include/cmodel.h (please refer 'IMediaFileIO::CreateInstance')
	******************************************************************************************************************/
	static	SIGN32	CreateInstance(
						SIGN32	s32mode,
						IMediaFileIO	**pp,
						char	*ps8FileName,
						REAL64	*pf64fps,
						SIGN32	*ps32cols,
						SIGN32	*ps32rows,
						SIGN32	**ppVideoInfo,
						SIGN32	**ppAudioInfo,
						SIGN32	*ps32compression,
						SIGN32	*ps32ReadPictureCount
						);

	/******************************************************************************************************************
	*	Function: CMediaFileIO::Release
	*	Prototype description: ../_include/cmodel.h
	******************************************************************************************************************/
	void	Release();

	/******************************************************************************************************************
	*	Function: CMediaFileIO::Init
	*	Prototype description: ../_include/cmodel.h (please refer 'IMediaFileIO::CreateInstance')
	******************************************************************************************************************/
	virtual	SIGN32	Init(
						char	*ps8FileName,
						REAL64	*pf64fps,
						SIGN32	*ps32cols,
						SIGN32	*ps32rows,
						SIGN32	**ppVideoInfo,
						SIGN32	**ppAudioInfo,
						SIGN32	*ps32compression,
						SIGN32	*ps32ReadPictureCount
						) = 0;

	/**	ENDOFDEF: CLASS CMediaFileIO **/
	};

/**	ENDOFSECTION
 */



/**	SECTION - definition of CLASS CBMPFileIO
 */
	/******************************************************************************************************************
	*	DEFINITION - CLASS CBMPFileIO
	******************************************************************************************************************/
	class CBMPFileIO : public CMediaFileIO {

	char	arrs8FileName[KILO];
	char	arrs8prefix[KILO];
	char	arrs8suffix[16];
	char	*ps8suffix;

	/******************************************************************************************************************
	*	Function: CBMPFileIO::SetFileName
	*	Description: Set BMP file name based on input index.
	******************************************************************************************************************/
	void	SetFileName(SIGN32	s32index				/*!	BMP file name suffix number !*/
						)
	{
	sprintf(arrs8suffix, "%08d.bmp", s32index);
	strcpy(arrs8FileName, arrs8prefix); strcat(arrs8FileName, ps8suffix);
	/**	ENDOFFUNCTION: CBMPFileIO::SetFileName **/
	}



	public:

	SIGN32	s32ClrBitCount;								/* 8 for grey scale and 24 for RGB */
	SIGN32	s32cols, s32rows;							/* BMP resolution */

	/******************************************************************************************************************
	*	Function: BMPFileIO::Init
	*	Prototype description: ../_include/cmodel.h (please refer 'IMediaFileIO::CreateInstance')
	******************************************************************************************************************/
	SIGN32	Init(		char	*ps8FileName,
						REAL64	*pf64fps,
						SIGN32	*ps32cols,
						SIGN32	*ps32rows,
						SIGN32	**ppVideoInfo,
						SIGN32	**ppAudioInfo,
						SIGN32	*ps32compression,
						SIGN32	*ps32ReadPictureCount
						)
	{
	if(pf64fps) *pf64fps = 0;
	if(ppVideoInfo) *ppVideoInfo = 0;
	if(ppAudioInfo) *ppAudioInfo = 0;
	if(ps32compression) *ps32compression = 0;

	/* Up-to 8 '?' supported in file name */
	ps8suffix = arrs8suffix + 8;

	/* Make a copy */
	SIGN32	i = (SIGN32)strlen(ps8FileName) - 5;
	ps8FileName[i + 1] = NULL; strcpy(arrs8prefix, ps8FileName);

	/* Get prefix from "Prefix???.BMP" */
	for(; i >= 0 ; i --) {
		if(ps8FileName[i] != '?')
			break;

		/* Adjust file name prefix and suffix positions */
		ps8suffix --; arrs8prefix[i] = NULL;
	}

	if(!ps32ReadPictureCount) {
		s32cols = *ps32cols; s32rows = *ps32rows; s32ClrBitCount = 24;
		return SUCCESS;
	}
	else {
		SetFileName(0);
		return LoadBMP(arrs8FileName, ps32cols, ps32rows, NULL, &s32ClrBitCount);
	}

	/**	ENDOFFUNCTION: CBMPFileIO::Init **/
	}



	/******************************************************************************************************************
	*	Function: CBMPFileIO::Load
	*	Prototype description: ../_include/cmodel.h
	******************************************************************************************************************/
	SIGN32	Load(		SIGN8	s8v,
						UNSG8	*pu8Data,
						SIGN32	s32index,
						char	*ps8SaveBMP
						)
	{
	/* Prepare BMP file name */
	SetFileName(s32index);

	s32cols = 0; s32rows = 0; s32ClrBitCount = 24;
	if(LoadBMP(arrs8FileName, &s32cols, &s32rows, &pu8Data, &s32ClrBitCount) == SUCCESS)
		if(ps8SaveBMP)
			SaveBMP(ps8SaveBMP, s32cols, s32rows, pu8Data, s32ClrBitCount);

	return dibImageSize(s32ClrBitCount == 8, s32cols, s32rows);
	/**	ENDOFFUNCTION: CBMPFileIO::Load **/
	}



	/******************************************************************************************************************
	*	Function: CBMPFileIO::Save
	*	Prototype description: ../_include/cmodel.h
	******************************************************************************************************************/
	SIGN32	Save(		SIGN8	s8v,
						UNSG8	*pu8Data,
						SIGN32	s32index,
						SIGN32	s32DataSize,
						SIGN32	dwFlags
						)
	{
	/* Prepare BMP file name */
	SetFileName(s32index);

	return SaveBMP(arrs8FileName, s32cols, s32rows, pu8Data, s32ClrBitCount);
	/**	ENDOFFUNCTION: CBMPFileIO::Save **/
	}

	/**	ENDOFDEF: CLASS CBMPFileIO **/
	};

/**	ENDOFSECTION
 */



#if(!defined(WIN32))
	#define	CAVIFileIO	CBMPFileIO
#else
/**	WIN32
 */

#include	<vfw.h>
#pragma	comment(lib, "vfw32.lib")

/**	SECTION - definition of CLASS CAVIFileIO
 */
	/******************************************************************************************************************
	*	DEFINITION - CLASS CAVIFileIO
	******************************************************************************************************************/
	class CAVIFileIO : public CMediaFileIO {

	public:

	PAVIFILE			paviFile;						/* AVI file handle */
	PAVISTREAM			pavi_video, pavi_audio;			/* AVI video/audio stream handles */
	AVISTREAMINFOA		avi_vInfo, avi_aInfo;			/* AVI video/audio information */

	/******************************************************************************************************************
	*	Function: CAVIFileIO::Init
	*	Prototype description: ../_include/cmodel.h (please refer 'IMediaFileIO::CreateInstance')
	******************************************************************************************************************/
	SIGN32	Init(		char	*ps8FileName,
						REAL64	*pf64fps,
						SIGN32	*ps32cols,
						SIGN32	*ps32rows,
						SIGN32	**ppVideoInfo,
						SIGN32	**ppAudioInfo,
						SIGN32	*ps32compression,
						SIGN32	*ps32ReadPictureCount
						)
	{
	HRESULT	hr;
	paviFile = NULL;
	pavi_video = pavi_audio = NULL;
	AVIFileInit();

	if(ps32ReadPictureCount) {							/* Load AVI file */

		/* Open AVI file for read */
		if((hr = AVIFileOpen(&paviFile, ps8FileName, OF_READ, NULL)) != AVIERR_OK)
			return ERR_FILE;

		PAVISTREAM pavi[2] = { NULL, NULL };
		for(SIGN32 i = 0; i < 2; i ++)
			if(AVIFileGetStream(paviFile, &pavi[i], 0, i) != AVIERR_OK)
				break;

		/* Get video/audio interfaces */
		pavi_video = pavi[0]; pavi_audio = pavi[1];
		if(pavi[0]) {
			AVIStreamInfo(pavi[0], &avi_vInfo, sizeof(AVISTREAMINFOA));
			if(avi_vInfo.fccType != streamtypeVIDEO) {
				pavi_video = pavi[1]; pavi_audio = pavi[0];
			}
		}

		/* Get video information */
		if(ppVideoInfo) *ppVideoInfo = pavi_video ? (SIGN32*)&avi_vInfo : NULL;
		if(pavi_video) {
			AVIStreamInfo(pavi_video, &avi_vInfo, sizeof(AVISTREAMINFOA));
			if(pf64fps) *pf64fps = (REAL64)avi_vInfo.dwRate / avi_vInfo.dwScale;
			if(ps32cols) *ps32cols = avi_vInfo.rcFrame.right - avi_vInfo.rcFrame.left;
			if(ps32rows) *ps32rows = avi_vInfo.rcFrame.bottom - avi_vInfo.rcFrame.top;
			if(ps32compression) *ps32compression = avi_vInfo.fccHandler;
			*ps32ReadPictureCount = MIN(*ps32ReadPictureCount, AVIStreamLength(pavi_video));
		}

		/* Get audio information */
		if(ppAudioInfo) *ppAudioInfo = pavi_audio ? (SIGN32*)&avi_aInfo : NULL;
		if(pavi_audio) {
			AVIStreamInfo(pavi_audio, &avi_aInfo, sizeof(AVISTREAMINFOA));
			if(!pavi_video) *ps32ReadPictureCount = 0;
		}
	}
	else {												/* Save AVI file */
		FILE *fp = fopen(ps8FileName, "rb");
		if(fp) { fclose(fp); DeleteFile(ps8FileName); }

		/* Open AVI file for write */
		if((hr = AVIFileOpen(&paviFile, ps8FileName, OF_WRITE | OF_CREATE, NULL)) != AVIERR_OK)
			return ERR_FILE;

		if(ps32compression) {							/* Write video */
			if(!ps32cols || !ps32rows)
				return ERR_POINTER;

			/* Prepare video information */
			if(ppVideoInfo)
				memcpy(&avi_vInfo, *ppVideoInfo, sizeof(AVISTREAMINFOA));
			else {
				memset(&avi_vInfo, 0, sizeof(AVISTREAMINFOA));
				avi_vInfo.fccType = streamtypeVIDEO;
				avi_vInfo.fccHandler = *ps32compression;
				avi_vInfo.dwScale = 1001;
				avi_vInfo.dwRate = pf64fps ? ROUND(*pf64fps * 1001) : 30000;
				avi_vInfo.dwSuggestedBufferSize= dibImageSize(0, *ps32cols, *ps32rows);
			}

			if((hr = AVIFileCreateStream(paviFile, &pavi_video, &avi_vInfo)) != AVIERR_OK)
				return ERR_UNKNOWN;

			/* Prepare picture information */
			SbmpInfoHeader bmpHeader;
			bmpHeader.biSize = sizeof(SbmpInfoHeader);
			bmpHeader.biWidth = *ps32cols;
			bmpHeader.biHeight = *ps32rows;
			bmpHeader.biPlanes = 1;
			bmpHeader.biCompression = *ps32compression;
			bmpHeader.biBitCount = (*ps32compression) ? 0 : 24;
			bmpHeader.biClrUsed = bmpHeader.biClrImportant = 0;
			bmpHeader.biXPelsPerMeter = bmpHeader.biYPelsPerMeter = 0;
			bmpHeader.biSizeImage = avi_vInfo.dwSuggestedBufferSize;

			if((hr = AVIStreamSetFormat(pavi_video, 0, &bmpHeader, sizeof(SbmpInfoHeader))) != AVIERR_OK)
				return ERR_UNKNOWN;
		}

		if(ppAudioInfo) {								/* Write audio */

			/* Prepare audio information */
			memcpy(&avi_aInfo, *ppAudioInfo, sizeof(AVISTREAMINFOA));
			if((hr = AVIFileCreateStream(paviFile, &pavi_audio, &avi_aInfo)) != AVIERR_OK)
				return ERR_UNKNOWN;
		}
	}

	return SUCCESS;
	/**	ENDOFFUNCTION: CAVIFileIO::Init **/
	}



	/******************************************************************************************************************
	*	Function: CAVIFileIO::~CAVIFileIO
	*	Description: Release an opened AVI file for read or write.
	******************************************************************************************************************/
	~CAVIFileIO(		)
	{
	/* Release AVI resources */
	if(pavi_video)
		AVIStreamRelease(pavi_video);
	if(pavi_audio)
		AVIStreamRelease(pavi_audio);
	if(paviFile)
		AVIFileRelease(paviFile);

	AVIFileExit();
	/**	ENDOFFUNCTION: CAVIFileIO::~CAVIFileIO **/
	}



	/******************************************************************************************************************
	*	Function: CAVIFileIO::Load
	*	Prototype description: ../_include/cmodel.h
	******************************************************************************************************************/
	SIGN32	Load(		SIGN8	s8v,
						UNSG8	*pu8Data,
						SIGN32	s32index,
						char	*ps8SaveBMP
						)
	{
	/* Value 0 will be returned if unsuccessful */
	SIGN32	s32DataSize = 0;

	if((s8v == 0) && pavi_audio)						/* Load audio */
		AVIStreamRead(pavi_audio, s32index, 1, pu8Data, 512000, (long*)&s32DataSize, NULL);

	if((s8v == 1) && pavi_video) {						/* Load video */

		/* Prepare parameters */
		SIGN32	s32cols = avi_vInfo.rcFrame.right - avi_vInfo.rcFrame.left;
		SIGN32	s32rows = avi_vInfo.rcFrame.bottom - avi_vInfo.rcFrame.top;
		s32DataSize = dibImageSize(0, s32cols, s32rows);

		/* Read a picture */
		AVIStreamRead(pavi_video, s32index, 1, pu8Data, s32DataSize, (long*)&s32DataSize, NULL);

		/* Save this picture to a BMP file */
		if(ps8SaveBMP)
			SaveBMP(ps8SaveBMP, s32cols, s32rows, pu8Data, 24);
	}

	return s32DataSize;
	/**	ENDOFFUNCTION: CAVIFileIO::Load **/
	}



	/******************************************************************************************************************
	*	Function: CAVIFileIO::Save
	*	Prototype description: ../_include/cmodel.h
	******************************************************************************************************************/
	SIGN32	Save(		SIGN8	s8v,
						UNSG8	*pu8Data,
						SIGN32	s32index,
						SIGN32	s32DataSize,
						SIGN32	dwFlags
						)
	{
	PAVISTREAM pavi = NULL;
	if((s8v == 0) && pavi_audio) pavi = pavi_audio;		/* Save audio */
	if((s8v == 1) && pavi_video) pavi = pavi_video;		/* Save video */

	/* Write a frame */
	if(pavi) {
		LET(dwFlags, dwFlags, AVIIF_KEYFRAME);			/* Save as key frame by default */
		AVIStreamWrite(pavi, s32index, 1, pu8Data, s32DataSize, dwFlags, NULL, NULL);
	}

	return pavi ? SUCCESS : ERR_UNKNOWN;
	/**	ENDOFFUNCTION: CAVIFileIO::Save **/
	}

	/**	ENDOFDEF: CLASS CAVIFileIO **/
	};

/**	ENDOFSECTION
 */

/**	WIN32
 */
#endif



/**	SECTION - definition of CLASS CYUVFileIO
 */
	/******************************************************************************************************************
	*	DEFINITION - CLASS CYUVFileIO
	******************************************************************************************************************/
	class CYUVFileIO : public CMediaFileIO {

	public:

	FILE				*fp;							/* YUV file handle */
	SIGN32				s32cols;						/* YUV picture horizontal size */
	SIGN32				s32rows;						/* YUV picture vertical size */
	SIGN32				s32format;						/* YUV format: bytes per 2x2 pixels */

	/******************************************************************************************************************
	*	Function: CYUVFileIO::Init
	*	Prototype description: ../_include/cmodel.h (please refer 'IMediaFileIO::CreateInstance')
	******************************************************************************************************************/
	SIGN32	Init(		char	*ps8FileName,
						REAL64	*pf64fps,
						SIGN32	*ps32cols,
						SIGN32	*ps32rows,
						SIGN32	**ppVideoInfo,
						SIGN32	**ppAudioInfo,
						SIGN32	*ps32compression,
						SIGN32	*ps32ReadPictureCount
						)
	{
	char	ps8path[KILO], ps8file[KILO];
	SIGN32	i;
	SplitPath(ps8FileName, ps8path, ps8file);

	if(ps32ReadPictureCount) {							/* Load YUV file */
		i = strlen(ps8file) - 4;

		/* Open YUV file for read */
		if((i < 1) || !(fp = fopen(ps8FileName, "rb")))
			return ERR_FILE;

		if(!_stricmp(ps8file + i, ".420") || !_stricmp(ps8file + i, ".yuv"))
			s32format = 4 + 1 + 1;
		else
		if(!_stricmp(ps8file + i, ".422") || !_stricmp(ps8file + i, "uyvy"))
			s32format = 4 + 2 + 2;
		else
			s32format = 4 + 4 + 4;

		if(ps32compression)
			*ps32compression = s32format;

		if((ps8file[0] < '0') || (ps8file[0] > '9'))
			s32cols = *ps32cols; s32rows = *ps32rows;
		else {
			sscanf(ps8file, "%dx%s", &s32cols, garrs8);
			for(i = 0; i < 8; i ++)
				if((garrs8[i] < '0') || (garrs8[i] > '9')) {
					garrs8[i] = '\0'; break;
				}
			sscanf(garrs8, "%d", &s32rows);
			if(ps32cols) *ps32cols = s32cols; if(ps32rows) *ps32rows = s32rows;
		}

		fseek(fp, 0, SEEK_END);
		*ps32ReadPictureCount = ftell(fp) / (s32cols * s32rows * s32format / 4);
		fseek(fp, 0, SEEK_SET);
	}
	else {												/* Save YUV file */
		s32cols = *ps32cols; s32rows = *ps32rows;
		s32format = ps32compression ? *ps32compression : (4 + 1 + 1);
		sprintf(garrs8, "%s%dx%d_%s", ps8path, *ps32cols, *ps32rows, ps8file);
		switch(s32format) {
		case 4 + 1 + 1: strcat(garrs8, ".420"); break;
		case 4 + 2 + 2: strcat(garrs8, ".422"); break;
		case 4 + 4 + 4: strcat(garrs8, ".444"); break;
		}

		if(!(fp = fopen(garrs8, "wb")))
			return ERR_FILE;
	}

	return SUCCESS;
	/**	ENDOFFUNCTION: CYUVFileIO::Init **/
	}



	/******************************************************************************************************************
	*	Function: CYUVFileIO::~CYUVFileIO
	*	Description: Release an opened AVI file for read or write.
	******************************************************************************************************************/
	~CYUVFileIO(		)
	{

	if(fp) fclose(fp);
	/**	ENDOFFUNCTION: CYUVFileIO::~CYUVFileIO **/
	}



	/******************************************************************************************************************
	*	Function: CYUVFileIO::Load
	*	Prototype description: ../_include/cmodel.h
	******************************************************************************************************************/
	SIGN32	Load(		SIGN8	s8v,
						UNSG8	*pu8Data,
						SIGN32	s32index,
						char	*ps8SaveBMP
						)
	{
	if(!s8v) return 0;
	SIGN32	s32DataSize = s32cols * s32rows * s32format / 4;

	fseek(fp, s32index * s32DataSize, SEEK_SET);
	s32DataSize = fread(pu8Data, 1, s32DataSize, fp);

	/* Save this picture to a BMP file */
	if(ps8SaveBMP)
		;	/* Not supported yet */

	return s32DataSize;
	/**	ENDOFFUNCTION: CYUVFileIO::Load **/
	}



	/******************************************************************************************************************
	*	Function: CYUVFileIO::Save
	*	Prototype description: ../_include/cmodel.h
	******************************************************************************************************************/
	SIGN32	Save(		SIGN8	s8v,
						UNSG8	*pu8Data,
						SIGN32	s32index,
						SIGN32	s32DataSize,
						SIGN32	dwFlags
						)
	{
	if(!s8v) return SUCCESS;
	SIGN32	s32DataSize = s32cols * s32rows * s32format / 4;

	fseek(fp, s32index * s32DataSize, SEEK_SET);
	fwrite(pu8Data, 1, s32DataSize, fp);

	return SUCCESS;
	/**	ENDOFFUNCTION: CYUVFileIO::Save **/
	}

	/**	ENDOFDEF: CLASS CYUVFileIO **/
	};

/**	ENDOFSECTION
 */



/**	SECTION - implementation of CMediaFileIO
 */
	/******************************************************************************************************************
	*	Function: CMediaFileIO::CreateInstance
	*	Prototype description: ../_include/cmodel.h (please refer 'IMediaFileIO::CreateInstance')
	******************************************************************************************************************/
	SIGN32	CMediaFileIO::CreateInstance(
						SIGN32	s32mode,
						IMediaFileIO	**pp,
						char	*ps8FileName,
						REAL64	*pf64fps,
						SIGN32	*ps32cols,
						SIGN32	*ps32rows,
						SIGN32	**ppVideoInfo,
						SIGN32	**ppAudioInfo,
						SIGN32	*ps32compression,
						SIGN32	*ps32ReadPictureCount
						)
	{
	if(!pp) return ERR_POINTER;
	*pp = NULL;

	/* Construct instance */
	switch(s32mode) {
	case __media_bmp:
			*pp = (IMediaFileIO*)new CBMPFileIO();
			break;
	case __media_avi:
			*pp = (IMediaFileIO*)new CAVIFileIO();
			break;
	case __media_yuv:
			*pp = (IMediaFileIO*)new CYUVFileIO();
			break;
	}

	if(!*pp) return ERR_UNKNOWN;
	((CMediaFileIO*)*pp)->s32mode = s32mode;

	/* Initialization */
	SIGN32 s32err = ((CMediaFileIO*)*pp)->Init(	ps8FileName,
												pf64fps,
												ps32cols,
												ps32rows,
												ppVideoInfo,
												ppAudioInfo,
												ps32compression,
												ps32ReadPictureCount
												);

	return (s32err == SUCCESS) ? SUCCESS : ERR_PARAMETER;
	/**	ENDOFFUNCTION: CMediaFileIO::CreateInstance **/
	}



	/******************************************************************************************************************
	*	Function: CMediaFileIO::Release
	*	Prototype description: ../_include/cmodel.h
	******************************************************************************************************************/
	void	CMediaFileIO::Release(
						)
	{
	/* Destruct instance */
	switch(s32mode) {
	case __media_bmp:
			delete (CBMPFileIO*)this;
			break;
	case __media_avi:
			delete (CAVIFileIO*)this;
			break;
	case __media_yuv:
			delete (CYUVFileIO*)this;
			break;
	}

	/**	ENDOFFUNCTION: CMediaFileIO::Release **/
	}

/**	ENDOFSECTION
 */



/**	SECTION - static implementation of IMediaFileIO
 */
	/******************************************************************************************************************
	*	Function: IMediaFileIO::CreateInstance
	*	Prototype description: ../_include/cmodel.h
	******************************************************************************************************************/
	SIGN32	IMediaFileIO::CreateInstance(
						IMediaFileIO	**pp,
						char	*ps8FileName,
						REAL64	*pf64fps,
						SIGN32	*ps32cols,
						SIGN32	*ps32rows,
						SIGN32	**ppVideoInfo,
						SIGN32	**ppAudioInfo,
						SIGN32	*ps32compression,
						SIGN32	*ps32ReadPictureCount
						)
	{
	char	*ps8FileSuffix = ps8FileName + strlen(ps8FileName) - 4;
	SIGN32	s32mode = __media_err;

	/* Decide media type */
	if(!_stricmp(ps8FileSuffix, ".bmp"))
		s32mode = __media_bmp;
	else
	if(!_stricmp(ps8FileSuffix, ".avi"))
		s32mode = __media_avi;
	else
	if(!_stricmp(ps8FileSuffix, ".420") ||
	   !_stricmp(ps8FileSuffix, ".yuv") ||
	   !_stricmp(ps8FileSuffix, ".422") ||
	   !_stricmp(ps8FileSuffix, "uyvy") ||
	   !_stricmp(ps8FileSuffix, ".444")
			)
		s32mode = __media_yuv;

	return CMediaFileIO::CreateInstance(		s32mode,
												pp,
												ps8FileName,
												pf64fps,
												ps32cols,
												ps32rows,
												ppVideoInfo,
												ppAudioInfo,
												ps32compression,
												ps32ReadPictureCount
												);
	/**	ENDOFFUNCTION: IMediaFileIO::CreateInstance **/
	}



	/******************************************************************************************************************
	*	Function: IMediaFileIO::CreateInstance
	*	Prototype description: ../_include/cmodel.h
	******************************************************************************************************************/
	SIGN32	IMediaFileIO::CreateInstance(
						IMediaFileIO	**pp,
						FILE	*fpCFG,
						char	*ps8FileName,
						REAL64	*pf64fps,
						SIGN32	*ps32cols,
						SIGN32	*ps32rows,
						SIGN32	**ppVideoInfo,
						SIGN32	**ppAudioInfo,
						SIGN32	*ps32compression,
						SIGN32	*ps32ReadPictureCount
						)
	{
	char	arrs8FileName[KILO];
	char	*ps8ItemName = ps32ReadPictureCount ? "LoadSourceMediaFile" : "SaveTargetMediaFile";
	if(!ps8FileName) ps8FileName = arrs8FileName;

	/* Get media file name from configuration file */
	SIGN32	s32r = _cfgAPI_GetItemString(fpCFG, ps8ItemName, "", ps8FileName);
	if(s32r == SUCCESS) s32r = CreateInstance(	pp,
												ps8FileName,
												pf64fps,
												ps32cols,
												ps32rows,
												ppVideoInfo,
												ppAudioInfo,
												ps32compression,
												ps32ReadPictureCount
												);
	else s32r = ERR_FILE;

	return s32r;
	/**	ENDOFFUNCTION: IMediaFileIO::CreateInstance **/
	}



	/******************************************************************************************************************
	*	Function: IMediaFileIO::LoadBMP
	*	Prototype description: ../_include/cmodel.h
	******************************************************************************************************************/
	SIGN32	IMediaFileIO::LoadBMP(
						char	*ps8FileName,
						SIGN32	*ps32cols,
						SIGN32	*ps32rows,
						UNSG8	**ppu8BMP,
						SIGN32	*ps32ClrBitCount
						)
	{
	/* Open BMP file first */
	FILE	*fp = fopen(ps8FileName, "rb");
	if(!fp) return ERR_FILE;

	/* Read header */
	SbmpInfoHeader bmpHeader;
	fseek(fp, sizeof(SbmpFileHeader), SEEK_SET);
	fread(&bmpHeader, sizeof(SbmpInfoHeader), 1, fp);

	SIGN32	s32width;
	if(bmpHeader.biBitCount == 8) {
		s32width = dibPitch(1, bmpHeader.biWidth);
		fseek(fp, 1024, SEEK_CUR);						/* Skip palette for grey scale BMP */
	}
	else if(bmpHeader.biBitCount == 24)
		s32width = dibPitch(0, bmpHeader.biWidth);
	else {
		fclose(fp);
		return ERR_UNKNOWN;								/* Other format not supported */
	}

	if(ps32cols) *ps32cols = bmpHeader.biWidth;
	if(ps32rows) *ps32rows = bmpHeader.biHeight;
	if(ps32ClrBitCount) *ps32ClrBitCount = bmpHeader.biBitCount;

	if(ppu8BMP) {
		if(!*ppu8BMP)									/* Allocate memory for picture data */
			*ppu8BMP = new UNSG8[s32width * bmpHeader.biHeight];

		/* Read picture data */
		if(fread(*ppu8BMP, s32width * bmpHeader.biHeight, 1, fp) < 1) {
			fclose(fp);
			return ERR_UNKNOWN;							/* Ruined BMP file */
		}
	}

	fclose(fp);
	return SUCCESS;
	/**	ENDOFFUNCTION: IMediaFileIO::LoadBMP **/
	}



	/******************************************************************************************************************
	*	Function: IMediaFileIO::SaveBMP
	*	Prototype description: ../_include/cmodel.h
	******************************************************************************************************************/
	SIGN32	IMediaFileIO::SaveBMP(
						char	*ps8FileName,
						SIGN32	s32cols,
						SIGN32	s32rows,
						UNSG8	*pu8BMP,
						SIGN32	s32ClrBitCount
						)
	{
	/* Check data pointer */
	if(!pu8BMP) return ERR_POINTER;

	/* Open BMP file */
	FILE	*fp = fopen(ps8FileName, "wb");
	if(!fp) return ERR_FILE;

    SbmpInfoHeader bmpHeader;
	bmpHeader.biSize = sizeof(SbmpInfoHeader);
	bmpHeader.biWidth = s32cols; bmpHeader.biHeight = s32rows;
	bmpHeader.biPlanes = 1; bmpHeader.biBitCount = s32ClrBitCount; bmpHeader.biCompression = BI_RGB;
	bmpHeader.biClrUsed = bmpHeader.biClrImportant = 1 << bmpHeader.biBitCount;
	bmpHeader.biXPelsPerMeter = bmpHeader.biYPelsPerMeter = 0;
	bmpHeader.biSizeImage = dibImageSize(bmpHeader.biBitCount == 8, s32cols, s32rows);

	SbmpFileHeader bmpfh;
	strcpy((char*)&(bmpfh.bfType), "BM");
	bmpfh.bfOffBits = sizeof(SbmpFileHeader) + sizeof(SbmpInfoHeader) + ((bmpHeader.biBitCount == 8) ? 1024 : 0);
	bmpfh.bfSize = bmpfh.bfOffBits + bmpHeader.biSizeImage;

	/* Write headers */
	fwrite(&bmpfh, sizeof(SbmpFileHeader), 1, fp); fwrite(&bmpHeader, sizeof(SbmpInfoHeader), 1, fp);

	if(bmpHeader.biBitCount == 8) {						/* Write palette for grey scale BMP */
		SrgbQuad quad;
		for(SIGN32 i = 0; i < 256; i ++) {
			quad.rgbBlue = quad.rgbGreen = quad.rgbRed = i; quad.rgbReserved = 0;
			fwrite(&quad, 4, 1, fp);
		}
	}

	/* Write picture data */
	fwrite(pu8BMP, bmpHeader.biSizeImage, 1, fp);

	fclose(fp);
	return SUCCESS;
	/**	ENDOFFUNCTION: IMediaFileIO::SaveBMP **/
	}

/**	ENDOFSECTION
 */



/**	__cplusplus && !__CODE_LINK__
 */
#endif

/**	ENDOFFILE: mfio.cpp ***********************************************************************************************
 */

