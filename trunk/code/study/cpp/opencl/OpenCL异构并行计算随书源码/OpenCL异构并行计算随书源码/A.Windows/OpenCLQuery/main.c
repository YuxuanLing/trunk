// OpenCLQuery.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "ocl_query.h"

#include <stdio.h>

#define MAX_LOADSTRING                  100

#define MAX_FEATURE_SHOW_LIST_COUNT     512

// Global Variables:
static HINSTANCE hInst;								// current instance
static HWND hWnd;
static TCHAR szTitle[MAX_LOADSTRING];			    // The title bar text
static TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
static ATOM				MyRegisterClass(HINSTANCE hInstance);
static BOOL				InitInstance(HINSTANCE, int);
static LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
static INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

static HWND sPlatformCombo;
static HWND sDeviceCombo;
static HWND sInfoList;

static const char* sOpenCLFeatureNameList[MAX_FEATURE_SHOW_LIST_COUNT];
static char sOpenCLFeatureDescriptionList[MAX_FEATURE_SHOW_LIST_COUNT][256];


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_OPENCLQUERY, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OPENCLQUERY));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
static ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OPENCLQUERY));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_OPENCLQUERY);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
static BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   // 以640x480尺寸创建窗口，且不含有最大化功能，也不具有尺寸缩放功能
   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
      CW_USEDEFAULT, 0, 640, 480, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


// 将UTF8编码字符串转为宽字符串
static int UTF8ToMultibytes(wchar_t dst[], const char *org)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, org, -1, dst, 0);
    MultiByteToWideChar(CP_UTF8, 0, org, -1, dst, len);
    dst[len] = L'\0';
    return len;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

    int nPlatforms;

    wchar_t buffer[512];

	switch (message)
	{
    case WM_CREATE:
        /** 此消息用于创建窗口中的控件 */

        // 创建选择OpenCL平台提示标签
        UTF8ToMultibytes(buffer, "请选择OpenCL平台");
        CreateWindow(L"STATIC", buffer, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_SIMPLE, 20, 30, 135, 20, hWnd, (HMENU)ID_SELECTION_PLATFORM_LABEL, hInst, NULL);

        // 创建选择OpenCL设备提示标签
        UTF8ToMultibytes(buffer, "请选择OpenCL设备");
        CreateWindow(L"STATIC", buffer, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_SIMPLE, 20, 70, 135, 20, hWnd, (HMENU)ID_SELECTION_DEVICE_LABEL, hInst, NULL);

        // 创建平台选择框
        sPlatformCombo = CreateWindow(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_AUTOHSCROLL, 170, 27, 350, 150, hWnd, (HMENU)ID_SELECTION_PLATFORM_COMBO, hInst, NULL);

        // 创建设备选择框
        sDeviceCombo = CreateWindow(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_AUTOHSCROLL, 170, 67, 350, 150, hWnd, (HMENU)ID_SELECTION_DEVICE_COMBO, hInst, NULL);

        // 创建用于显示OpenCL特征信息的列表
        sInfoList = CreateWindow(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | WS_BORDER | LBS_HASSTRINGS, 20, 120, 580, 300, hWnd, (HMENU)ID_OPENCL_INFO_SHOW_LIST, hInst, NULL);

        // 最后初始化OpenCL平台
        nPlatforms = CL_Query_InitPlatforms();
        if (nPlatforms == 0)
        {
            UTF8ToMultibytes(buffer, "当前环境中没有找到可用的OpenCL平台!");
            MessageBox(hWnd, buffer, L"Warning", MB_OK);
        }
        else
        {
            // 为OpenCL平台选择框添加所有支持的OpenCL平台名
            for (int i = 0; i < nPlatforms; i++)
            {
                UTF8ToMultibytes(buffer, CL_Query_GetPlatformName(i));
                SendMessage(sPlatformCombo, CB_ADDSTRING, 0, (LPARAM)buffer);
            }
        }

        break;

	case WM_COMMAND:
        /** 处理控件消息 */

		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

        if ((HWND)lParam == sPlatformCombo)
        {
            // 处理平台选择框消息
            if (wmEvent == CBN_SELCHANGE)
            {
                int platformIndex = (int)SendMessage(sPlatformCombo, CB_GETCURSEL, 0, 0);
                int nDevices = CL_Query_InitDevices(platformIndex);

                if (nDevices == 0)
                {
                    UTF8ToMultibytes(buffer, "在当前OpenCL平台下没有找到可用的OpenCL设备!");
                    MessageBox(hWnd, buffer, L"Warning", MB_OK);
                }
                else
                {
                    // 先清空设备选择框内的所有内容
                    SendMessage(sDeviceCombo, CB_RESETCONTENT, 0, 0);

                    // 为OpenCL平台选择框添加所有支持的OpenCL平台名
                    for (int i = 0; i < nDevices; i++)
                    {
                        UTF8ToMultibytes(buffer, CL_Query_GetDeviceName(i));
                        SendMessage(sDeviceCombo, CB_ADDSTRING, 0, (LPARAM)buffer);
                    }
                }
            }

            break;
        }
        else if ((HWND)lParam == sDeviceCombo)
        {
            // 处理设备选择框消息
            if (wmEvent == CBN_SELCHANGE)
            {
                // 先清空列表框里的所有内容
                SendMessage(sInfoList, LB_RESETCONTENT, 0, 0);

                int deviceIndex = (int)SendMessage(sDeviceCombo, CB_GETCURSEL, 0, 0);

                int nFeatures = CL_Query_GetDeviceFeatures(deviceIndex, "query.cl", sOpenCLFeatureNameList, sOpenCLFeatureDescriptionList);
                if (nFeatures > MAX_FEATURE_SHOW_LIST_COUNT)
                    nFeatures = MAX_FEATURE_SHOW_LIST_COUNT;

                for (int i = 0; i < nFeatures; i++)
                {
                    char tmpBuffer[512];
                    strcpy(tmpBuffer, sOpenCLFeatureNameList[i]);
                    const int len = (int)strlen(tmpBuffer);
                    int spaces = 50 - len;
                    if (spaces < 4)
                        spaces = 4;
                    for (int i = 0; i < spaces; i++)
                        tmpBuffer[len + i] = ' ';
                    strcpy(&tmpBuffer[len + spaces], sOpenCLFeatureDescriptionList[i]);

                    UTF8ToMultibytes(buffer, tmpBuffer);
                    SendMessage(sInfoList, LB_ADDSTRING, 0, (LPARAM)buffer);
                }
            }

            break;
        }

		// 处理菜单消息
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

