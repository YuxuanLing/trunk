#pragma once
#pragma include_alias( "dxtrans.h", "qedit.h" )
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__
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
	int m_nFrameIndex;

public:
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppv);
	HRESULT STDMETHODCALLTYPE SampleCB(double SampleTime, IMediaSample *pSample);
	HRESULT STDMETHODCALLTYPE BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen);


};

