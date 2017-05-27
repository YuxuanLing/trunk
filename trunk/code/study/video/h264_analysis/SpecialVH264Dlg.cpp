/* 
 * H.264 ·ÖÎöÆ÷
 * H.264 Analysis
 *
 * À×Ïöæè Lei Xiaohua
 * leixiaohua1020@126.com
 * ÖÐ¹ú´«Ã½´óÑ§/Êý×ÖµçÊÓ¼¼Êõ
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 * 
 * H.264ÂëÁ÷·ÖÎö¹¤¾ß
 * H.264 Stream Analysis Tools
 *
 */
#include "stdafx.h"
#include "SpecialVH264.h"
#include "SpecialVH264Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ÓÃÓÚÓ¦ÓÃ³ÌÐò¡°¹ØÓÚ¡±²Ëµ¥ÏîµÄ CAboutDlg ¶Ô»°¿ò

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ¶Ô»°¿òÊý¾Ý
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV Ö§³Ö

// ÊµÏÖ
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


// CSpecialVH264Dlg ¶Ô»°¿ò




CSpecialVH264Dlg::CSpecialVH264Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSpecialVH264Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSpecialVH264Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_V_H264_INPUTURL, m_vh264inputurl);
	DDX_Control(pDX, IDC_V_H264_NALLIST, m_vh264nallist);
	DDX_Control(pDX, IDC_V_H264_NALLIST_MAXNUM, m_vh264nallistmaxnum);
	DDX_Control(pDX, IDC_V_H264_NALINFO, m_vh264nalinfo);
	DDX_Control(pDX, IDC_V_H264_LANG, m_vh264lang);
}

BEGIN_MESSAGE_MAP(CSpecialVH264Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_V_H264_INPUTURL_OPEN, &CSpecialVH264Dlg::OnBnClickedVH264InputurlOpen)
	ON_NOTIFY ( NM_CUSTOMDRAW,IDC_V_H264_NALLIST, OnCustomdrawMyList )
	ON_BN_CLICKED(IDC_V_H264_ABOUT, &CSpecialVH264Dlg::OnBnClickedVH264About)
ON_WM_DROPFILES()
ON_CBN_SELCHANGE(IDC_V_H264_LANG, &CSpecialVH264Dlg::OnSelchangeVH264Lang)
ON_NOTIFY(LVN_ITEMCHANGED, IDC_V_H264_NALLIST, &CSpecialVH264Dlg::OnItemchangedVH264Nallist)
END_MESSAGE_MAP()


// CSpecialVH264Dlg ÏûÏ¢´¦Àí³ÌÐò

BOOL CSpecialVH264Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ½«¡°¹ØÓÚ...¡±²Ëµ¥ÏîÌí¼Óµ½ÏµÍ³²Ëµ¥ÖÐ¡£

	// IDM_ABOUTBOX ±ØÐëÔÚÏµÍ³ÃüÁî·¶Î§ÄÚ¡£
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
	//ÕûÐÐÑ¡Ôñ£»ÓÐ±í¸ñÏß£»±íÍ·£»µ¥»÷¼¤»î
	DWORD dwExStyle=LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP|LVS_EX_ONECLICKACTIVATE;
	//±¨±í·ç¸ñ£»µ¥ÐÐÑ¡Ôñ£»¸ßÁÁÏÔÊ¾Ñ¡ÔñÐÐ
	//ÊÓÆµ
	m_vh264nallist.ModifyStyle(0,LVS_SINGLESEL|LVS_REPORT|LVS_SHOWSELALWAYS);
	m_vh264nallist.SetExtendedStyle(dwExStyle);

	m_vh264nallist.InsertColumn(0,_T("Number"),LVCFMT_CENTER,50,0);
	m_vh264nallist.InsertColumn(1,_T("nal_reference_idc"),LVCFMT_CENTER,100,0);
	m_vh264nallist.InsertColumn(2,_T("NAL Type(nal_unit_type)"),LVCFMT_CENTER,100,0);
	m_vh264nallist.InsertColumn(3,_T("NAL Size(len)"),LVCFMT_CENTER,100,0);
	//---------------------
	m_vh264nallistmaxnum.SetCheck(1);
	nl_index=0;
	//------------
	m_vh264inputurl.EnableFileBrowseButton(
		NULL,
		_T("H.264 Files (*.264,*.h264,*.26l)|*.264;*.h264;*.26l|All Files (*.*)|*.*||")
		);
	// ÉèÖÃ´Ë¶Ô»°¿òµÄÍ¼±ê¡£µ±Ó¦ÓÃ³ÌÐòÖ÷´°¿Ú²»ÊÇ¶Ô»°¿òÊ±£¬¿ò¼Ü½«×Ô¶¯
	//  Ö´ÐÐ´Ë²Ù×÷
	SetIcon(m_hIcon, TRUE);			// ÉèÖÃ´óÍ¼±ê
	SetIcon(m_hIcon, FALSE);		// ÉèÖÃÐ¡Í¼±ê

	// TODO: ÔÚ´ËÌí¼Ó¶îÍâµÄ³õÊ¼»¯´úÂë
	m_vh264lang.InsertString(0,_T("Chinese"));
	m_vh264lang.InsertString(1,_T("English"));

	//_CrtSetBreakAlloc(5819);

	return TRUE;  // ³ý·Ç½«½¹µãÉèÖÃµ½¿Ø¼þ£¬·ñÔò·µ»Ø TRUE
}

void CSpecialVH264Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// Èç¹ûÏò¶Ô»°¿òÌí¼Ó×îÐ¡»¯°´Å¥£¬ÔòÐèÒªÏÂÃæµÄ´úÂë
//  À´»æÖÆ¸ÃÍ¼±ê¡£¶ÔÓÚÊ¹ÓÃÎÄµµ/ÊÓÍ¼Ä£ÐÍµÄ MFC Ó¦ÓÃ³ÌÐò£¬
//  Õâ½«ÓÉ¿ò¼Ü×Ô¶¯Íê³É¡£

void CSpecialVH264Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ÓÃÓÚ»æÖÆµÄÉè±¸ÉÏÏÂÎÄ

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ê¹Í¼±êÔÚ¹¤×÷Çø¾ØÐÎÖÐ¾ÓÖÐ
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// »æÖÆÍ¼±ê
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
	
}

//µ±ÓÃ»§ÍÏ¶¯×îÐ¡»¯´°¿ÚÊ±ÏµÍ³µ÷ÓÃ´Ëº¯ÊýÈ¡µÃ¹â±ê
//ÏÔÊ¾¡£
HCURSOR CSpecialVH264Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSpecialVH264Dlg::OnBnClickedVH264InputurlOpen()
{
	SystemClear();
	CString strFilePath;
	m_vh264inputurl.GetWindowText(strFilePath);
	if(strFilePath.IsEmpty()==TRUE){
		text.LoadString(IDS_INPUTURL_ERROR);
		AfxMessageBox(text);
		return;
	}

#ifdef _UNICODE
	USES_CONVERSION;
	strcpy(fileurl,W2A(strFilePath));
#else
	strcpy_s(fileurl,strFilePath);
#endif
	h264_nal_parse(this,fileurl);
}

//Ìí¼ÓÒ»Ìõ¼ÇÂ¼
//Ã¿¸ö×Ö¶ÎµÄº¬Òå£ºÀàÐÍ£¬Êý¾Ý´óÐ¡£¬Ê±¼ä´Á£¬streamid£¬dataµÄµÚÒ»¸ö×Ö½Ú
//data_lenthÊÇ°üº¬ÆðÊ¼ÂëµÄNAL³¤¶È
//lenÊÇ²»°üº¬ÆðÊ¼ÂëµÄNAL³¤¶È
int CSpecialVH264Dlg::AppendNLInfo(int nal_reference_idc,int nal_unit_type,int len,int data_lenth,int data_offset){
	//Èç¹ûÑ¡ÔñÁË¡°×î¶àÊä³ö5000Ìõ¡±£¬ÅÐ¶ÏÊÇ·ñ³¬¹ý5000Ìõ
	if(m_vh264nallistmaxnum.GetCheck()==1&&nl_index>5000){
		return 0;
	}
	CString temp_index,temp_nal_reference_idc,temp_nal_unit_type,temp_len;
	int nIndex=0;
	switch(nal_unit_type){
	case 1:temp_nal_unit_type.Format(_T("SLICE"));break;
	case 2:temp_nal_unit_type.Format(_T("DPA"));break;
	case 3:temp_nal_unit_type.Format(_T("DPB"));break;
	case 4:temp_nal_unit_type.Format(_T("DPC"));break;
	case 5:temp_nal_unit_type.Format(_T("IDR_SLICE"));break;
	case 6:temp_nal_unit_type.Format(_T("SEI"));break;
	case 7:temp_nal_unit_type.Format(_T("SPS"));break;
	case 8:temp_nal_unit_type.Format(_T("PPS"));break;
	case 9:temp_nal_unit_type.Format(_T("AUD"));break;
	case 10:temp_nal_unit_type.Format(_T("END_SEQUENCE"));break;
	case 11:temp_nal_unit_type.Format(_T("END_STREAM"));break;
	case 12:temp_nal_unit_type.Format(_T("FILLER_DATA"));break;
	case 13:temp_nal_unit_type.Format(_T("SPS_EXT"));break;
	case 19:temp_nal_unit_type.Format(_T("AUXILIARY_SLICE"));break;
	default:temp_nal_unit_type.Format(_T("Other"));break;
	}
	temp_index.Format(_T("%d"),nl_index);

	switch(nal_reference_idc){
	case 0:temp_nal_reference_idc.Format(_T("DISPOS"));break;
	case 1:temp_nal_reference_idc.Format(_T("LOW"));break;
	case 2:temp_nal_reference_idc.Format(_T("HIGH"));break;
	case 3:temp_nal_reference_idc.Format(_T("HIGHEST"));break;
	default:temp_nal_reference_idc.Format(_T("Other"));break;
	}

	temp_len.Format(_T("%d"),len);
	//»ñÈ¡µ±Ç°¼ÇÂ¼ÌõÊý
	nIndex=m_vh264nallist.GetItemCount();
	//¡°ÐÐ¡±Êý¾Ý½á¹¹
	LV_ITEM lvitem;
	lvitem.mask=LVIF_TEXT;
	lvitem.iItem=nIndex;
	lvitem.iSubItem=0;
	//×¢£ºvframe_index²»¿ÉÒÔÖ±½Ó¸³Öµ£¡
	//Îñ±ØÊ¹ÓÃf_indexÖ´ÐÐFormat!ÔÙ¸³Öµ£¡
	lvitem.pszText=temp_index.GetBuffer();
	//------------------------
	//Õâ¸övector¼ÇÂ¼ÁËnalµÄÎ»ÖÃÐÅÏ¢
	//Ê¹ÓÃËüÎÒÃÇ¿ÉÒÔ»ñÈ¡µ½NALµÄÏêÏ¸ÐÅÏ¢
	//ÎÒÃÇÒª´æ´¢°üº¬ÆðÊ¼ÂëµÄ³¤¶È
	//ÆðÊ¼ÂëÔ­±¾²»ÊÇNALµÄÒ»²¿·Ö
	NALInfo nalinfo;
	nalinfo.data_lenth=data_lenth;
	nalinfo.data_offset=data_offset;
	nl_infovector.push_back(nalinfo);
	//------------------------
	m_vh264nallist.InsertItem(&lvitem);
	m_vh264nallist.SetItemText(nIndex,1,temp_nal_reference_idc);
	m_vh264nallist.SetItemText(nIndex,2,temp_nal_unit_type);
	m_vh264nallist.SetItemText(nIndex,3,temp_len);
	nl_index++;
	return TRUE;
}

void CSpecialVH264Dlg::SystemClear(){
	nl_infovector.clear();
	m_vh264nallist.DeleteAllItems();
	nl_index=0;
}
//ListCtrl¼ÓÑÕÉ«
void CSpecialVH264Dlg::OnCustomdrawMyList ( NMHDR* pNMHDR, LRESULT* pResult )
{
	//This code based on Michael Dunn's excellent article on
	//list control custom draw at http://www.codeproject.com/listctrl/lvcustomdraw.asp

	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );

	// Take the default processing unless we set this to something else below.
	*pResult = CDRF_DODEFAULT;

	// First thing - check the draw stage. If it's the control's prepaint
	// stage, then tell Windows we want messages for every item.
	if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		// This is the notification message for an item.  We'll request
		// notifications before each subitem's prepaint stage.

		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )
	{

		COLORREF clrNewTextColor, clrNewBkColor;

		int    nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );

		CString strTemp = m_vh264nallist.GetItemText(nItem,2);
		if(strTemp.Compare(_T("SLICE"))==0){
			clrNewTextColor = RGB(0,0,0);		//Set the text 
			clrNewBkColor = RGB(0,255,255);		//ÇàÉ«
		}
		else if(strTemp.Compare(_T("SPS"))==0){
			clrNewTextColor = RGB(0,0,0);		//text 
			clrNewBkColor = RGB(255,255,0);		//»ÆÉ«
		}
		else if(strTemp.Compare(_T("PPS"))==0){
			clrNewTextColor = RGB(0,0,0);		//text
			clrNewBkColor = RGB(255,153,0);		//¿§·ÈÉ«
		}else if(strTemp.Compare(_T("SEI"))==0){
			clrNewTextColor = RGB(0,0,0);		//text
			clrNewBkColor = RGB(255,66,255);			//·ÛºìÉ«
		}else if(strTemp.Compare(_T("IDR_SLICE"))==0){
			clrNewTextColor = RGB(0,0,0);		//text
			clrNewBkColor = RGB(255,0,0);			//ºìÉ«
		}else{
			clrNewTextColor = RGB(0,0,0);		//text
			clrNewBkColor = RGB(255,255,255);			//°×É«
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;


		// Tell Windows to paint the control itself.
		*pResult = CDRF_DODEFAULT;


	}
}

void CSpecialVH264Dlg::OnBnClickedVH264About()
{
	CAboutDlg dlg;
	dlg.DoModal();
}


void CSpecialVH264Dlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: ÔÚ´ËÌí¼ÓÏûÏ¢´¦Àí³ÌÐò´úÂëºÍ/»òµ÷ÓÃÄ¬ÈÏÖµ
	CDialogEx::OnDropFiles(hDropInfo);
	LPTSTR pFilePathName =(LPTSTR)malloc(MAX_URL_LENGTH);
	::DragQueryFile(hDropInfo, 0, pFilePathName,MAX_URL_LENGTH);  // »ñÈ¡ÍÏ·ÅÎÄ¼þµÄÍêÕûÎÄ¼þÃû£¬×î¹Ø¼ü£¡
	m_vh264inputurl.SetWindowText(pFilePathName);
	::DragFinish(hDropInfo);   // ×¢ÒâÕâ¸ö²»ÄÜÉÙ£¬ËüÓÃÓÚÊÍ·ÅWindows Îª´¦ÀíÎÄ¼þÍÏ·Å¶ø·ÖÅäµÄÄÚ´æ
	free(pFilePathName);
}


void CSpecialVH264Dlg::OnSelchangeVH264Lang()
{
	//ÅäÖÃÎÄ¼þÂ·¾¶
	char conf_path[300]={0};
	//»ñµÃexe¾ø¶ÔÂ·¾¶
	GetModuleFileNameA(NULL,(LPSTR)conf_path,300);//
	//»ñµÃexeÎÄ¼Ò¼ÐÂ·¾¶
	strrchr( conf_path, '\\')[0]= '\0';//
	//_getcwd(realpath,MYSQL_S_LENGTH);
	printf("%s",conf_path);
	strcat_s(conf_path,"\\configure.ini");
	//Ð´ÈëÅäÖÃÎÄ¼þ
	switch(m_vh264lang.GetCurSel()){
	case 0:WritePrivateProfileStringA("Settings","language","Chinese",conf_path);break;
	case 1:WritePrivateProfileStringA("Settings","language","English",conf_path);break;
	default:break;
	}
	//ÖØÆôÈí¼þ
	char exe_path[300]={0};
	//»ñµÃexe¾ø¶ÔÂ·¾¶ 
	GetModuleFileNameA(NULL,(LPSTR)exe_path,300);
	ShellExecuteA( NULL,"open",exe_path,NULL,NULL,SW_SHOWNORMAL);
	OnCancel();
}


void CSpecialVH264Dlg::OnItemchangedVH264Nallist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	 //TODO: ÔÚ´ËÌí¼Ó¿Ø¼þÍ¨Öª´¦Àí³ÌÐò´úÂë
	POSITION ps;
	int nIndex;
	
	ps=m_vh264nallist.GetFirstSelectedItemPosition();
	nIndex=m_vh264nallist.GetNextSelectedItem(ps);
	if(nIndex>=0){
	//----------------------
	int data_offset,data_lenth;
	data_offset=nl_infovector[nIndex].data_offset;
	data_lenth=nl_infovector[nIndex].data_lenth;
	char *outputstr=probe_nal_unit(fileurl,data_offset,data_lenth);
	CString outputstr1;
#ifdef _UNICODE
	USES_CONVERSION;
	outputstr1.Format(_T("%s"),A2W(outputstr));
#else
	outputstr1.Format(_T("%s"),outputstr);
#endif
	
	m_vh264nalinfo.SetWindowText(outputstr1);
}


	*pResult = 0;
}
