// FixEncoding.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "FixEncoding.h"

#define MAX_LOADSTRING 100
#define DEFAULT_CODEPAGE 936 //GB2312

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

//wchar_t szSource[MAX_PATH];
LPTSTR lpSource;


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	//UNREFERENCED_PARAMETER(lpCmdLine);

	//MessageBox(0, lpCmdLine, NULL, 0);
	//_tcscpy_s(szSource, lpCmdLine);
	lpSource = lpCmdLine; // Store the command line string pointer in lpSource

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_FIXENCODING, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FIXENCODING));

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
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FIXENCODING));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_FIXENCODING);
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 300, 200, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

BOOL ConvertMultiByteToUnicode(UINT CodePage)
{
	// Get required buffer size
	int cbMultiByte = WideCharToMultiByte(CP_ACP, 0, lpSource, -1, NULL, 0, NULL, NULL);
	LPSTR lpTemp = (LPSTR)_malloca(cbMultiByte);
	WideCharToMultiByte(CP_ACP, 0, lpSource, -1, lpTemp, cbMultiByte, NULL, NULL);

	// Convert the CodePage encoded str to unicode
	int cchWideChar = MultiByteToWideChar(CodePage, 0, lpTemp, -1, NULL, 0);
	int cbUnicode = cchWideChar * sizeof(WCHAR);
	LPWSTR lpTarget = (LPWSTR)_malloca(cbUnicode);
	MultiByteToWideChar(CodePage, 0, lpTemp, -1, lpTarget, cchWideChar); 
	BOOL fRenamed = MoveFile(lpSource, lpTarget) != 0;
	_freea(lpTemp);
	_freea(lpTarget);
	return fRenamed;
}

BOOL ConvertTextFileToUnicode(UINT CodePage)
{
	int const cbMax = 1024 * 1024;   // Unable to process files larger than 1MB
	HANDLE hFile = CreateFile(lpSource, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return FALSE;
	LPSTR lpSourceText = (LPSTR)_malloca(cbMax);
	DWORD NumberOfBytesRead; 
	ReadFile(hFile, lpSourceText, cbMax, &NumberOfBytesRead, NULL);
	CloseHandle(hFile);

	// Convert the CodePage encoded text to unicode
	int cchWideChar = MultiByteToWideChar(CodePage, 0, lpSourceText, NumberOfBytesRead, NULL, 0);
	int cbUnicode = cchWideChar * sizeof(WCHAR);
	LPWSTR lpTarget = (LPWSTR)_malloca(cbUnicode);
	int nWideCharsWritten = MultiByteToWideChar(CodePage, 0, lpSourceText, NumberOfBytesRead, lpTarget, cchWideChar); 
	_freea(lpSourceText);

	// Backup the source file
	TCHAR szDefaultExt[5] = TEXT(".bak");
	//int nLength = _tcslen(lpSource) + _tcslen(szDefaultExt);
	//LPTSTR lpBakFileName = (LPTSTR)malloc(nLength * sizeof(TCHAR));
	//*lpBakFileName = NULL;
	TCHAR lpBakFileName[MAX_PATH] = TEXT("");
	_tcscat_s(lpBakFileName, MAX_PATH, lpSource);
	_tcscat_s(lpBakFileName, MAX_PATH, szDefaultExt);
	if (MoveFile(lpSource, lpBakFileName) == 0) return FALSE;

	// Write the unicode text
	HANDLE hNewFile = CreateFile(lpSource, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hNewFile == INVALID_HANDLE_VALUE) return FALSE;
	DWORD NumberOfBytesWritten;
	// Write the unicode signature
	const WORD bom = 0xfeff;
	WriteFile(hFile, &bom, sizeof(bom), &NumberOfBytesWritten, NULL);
	WriteFile(hNewFile, lpTarget, nWideCharsWritten * sizeof(WCHAR), &NumberOfBytesWritten, NULL); // Treat lpTarget as ansi type to get the length in byte
	_freea(lpTarget);
	CloseHandle(hNewFile);
	return TRUE;
}

void ShowSuccess(BOOL flag)
{
	int stringID = flag?IDS_COMPLETED:IDS_FAILED;
	TCHAR message[MAX_LOADSTRING];
	LoadString(hInst, stringID, message, MAX_LOADSTRING);
	MessageBox(0, message, TEXT("Result"), MB_ICONINFORMATION);
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;
	static HFONT hFont;

	switch (message)
	{
	case WM_CREATE:
		hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 0, 0, 0, ANTIALIASED_QUALITY, 0, L"Segoeui");
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		//case IDM_ABOUT:
		//	DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
		//	break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_FILE_CONVERT:
			ShowSuccess( ConvertMultiByteToUnicode(DEFAULT_CODEPAGE) );
			break;
		case IDM_FILE_CONVERTTEXTFILE:
			ShowSuccess( ConvertTextFileToUnicode(DEFAULT_CODEPAGE) );
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		SelectObject(hdc, hFont);
		
		GetClientRect(hWnd, &rect);
		DrawText(hdc, lpSource, _tcslen(lpSource), &rect, DT_WORDBREAK);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		DeleteObject(hFont);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
//INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	UNREFERENCED_PARAMETER(lParam);
//	switch (message)
//	{
//	case WM_INITDIALOG:
//		return (INT_PTR)TRUE;
//
//	case WM_COMMAND:
//		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
//		{
//			EndDialog(hDlg, LOWORD(wParam));
//			return (INT_PTR)TRUE;
//		}
//		break;
//	}
//	return (INT_PTR)FALSE;
//}
