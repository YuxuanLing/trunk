#include "stdafx.h"
#include "SampleGrabberCB.h"


CSampleGrabberCB::CSampleGrabberCB(void)
{
	m_fp_dst = NULL;
	m_bBeginEncode = FALSE; 
	m_bEndEncode = FALSE;
	m_bFirst = TRUE;
	m_nFrameIndex = 0;
	m_sSavePath = _T("");
}


CSampleGrabberCB::~CSampleGrabberCB(void)
{
	if (NULL != m_fp_dst)fclose(m_fp_dst);

	m_fp_dst = NULL;

}

ULONG STDMETHODCALLTYPE CSampleGrabberCB::AddRef() 
{ 
	return 2; 
}

ULONG STDMETHODCALLTYPE CSampleGrabberCB::Release() 
{ 
	return 1; 
}

HRESULT STDMETHODCALLTYPE CSampleGrabberCB::QueryInterface(REFIID riid, void ** ppv)
{
	if( riid == IID_ISampleGrabberCB || riid == IID_IUnknown )
	{ 
		*ppv = (void *) static_cast<ISampleGrabberCB*> ( this );
		return NOERROR;
	} 
        
	return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE CSampleGrabberCB::SampleCB(double SampleTime, IMediaSample *pSample)
{
	long BuffLen = pSample->GetActualDataLength();
	BYTE *pBuff;
	CTime time = CTime::GetCurrentTime();
	CString strSuffix = time.Format("%Y%m%d_%H%M%S_yxling.yuv");
	CString strSavePath = _T("");
	strSavePath.Format(_T("%s%s"), m_sSavePath, strSuffix);
	USES_CONVERSION;
	string strFullPath = W2A(strSavePath);
	pSample->GetPointer(&pBuff);
	if (m_bBeginEncode) {
		if(m_fp_dst == NULL)fopen_s(&m_fp_dst, strFullPath.c_str(), "wb+");
		if (m_fp_dst && pSample) {
			fwrite(pBuff, 1, BuffLen, m_fp_dst);
		}
	}



	return 0;
}
        
HRESULT STDMETHODCALLTYPE CSampleGrabberCB::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
{
	return 0;
}

