
// GetDeviceInfoDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "SampleGrabberCB.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE( x ) \
if ( NULL != x ) \
{ \
    x->Release(); \
    x = NULL; \
}
#endif

//class CSampleGrabberCB : public ISampleGrabberCB 
//{
//public:
//	long lWidth;
//	long lHeight;
//	
//
//	STDMETHODIMP_(ULONG) AddRef() { return 2; }
//	STDMETHODIMP_(ULONG) Release() { return 1; }
//	STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
//	{
//		if( riid == IID_ISampleGrabberCB || riid == IID_IUnknown )
//		{ 
//			*ppv = (void *) static_cast<ISampleGrabberCB*> ( this );
//			return NOERROR;
//		} 
//        
//		return E_NOINTERFACE;
//	}
//
//	HRESULT STDMETHODCALLTYPE SampleCB(double SampleTime, IMediaSample *pSample)
//	{
//		return 0;
//	}
//        
//	HRESULT STDMETHODCALLTYPE BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
//	{
//		CString str;
//		str.Format(_T("\n BufferCB--lBufferSize:%ld,lWidth:%d,lHeight:%d"), BufferLen, lWidth, lHeight);
//		OutputDebugString(str);
//
//
//		return 0;
//	}
//};

struct ImgDeviceInfo
{
	CString strDevicePidVid;		
	CString strDeviceName;			
	int nDeviceIndex;				

	ImgDeviceInfo()
	{
		strDevicePidVid = _T("");
		strDeviceName = _T("");
		nDeviceIndex = -1;
	};
	
	ImgDeviceInfo(const ImgDeviceInfo &other)
	{
		*this = other;
	};
	
	ImgDeviceInfo& operator = (const ImgDeviceInfo& other)
	{
		strDevicePidVid = other.strDevicePidVid;
		strDeviceName = other.strDeviceName;
		nDeviceIndex = other.nDeviceIndex;
		return *this;
	};
};
typedef CArray <ImgDeviceInfo, ImgDeviceInfo&> ASImgDeviceInfoArray;

struct CamResolutionInfo
{
	int nWidth;
	int nHeight;
	int nResolutionIndex;

	CamResolutionInfo()
	{
		nWidth = 640;
		nHeight = 480;
		nResolutionIndex = -1;
	};
	
	CamResolutionInfo(const CamResolutionInfo &other)
	{
		*this = other;
	};
	
	CamResolutionInfo& operator = (const CamResolutionInfo& other)
	{
		nWidth = other.nWidth;
		nHeight = other.nHeight;
		nResolutionIndex = other.nResolutionIndex;
		return *this;
	};
};
typedef CArray <CamResolutionInfo, CamResolutionInfo&> ASCamResolutionInfoArray;

// CGetDeviceInfoDlg dialog
class CGetDeviceInfoDlg : public CDialogEx
{
// Construction
public:
	CGetDeviceInfoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_GETDEVICEINFO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedBtnGetdevice();
	afx_msg void OnBnClickedBtnInit();
	afx_msg void OnBnClickedBtnPreview();
	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnBnClickedBtnBeginEncode();
	afx_msg void OnBnClickedBtnEndEncode();
	afx_msg void OnDestroy();
	BOOL ListVideoCaptureDevices();
	BOOL ListAudioCaptureDevices();
	BOOL ListVideoCompressDevices();
	BOOL ListAudioCompressDevices();
	HRESULT InitCaptureGraphBuilder(IGraphBuilder **ppGraph, ICaptureGraphBuilder2 **ppBuild);
	void CreateVideoFilter(CString strSelectedDevice, IBaseFilter **pBaseFilter);
	void GetVideoResolution();
	void FreeMediaType(AM_MEDIA_TYPE *pmt);
	

public:
	CComboBox m_cbxCtrl;
	CComboBox m_cbxAudioCtrl;
	CComboBox m_cbxCompressCtrl;
	CComboBox m_cbxAudioCompressor;
	CComboBox m_cbxResolutionCtrl;
	ASImgDeviceInfoArray m_asVideoDeviceInfo;
	ASImgDeviceInfoArray m_asAudioDeviceInfo;
	ASImgDeviceInfoArray m_asCompressDeviceInfo;
	ASImgDeviceInfoArray m_asAudioCompressorInfo;
	ASCamResolutionInfoArray m_arrCamResolutionArr;
	IGraphBuilder *m_pGraphBuilder;
    ICaptureGraphBuilder2* m_pCapture;
	IMediaControl  *m_pMediaControl;
	IVideoWindow* m_pVW;
	IBaseFilter* m_pVideoFilter;
	IBaseFilter* m_pGrabberFilter;
	HWND m_hShowWnd;
	BOOL m_bIsVideoOpen;
	BOOL m_bRendered;
	ISampleGrabber* m_pGrabber;
	CSampleGrabberCB mCB;
	

	afx_msg void OnBnClickedBtnPath();
};
