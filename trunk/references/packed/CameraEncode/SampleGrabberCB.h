#pragma once
#include "qedit.h"
class CSampleGrabberCB :
	public ISampleGrabberCB
{
public:
	CSampleGrabberCB(void);
	virtual ~CSampleGrabberCB(void);

public:
	long lWidth;
	long lHeight;
	BOOL m_bBeginEncode;
	BOOL m_bFirst;
	BOOL m_bEndEncode;
	CString m_sSavePath;

	FILE* m_fp_dst;
	x264_param_t* m_pParam;
	x264_picture_t* m_pPic_in;
	x264_picture_t* m_pPic_out;
	x264_t* m_pHandle;
	int m_nFrameIndex;

public:
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppv);
	HRESULT STDMETHODCALLTYPE SampleCB(double SampleTime, IMediaSample *pSample);
	HRESULT STDMETHODCALLTYPE BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen);
	BOOL RGB2YUV(LPBYTE RgbBuf, UINT nWidth, UINT nHeight, LPBYTE yuvBuf, unsigned long *len);


};

