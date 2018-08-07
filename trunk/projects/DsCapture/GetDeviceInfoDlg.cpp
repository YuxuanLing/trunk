
// GetDeviceInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GetDeviceInfo.h"
#include "GetDeviceInfoDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CGetDeviceInfoDlg dialog
CGetDeviceInfoDlg::CGetDeviceInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGetDeviceInfoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_asVideoDeviceInfo.RemoveAll();
	m_asAudioDeviceInfo.RemoveAll();
	m_asCompressDeviceInfo.RemoveAll();
	m_asAudioCompressorInfo.RemoveAll();
	m_arrCamResolutionArr.RemoveAll();
	m_yuvFormatArr.RemoveAll();
	m_pGraphBuilder = NULL;
	m_pCapture = NULL;
	m_pMediaControl = NULL;
	m_pVW = NULL;
	m_pVideoFilter = NULL;
	m_pGrabberFilter = NULL;
	m_hShowWnd = NULL;
	m_bIsVideoOpen = FALSE;
	m_bRendered = FALSE;
	m_pGrabber = NULL;
}

void CGetDeviceInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CBO_DSHOW_DEVICE, m_cbxCtrl);
	DDX_Control(pDX, IDC_CBO_AUDIO_DEVICE, m_cbxAudioCtrl);
	DDX_Control(pDX, IDC_CBO_COMPRESS_DEVICE, m_cbxCompressCtrl);
	DDX_Control(pDX, IDC_CBO_AUDIO_COMPRESSOR, m_cbxAudioCompressor);
	DDX_Control(pDX, IDC_CBO_VIDEO_RESOLUTION, m_cbxResolutionCtrl);
	DDX_Control(pDX, IDC_CBO_VIDEO_FORMAT, m_cbxFormatCtrl);
}

BEGIN_MESSAGE_MAP(CGetDeviceInfoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_GETDEVICE, &CGetDeviceInfoDlg::OnBnClickedBtnGetdevice)
	ON_BN_CLICKED(IDC_BTN_INIT, &CGetDeviceInfoDlg::OnBnClickedBtnInit)
	ON_BN_CLICKED(IDC_BTN_PREVIEW, &CGetDeviceInfoDlg::OnBnClickedBtnPreview)
	ON_BN_CLICKED(IDC_BTN_STOP, &CGetDeviceInfoDlg::OnBnClickedBtnStop)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_BEGIN_ENCODE, &CGetDeviceInfoDlg::OnBnClickedBtnBeginEncode)
	ON_BN_CLICKED(IDC_BTN_END_ENCODE, &CGetDeviceInfoDlg::OnBnClickedBtnEndEncode)
	ON_BN_CLICKED(IDC_BTN_PATH, &CGetDeviceInfoDlg::OnBnClickedBtnPath)
END_MESSAGE_MAP()


// CGetDeviceInfoDlg message handlers
BOOL CGetDeviceInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// Add "About..." menu item to system menu.
	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGetDeviceInfoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGetDeviceInfoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGetDeviceInfoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


BOOL CGetDeviceInfoDlg::ListVideoCaptureDevices()
{
	int count = 0;

	ImgDeviceInfo sDevice;
   // enumerate all video capture devices
	ICreateDevEnum *pCreateDevEnum = NULL;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
									IID_ICreateDevEnum, (void**)&pCreateDevEnum);

    IEnumMoniker *pEm = NULL;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
    if (hr != NOERROR) 
		return 0;

    ULONG cFetched;
    IMoniker *pM = NULL;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    {
		sDevice.nDeviceIndex = count;

		LPOLESTR strPidvid = NULL;
		hr = pM->GetDisplayName(0, 0, &strPidvid);  
		if(SUCCEEDED(hr))
		{
			
			USES_CONVERSION; //OLE2T
			CString sPidvid = strPidvid;
			string str = T2A(sPidvid);
			string result;
			static const regex re("(vid_[0-9a-f]{4}&pid_[0-9a-f]{4})",regex::icase);
			smatch match;
			if (regex_search(str, match, re) && match.size() > 1) 
			{
				result = match.str(1);
			} 
			else 
			{
				result = string("");
			} 
			CString strPid(result.c_str());
			strPid.MakeUpper(); //
			sDevice.strDevicePidVid = strPid;

			IPropertyBag *pBag=0;
			hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if(SUCCEEDED(hr))
			{
				VARIANT var;
				var.vt = VT_BSTR;
				hr = pBag->Read(L"FriendlyName", &var, NULL);
				if(hr == NOERROR)
				{
					char camera_name[1024]; 
					WideCharToMultiByte(CP_ACP,0,var.bstrVal,-1, camera_name, sizeof(camera_name) ,"",NULL);
					CString str(camera_name);
					sDevice.strDeviceName = str;
					m_asVideoDeviceInfo.Add(sDevice);
					m_cbxCtrl.AddString(str);

					SysFreeString(var.bstrVal);				
				}
				pBag->Release();
			}
		}
		pM->Release();
		
		count++;
    }

	if (m_cbxCtrl.GetCount() > 0)
	{
		m_cbxCtrl.SetCurSel(0);
	}

	pEm->Release();
	pCreateDevEnum->Release();

	return 1;
}

BOOL CGetDeviceInfoDlg::ListAudioCaptureDevices()
{
	int count = 0;

	ImgDeviceInfo sDevice;
   // enumerate all Audio capture devices
	ICreateDevEnum *pCreateDevEnum = NULL;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
									IID_ICreateDevEnum, (void**)&pCreateDevEnum);

    IEnumMoniker *pEm = NULL;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEm, 0);
    if (hr != NOERROR) 
		return 0;

    ULONG cFetched;
    IMoniker *pM = NULL;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    {
		sDevice.nDeviceIndex = count;

		LPOLESTR strPidvid = NULL;
		hr = pM->GetDisplayName(0, 0, &strPidvid); 
		if(SUCCEEDED(hr))
		{
			USES_CONVERSION; //OLE2T
			CString sPidvid = strPidvid;
			string str = T2A(sPidvid);
			string result;
			static const regex re("(vid_[0-9a-f]{4}&pid_[0-9a-f]{4})",regex::icase);
			smatch match;
			if (regex_search(str, match, re) && match.size() > 1) 
			{
				result = match.str(1);
			} 
			else 
			{
				result = string("");
			} 
			CString strPid(result.c_str());
			strPid.MakeUpper();
			sDevice.strDevicePidVid = strPid;

			IPropertyBag *pBag=0;
			hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if(SUCCEEDED(hr))
			{
				VARIANT var;
				var.vt = VT_BSTR;
				hr = pBag->Read(L"FriendlyName", &var, NULL);
				if(hr == NOERROR)
				{
					char camera_name[1024]; 
					WideCharToMultiByte(CP_ACP,0,var.bstrVal,-1, camera_name, sizeof(camera_name) ,"",NULL);
					CString str(camera_name);
					sDevice.strDeviceName = str;
					m_asAudioDeviceInfo.Add(sDevice);
					m_cbxAudioCtrl.AddString(str);

					SysFreeString(var.bstrVal);				
				}
				pBag->Release();
			}
		}
		pM->Release();
		
		count++;
    }

	if (m_cbxAudioCtrl.GetCount() > 0)
	{
		m_cbxAudioCtrl.SetCurSel(0);
	}

	pEm->Release();
	pCreateDevEnum->Release();

	return 1;
}

BOOL CGetDeviceInfoDlg::ListVideoCompressDevices()
{
	int count = 0;

	ImgDeviceInfo sDevice;
   // enumerate all video compressor
	ICreateDevEnum *pCreateDevEnum = NULL;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
									IID_ICreateDevEnum, (void**)&pCreateDevEnum);

    IEnumMoniker *pEm = NULL;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoCompressorCategory, &pEm, 0);
    if (hr != NOERROR) 
		return 0;

    ULONG cFetched;
    IMoniker *pM = NULL;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    {
		sDevice.nDeviceIndex = count;
		IPropertyBag *pBag=0;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if(SUCCEEDED(hr))
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL); 
			if(hr == NOERROR)
			{
				char camera_name[1024]; 
				WideCharToMultiByte(CP_ACP,0,var.bstrVal,-1, camera_name, sizeof(camera_name) ,"",NULL);
				CString str(camera_name);
				sDevice.strDeviceName = str;
				m_asCompressDeviceInfo.Add(sDevice);
				m_cbxCompressCtrl.AddString(str);

				SysFreeString(var.bstrVal);				
			}
			pBag->Release();
		}
		pM->Release();
		
		count++;
    }

	if (m_cbxCompressCtrl.GetCount() > 0)
	{
		m_cbxCompressCtrl.SetCurSel(0);
	}

	pEm->Release();
	pCreateDevEnum->Release();

	return 1;
}

BOOL CGetDeviceInfoDlg::ListAudioCompressDevices()
{
	int count = 0;

	ImgDeviceInfo sDevice;
   // enumerate all Audio compressor 
	ICreateDevEnum *pCreateDevEnum = NULL;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
									IID_ICreateDevEnum, (void**)&pCreateDevEnum);

    IEnumMoniker *pEm = NULL;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_AudioCompressorCategory, &pEm, 0);
    if (hr != NOERROR) 
		return 0;

    ULONG cFetched;
    IMoniker *pM = NULL;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    {
		sDevice.nDeviceIndex = count;
		IPropertyBag *pBag=0;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if(SUCCEEDED(hr))
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL); 
			if(hr == NOERROR)
			{
				char camera_name[1024]; 
				WideCharToMultiByte(CP_ACP,0,var.bstrVal,-1, camera_name, sizeof(camera_name) ,"",NULL);
				CString str(camera_name);
				sDevice.strDeviceName = str;
				m_asAudioCompressorInfo.Add(sDevice);
				m_cbxAudioCompressor.AddString(str);

				SysFreeString(var.bstrVal);				
			}
			pBag->Release();
		}
		pM->Release();
		
		count++;
    }

	if (m_cbxAudioCompressor.GetCount() > 0)
	{
		m_cbxAudioCompressor.SetCurSel(0);
	}

	pEm->Release();
	pCreateDevEnum->Release();

	return 1;
}

void CGetDeviceInfoDlg::OnBnClickedBtnGetdevice()
{
	m_cbxCtrl.ResetContent();
	m_cbxAudioCtrl.ResetContent();
	m_cbxCompressCtrl.ResetContent();
	m_cbxAudioCompressor.ResetContent();
	m_asVideoDeviceInfo.RemoveAll();
	m_asAudioDeviceInfo.RemoveAll();
	m_asCompressDeviceInfo.RemoveAll();
	m_asAudioCompressorInfo.RemoveAll();
	ListVideoCaptureDevices();
	ListAudioCaptureDevices();
	ListVideoCompressDevices();
	ListAudioCompressDevices();
}


HRESULT CGetDeviceInfoDlg::InitCaptureGraphBuilder(IGraphBuilder **ppGraph, ICaptureGraphBuilder2 **ppBuild)
{
	if (!ppGraph || !ppBuild)
	{
		return E_POINTER;
	}
	IGraphBuilder *pGraph = NULL;
	ICaptureGraphBuilder2 *pBuild = NULL;
	// Create the Capture Graph Builder.
	HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild);
	if (SUCCEEDED(hr))
	{
		// Create the Filter Graph Manager.
		hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraph);
		if (SUCCEEDED(hr))
		{
			// Initialize the Capture Graph Builder.
			pBuild->SetFiltergraph(pGraph); 

			// Return both interface pointers to the caller.
			*ppBuild = pBuild;
			*ppGraph = pGraph; // The caller must release both interfaces.

			pGraph->QueryInterface(IID_IMediaControl, (void **)&m_pMediaControl);
			pGraph->QueryInterface(IID_IVideoWindow, (void **)&m_pVW);

			return S_OK;
		}
		else
		{
			pBuild->Release();
		}
	}

	return hr; // Failed
}

//Video Capture Filter
void CGetDeviceInfoDlg::CreateVideoFilter(CString strSelectedDevice, IBaseFilter **pBaseFilter)
{
	ICreateDevEnum *pCreateDevEnum = NULL;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
									IID_ICreateDevEnum, (void**)&pCreateDevEnum);

    IEnumMoniker *pEm = NULL;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
    if (hr != NOERROR) 
		return;

    ULONG cFetched;
    IMoniker *pM = NULL;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    {
		IPropertyBag *pBag=0;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if(SUCCEEDED(hr))
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if(hr == NOERROR)
			{
				char camera_name[1024]; 
				WideCharToMultiByte(CP_ACP,0,var.bstrVal,-1, camera_name, sizeof(camera_name) ,"",NULL);
				CString str(camera_name);
				SysFreeString(var.bstrVal);	
				if (strSelectedDevice == str)
				{
					hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pBaseFilter);
					if (FAILED(hr))
					{
						OutputDebugString(_T("BindToObject Failed"));
					}
				}
			}
			pBag->Release();
		}
		pM->Release();
    }

	pEm->Release();
	pCreateDevEnum->Release();
}

void CGetDeviceInfoDlg::OnBnClickedBtnInit()
{
	//1
	int nIndex  = m_cbxCtrl.GetCurSel();
	if (nIndex < 0)
	{
		MessageBox(_T("Choose Video Device"), _T("Attention"));
		return;
	}

	CString strDeviceName = _T("");
	m_cbxCtrl.GetLBText(nIndex, strDeviceName);
	if (strDeviceName.IsEmpty())
	{
		MessageBox(_T("Get NULL Video Device"), _T("Attention"));
		return;
	}
	//Video Capture Filter
	CreateVideoFilter(strDeviceName, &m_pVideoFilter);
	if (m_pVideoFilter == NULL)
	{
		MessageBox(_T("Get m_pVideoFilter Failed"), _T("Attention"));
		return;
	}

	//
	HRESULT hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&m_pGrabberFilter); 
	if (m_pGrabberFilter == NULL)
	{
		MessageBox(_T("Create m_pGrabberFilter Failed"), _T("Attention"));
		return;
	}

	//
	hr = InitCaptureGraphBuilder(&m_pGraphBuilder, &m_pCapture);
	if (S_OK != hr || m_pGraphBuilder == NULL)
	{
		MessageBox(_T("IGraphBuilder CaptureGraphBuilder2 init failed"), _T("Attention"));
		return;
	}

	//
	hr = m_pGraphBuilder->AddFilter(m_pVideoFilter, L"Video Capture Filter");
	if (S_OK != hr)
	{
		MessageBox(_T("add m_pVideoFilter to IGraphBuilder failed"), _T("Attention"));
		return;
	}

	//
	hr = m_pGraphBuilder->AddFilter(m_pGrabberFilter, L"Grabber");
	if(S_OK != hr)
	{
		MessageBox(_T("Fail to put sample grabber in graph"));
		return;
	}

	GetVideoResolution();
	GetDsCaptureFormat();
}




void CGetDeviceInfoDlg::OnBnClickedBtnPreview()
{
	if (m_bIsVideoOpen)
	{
		return;
	}

	if (!m_bRendered)
	{
		//
		int nSel = m_cbxResolutionCtrl.GetCurSel();
		if (nSel < 0)
		{
			MessageBox(_T("Resolution GetCurSel Failed"), _T("Attention"));
			return;
		}
		CString strResolution = _T("");
		m_cbxResolutionCtrl.GetLBText(nSel, strResolution);
		if (strResolution.IsEmpty())
		{
			MessageBox(_T("strResolution is empty"), _T("Attention"));
			return;
		}

		int nSetWidth = -1;
		int nSetHeight = -1;
		int nFindValue = strResolution.Find(_T("*"));
		if (nFindValue > 0)
		{
			nSetWidth = _ttoi(strResolution.Left(nFindValue));
			nSetHeight = _ttoi(strResolution.Mid(nFindValue + 1));
		}
		int nResolutionIndex = 0;
		for (int i = 0; i < m_arrCamResolutionArr.GetSize(); i++)
		{
			CamResolutionInfo camInfo = m_arrCamResolutionArr.GetAt(i);
			if (camInfo.nWidth = nSetWidth && camInfo.nHeight == nSetHeight)
			{
				nResolutionIndex = camInfo.nResolutionIndex;
				break;
			}
		}

		/*****************************************************************/
		IAMStreamConfig *pConfig = NULL;  
		m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, 
							m_pVideoFilter, IID_IAMStreamConfig, (void **) &pConfig);

		AM_MEDIA_TYPE *pmt = NULL; 
		VIDEO_STREAM_CONFIG_CAPS scc;
		pConfig->GetStreamCaps(nResolutionIndex, &pmt, (BYTE*)&scc);

		nSel = m_cbxFormatCtrl.GetCurSel();
		if (nSel < 0)
		{
			MessageBox(_T("Format GetCurSel Failed"), _T("Attention"));
			return;
		}
		GUID subFormatType;
		bool foundFormat = false;

		for (int i = 0; i < m_yuvFormatArr.GetSize(); i++)
		{
			if (nSel == m_yuvFormatArr.GetAt(i).nFormatIndex) {
				foundFormat = true;
				subFormatType = m_yuvFormatArr.GetAt(i).guidFormat;
				break;
			}
		}

		if (!foundFormat)
		{
			MessageBox(_T("Can not Found Selected Format!"), _T("Attention"));
			return;
		}

		pmt->majortype = MEDIATYPE_Video;	
		pmt->subtype = subFormatType;
		pmt->formattype = FORMAT_VideoInfo;

		pConfig->SetFormat(pmt);

		m_pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void **)&m_pGrabber);
		HRESULT hr = m_pGrabber->SetMediaType(pmt);
		if(FAILED(hr))
		{
			AfxMessageBox(_T("Fail to set media type!"));
			return;
		}
		//
		m_pGrabber->SetBufferSamples(FALSE);
		m_pGrabber->SetOneShot(FALSE);
		mCB.lWidth = nSetWidth;
		mCB.lHeight = nSetHeight;
		//set callback , 0 means using sample callback , 1 means use buff callback
		m_pGrabber->SetCallback(&mCB, 0);



		/*****************************************************************/
		hr = m_pCapture->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, m_pVideoFilter, m_pGrabberFilter, NULL);
		if( FAILED(hr))
		{
			AfxMessageBox(_T("RenderStream failed"));
			return;
		}

		m_hShowWnd = GetDlgItem(IDC_STC_SHOW)->m_hWnd ; 
		hr = m_pVW->put_Owner((OAHWND)m_hShowWnd);
		if (FAILED(hr)) 
			return;
		hr = m_pVW->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN);
		if (FAILED(hr)) 
			return;

		if (m_pVW)
		{    
			CRect rc;
			::GetClientRect(m_hShowWnd,&rc);
			m_pVW->SetWindowPosition(0, 0, rc.right, rc.bottom);
		} 

		hr = m_pVW->put_Visible(OATRUE);

		m_bRendered = TRUE;
	}

	HRESULT hr = m_pMediaControl->Run();
    if(FAILED(hr))
    {
        AfxMessageBox(_T("Couldn't run the graph!"));
        return;
    }
	m_bIsVideoOpen = TRUE;
}

void CGetDeviceInfoDlg::OnBnClickedBtnStop()
{
	if (m_pMediaControl)
	{
		m_pMediaControl->Stop();
	}
	m_bIsVideoOpen = FALSE;
}

void CGetDeviceInfoDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	if (m_bIsVideoOpen)
	{
		// Stop media playback
        if(m_pMediaControl)
        {
            m_pMediaControl->Stop();
        }
        
        if(m_pVW)
        {
            m_pVW->put_Visible(OAFALSE);
            m_pVW->put_Owner(NULL);
        }
        
        SAFE_RELEASE(m_pCapture);
        SAFE_RELEASE(m_pMediaControl);
        SAFE_RELEASE(m_pGraphBuilder);
        SAFE_RELEASE(m_pVideoFilter);
	}
}

void CGetDeviceInfoDlg::GetVideoResolution()
{
	if (m_pCapture)
	{
		m_arrCamResolutionArr.RemoveAll();
		m_cbxResolutionCtrl.ResetContent();
		IAMStreamConfig *pConfig = NULL;  
		HRESULT hr = m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, 
										m_pVideoFilter, IID_IAMStreamConfig, (void **)&pConfig);

		int iCount = 0, iSize = 0;
		hr = pConfig->GetNumberOfCapabilities(&iCount, &iSize);
		// Check the size to make sure we pass in the correct structure.
		if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
		{
			// Use the video capabilities structure.
			for (int iFormat = 0; iFormat < iCount; iFormat++)
			{
				VIDEO_STREAM_CONFIG_CAPS scc;
				AM_MEDIA_TYPE *pmtConfig = NULL;
				hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
				if (SUCCEEDED(hr))
				{
					if ((pmtConfig->majortype == MEDIATYPE_Video) &&
						(pmtConfig->formattype == FORMAT_VideoInfo) &&
						(pmtConfig->cbFormat >= sizeof (VIDEOINFOHEADER)) &&
						(pmtConfig->pbFormat != NULL))
					{
						VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
						// pVih contains the detailed format information.
						LONG lWidth = pVih->bmiHeader.biWidth;
						LONG lHeight = pVih->bmiHeader.biHeight;
						BOOL bFind = FALSE;
						for (int n=0; n < m_arrCamResolutionArr.GetSize(); n++)
						{
							CamResolutionInfo sInfo = m_arrCamResolutionArr.GetAt(n);
							if (sInfo.nWidth == lWidth && sInfo.nHeight == lHeight)
							{
								bFind = TRUE;
								break;
							}
						}
						if (!bFind)
						{
							CamResolutionInfo camInfo;
							camInfo.nResolutionIndex = iFormat;
							camInfo.nWidth = lWidth;
							camInfo.nHeight = lHeight;
							m_arrCamResolutionArr.Add(camInfo);

							CString strFormat = _T("");
							strFormat.Format(_T("%d * %d"), lWidth, lHeight);
							m_cbxResolutionCtrl.AddString(strFormat);
						}
					}
					FreeMediaType(pmtConfig);
				}
			}
		}
		if (m_cbxResolutionCtrl.GetCount() > 0)
		{
			m_cbxResolutionCtrl.SetCurSel(0);
		}
	}
}


bool ConvertYuvGuidToStrName(YuvFormatInfo &info)
{
	if (info.guidFormat == MEDIASUBTYPE_YUY2)
	{
		info.strFormatName = _T("YUV2");
	}
	else if(info.guidFormat == MEDIASUBTYPE_YUYV)
	{
		info.strFormatName = _T("YUYV");
	}
	else if (info.guidFormat == MEDIASUBTYPE_RGB24)
	{
		info.strFormatName = _T("RGB24");
	}
	else if (info.guidFormat == MEDIASUBTYPE_RGB32)
	{
		info.strFormatName = _T("RGB32");
	}
	else if (info.guidFormat == MEDIASUBTYPE_MJPG)
	{
		info.strFormatName = _T("MJPG");
	}
	else if (info.guidFormat == MEDIASUBTYPE_H264)
	{
		info.strFormatName = _T("H264");
	}
	else if (info.guidFormat == MEDIASUBTYPE_NV12)
	{
		info.strFormatName = _T("NV12");
	}
	else
	{
		//AfxMessageBox(_T("Found a not listed Capture Format"));
		return false;
	}
	
	return true;
	
}

void CGetDeviceInfoDlg::GetDsCaptureFormat()
{

	if (m_pCapture) {
		m_yuvFormatArr.RemoveAll();
		m_cbxFormatCtrl.ResetContent();
		//add a default format yuv2, Todo: fix it
		YuvFormatInfo defaultFormat;
		defaultFormat.strFormatName = _T("YUV2");
		defaultFormat.guidFormat = MEDIASUBTYPE_YUY2;
		defaultFormat.nFormatIndex = 0;
		m_yuvFormatArr.Add(defaultFormat);
		m_cbxFormatCtrl.AddString(defaultFormat.strFormatName);

		IAMStreamConfig *pConfig = NULL;
		HRESULT hr = m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
			m_pVideoFilter, IID_IAMStreamConfig, (void **)&pConfig);


		int iCount = 0, iSize = 0;
		hr = pConfig->GetNumberOfCapabilities(&iCount, &iSize);

		if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
		{
			// Use the video capabilities structure.
			for (int iFormat = 0; iFormat < iCount; iFormat++)
			{
				VIDEO_STREAM_CONFIG_CAPS scc;
				AM_MEDIA_TYPE *pmtConfig = NULL;
				hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
				if (SUCCEEDED(hr))
				{
					if ((pmtConfig->majortype == MEDIATYPE_Video) &&
						(pmtConfig->formattype == FORMAT_VideoInfo) &&
						(pmtConfig->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
						(pmtConfig->pbFormat != NULL))
					{
						BOOL bFind = FALSE;
						int n = 0;
						for (n = 0; n < m_yuvFormatArr.GetSize(); n++)
						{
							YuvFormatInfo sInfo = m_yuvFormatArr.GetAt(n);
							if (sInfo.guidFormat == pmtConfig->subtype)
							{
								bFind = TRUE;
								break;
							}
						}
						if (!bFind)
						{
							bool ret = true;
							YuvFormatInfo yuvInfo;
							yuvInfo.guidFormat = pmtConfig->subtype;
							yuvInfo.nFormatIndex = m_yuvFormatArr.GetSize();
							
							ret = ConvertYuvGuidToStrName(yuvInfo);

							if (ret) {
								m_yuvFormatArr.Add(yuvInfo);
								CString strFormat = yuvInfo.strFormatName;
								m_cbxFormatCtrl.AddString(strFormat);
							}
						}
					}
					FreeMediaType(pmtConfig);
				}
			}		
		}

		if (m_cbxFormatCtrl.GetCount() > 0)
		{
			m_cbxFormatCtrl.SetCurSel(0);
		}
	}

}



void CGetDeviceInfoDlg::FreeMediaType(AM_MEDIA_TYPE *pmt)
{
	if (pmt == NULL)
	{
		return;
	}

    if (pmt->cbFormat != 0) 
    {
        CoTaskMemFree((PVOID)pmt->pbFormat);
        // Strictly unnecessary but tidier
        pmt->cbFormat = 0;
        pmt->pbFormat = NULL;
    }
    
    if (pmt->pUnk != NULL) 
    {
        pmt->pUnk->Release();
        pmt->pUnk = NULL;
    }
} 

void CGetDeviceInfoDlg::OnBnClickedBtnPath()
{
	TCHAR pszPath[MAX_PATH];  
    BROWSEINFO bi;   
    bi.hwndOwner      = this->GetSafeHwnd();  
    bi.pidlRoot       = NULL;  
    bi.pszDisplayName = NULL;   
    bi.lpszTitle      = TEXT("Choose Folder");   
    bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;  
    bi.lpfn           = NULL;   
    bi.lParam         = 0;  
    bi.iImage         = 0;   
  
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);  
    if (pidl == NULL)  
    {  
        return;  
    }  
  
    if (SHGetPathFromIDList(pidl, pszPath))  
    {  
		GetDlgItem(IDC_EDT_PATH)->SetWindowText(pszPath);
    }
}


void CGetDeviceInfoDlg::OnBnClickedBtnBeginEncode()
{
	CString strPath = _T("");
	GetDlgItem(IDC_EDT_PATH)->GetWindowText(strPath);
	if (strPath.IsEmpty())
	{
		AfxMessageBox(_T("Choose Path to Store"));
		return;
	}
	if (strPath.Right(1) != _T("\\"))
	{
		strPath += _T("\\");
	}

	mCB.m_sSavePath = strPath;
	mCB.m_bEndEncode = FALSE;
	mCB.m_bBeginEncode = TRUE;
}

void CGetDeviceInfoDlg::OnBnClickedBtnEndEncode()
{
	mCB.m_bBeginEncode = FALSE;
	mCB.m_bEndEncode = TRUE;
	mCB.m_bFirst = TRUE;
}


