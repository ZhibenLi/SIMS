#include <windows.h>
#include <stdlib.h>
#include <TCHAR.h>
#include <tchar.h>
#include "sims.h"
#include "resource.h"
#include "dlgopen.h"
#include <stdio.h>
#include "data_struct.h"

#define GCW_STYLE           (-26)
#define GCL_STYLE           GCW_STYLE
#define GCW_CBWNDEXTRA      (-18)
#define GCL_CBWNDEXTRA      GCW_CBWNDEXTRA
#define GCW_HICON           (-14)
#define GCL_HICON           GCW_HICON
#define GCW_CBCLSEXTRA      (-20)
#define GCL_CBCLSEXTRA      GCW_CBCLSEXTRA
#define GCW_HCURSOR         (-12)
#define GCL_HCURSOR         GCW_HCURSOR
#define GCW_HBRBACKGROUND   (-10)
#define GCL_HBRBACKGROUND   GCW_HBRBACKGROUND

// Global variables
extern HWND hDlg;
extern HWND hRatiosWnd;
extern HWND hEditSetWnd, hEditGraphWnd;
extern HANDLE Files;
extern HANDLE Block[MAXMASSES];

int program;
HWND hwnd;
PROC lpEditWndProc;

//The main window class name
static TCHAR szMainWindowClass[] = _T("SIMS_main_window_class");
static TCHAR szRatioWindowClass[] = _T("SIMS_ratio_window_class");
static TCHAR szEditWindowClass[] = _T("SIMS_edit_window_class");

// The TCHAR that appers in the application's title bar
static TCHAR szMainTitle[] = "SIMS Analysis";
static TCHAR szRatioTitle[] = "SIMS Analysis";
static TCHAR szEditTitle[] = "SIMS Analysis";

// Stored instance handle for use in Win32 API calls such as FindResource
HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
extern LRESULT CALLBACK EditProc(HWND, UINT, WPARAM, LPARAM);
extern LRESULT CALLBACK RatiosProc(HWND, UINT, WPARAM, LPARAM);
extern void batch_ratios();
extern BOOL create_ratios_set_window(HANDLE);
extern BOOL create_edit_windows(HANDLE);

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
) {

	MSG msg;

	int done = FALSE;

	if (hPrevInstance)
		return (FALSE);

	if (!InitApplication(hInstance))
		return (FALSE);

	if (!InitInstance(hInstance, nCmdShow))
		return (FALSE);

	do {
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE)) {
			if (GetMessage(&msg, NULL, NULL, NULL)) {
				if (hDlg == NULL || !IsDialogMessage(hDlg, &msg)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			else done = TRUE;
		}
		else {
			switch (program) {
			case NOPROGRAM:
				break;
			case BATCH_RATIOS:
				batch_ratios();
				break;
			default:
				break;
			}
		}
	} while (!done);

	//while (GetMessage(&msg, NULL, 0, 0))
	//{
	//	TranslateMessage(&msg);
	//	DispatchMessage(&msg);
	//}

	return (int)msg.wParam;
}
/*****************************************************************************/

BOOL InitApplication(hInstance)
HANDLE hInstance;
{
	WNDCLASSEX wcex; //application window class
	WNDCLASSEX ratios_wcex; // window class for rations
	WNDCLASSEX edit_wcex; //window class for edit


	//Main window style and registration
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wcex.lpszClassName = "SimsWClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex)) {
		MessageBox(NULL,
			_T("Call to RegisterMainClassEx failed!"),
			_T("SIMS Main"),
			NULL);

		return 1;
	}



	//ratio window style and registration
	ratios_wcex.cbSize = sizeof(WNDCLASSEX);
	ratios_wcex.style = CS_HREDRAW | CS_VREDRAW;
	ratios_wcex.lpfnWndProc = RatiosProc;
	ratios_wcex.cbClsExtra = 0;
	ratios_wcex.cbWndExtra = 0;
	ratios_wcex.hInstance = hInstance;
	ratios_wcex.hIcon = LoadIcon(ratios_wcex.hInstance, IDI_APPLICATION);
	ratios_wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	ratios_wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	ratios_wcex.lpszMenuName = NULL;
	ratios_wcex.lpszClassName = "RatiosClass";
	ratios_wcex.hIconSm = LoadIcon(ratios_wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&ratios_wcex)) {
		MessageBox(NULL,
			_T("Call to RegisterRatiosClassEx failed!"),
			_T("SIMS Ratios"),
			NULL);
		return FALSE;
	}


	edit_wcex.cbSize = sizeof(WNDCLASSEX);
	edit_wcex.style = CS_HREDRAW | CS_VREDRAW;
	edit_wcex.lpfnWndProc = EditProc;
	edit_wcex.cbClsExtra = 0;
	edit_wcex.cbWndExtra = 0;
	edit_wcex.hInstance = hInstance;
	edit_wcex.hIcon = LoadIcon(edit_wcex.hInstance, IDI_APPLICATION);
	edit_wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	edit_wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	edit_wcex.lpszMenuName = NULL;
	edit_wcex.lpszClassName = "EditClass";
	edit_wcex.hIconSm = LoadIcon(edit_wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&edit_wcex)) {
		MessageBox(NULL,
			_T("Call to RegisterEditClassEx failed!"),
			_T("SIMS Edit"),
			NULL);

		return FALSE;
	}


	return TRUE;
}

/*****************************************************************************/

BOOL InitInstance(hInstance, nCmdShow)
HANDLE hInstance;
int nCmdShow;
{
	// Store instance handle in our global variable
	hInst = hInstance;

	// The parameters to CreateWindowEx explained:
   // WS_EX_OVERLAPPEDWINDOW : An optional extended window style.
   // szWindowClass: the name of the application
   // szTitle: the text that appears in the title bar
   // WS_OVERLAPPEDWINDOW: the type of window to create
   // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
   // 500, 100: initial size (width, length)
   // NULL: the parent of this window
   // NULL: this application does not have a menu bar
   // hInstance: the first parameter from WinMain
   // NULL: not used in this application
	hwnd = CreateWindow(
	"SimsWClass",
		szMainTitle,
		WS_OVERLAPPED | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		800, 600,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hwnd) {
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			_T("SIMS Analysis"),
			NULL);

		return 0;
	}

	// The parameters to ShowWindow explained:
   // hWnd: the value returned from CreateWindow
   // nCmdShow: the fourth parameter from WinMain
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);




	create_editsuperclass(hInstance);


	if ((Files = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 2L)) == NULL)
		MessageBox(NULL, _T("InitInstance : Files GlobalAlloc failed"),
			_T("Error"), MB_OK);


	program = NOPROGRAM;
	hRatiosWnd = NULL;
	hEditSetWnd = NULL;
	return (TRUE);
}

/*****************************************************************************/

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int i;

	switch (message) {
	case WM_COMMAND:
		switch (wParam) {
		case IDM_RATIOS:
			cancel_windows();
			if (!create_ratios_set_window(hInst))
				MessageBox(GetFocus(),
					_T("Creating Ratios Window Failed"),
					_T("SIMS Analysis"), MB_ICONASTERISK | MB_OK);
			else {
				ShowWindow(hRatiosWnd, SW_SHOW);
				UpdateWindow(hRatiosWnd);
				program = NOPROGRAM;
			}
			break;

		case IDM_EDIT:
			cancel_windows();
			if (!create_edit_windows(hInst))
				MessageBox(NULL, _T("Creating Edit Windows Failed"),
					_T("SIMS Analysis"), MB_ICONASTERISK | MB_OK);
			else {
				ShowWindow(hEditSetWnd, SW_SHOW);
				ShowWindow(hEditGraphWnd, SW_SHOW);
				program = NOPROGRAM;
			}
			break;

		case IDM_EXIT:
			for (i = 0; i < MAXMASSES; i++)
				Block[i] == GlobalFree(Block[i]);
			GlobalFree(Files);
			ExitWindows(0L, 0);
			break;

		case IDM_CLOSE:
			for (i = 0; i < MAXMASSES; i++)
				Block[i] == GlobalFree(Block[i]);
			GlobalFree(Files);
			DestroyWindow(hWnd);
			break;

		case IDM_ABOUT:
			
			DialogBox(hInst, MAKEINTRESOURCE(ABOUTBOX), hWnd, About);
			break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return (NULL);
}



/*****************************************************************************/

BOOL CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	/*  PURPOSE:  Processes messages for "About" dialog box */
	BOOL bRet = FALSE;
	switch (message) {
	case WM_INITDIALOG:
		bRet = TRUE;
		break;

	case WM_COMMAND:
		if (wParam == IDOK || wParam == IDCANCEL) {
			EndDialog(hDlg, TRUE);
			bRet = TRUE;
		}
		break;
	}
	return bRet;
}

/*****************************************************************************/



cancel_windows()
{
	/*Check for the existence of some child windows.  If they exist, a sub-
   program module is still active and needs to be closed out.  Call the
   appropriate close out routine.  This clears the way for a new sub-
   program to be run without the user having to cancel before leaving a
   sub-program.*/

	if (IsWindow(hRatiosWnd)) {
		DestroyWindow(hRatiosWnd);
		hRatiosWnd = NULL;
	}

	if (IsWindow(hEditSetWnd)) {
		DestroyWindow(hEditSetWnd);
		DestroyWindow(hEditGraphWnd);
		hEditGraphWnd = NULL;
	}
}



/*****************************************************************************/

create_editsuperclass(hInstance)
HANDLE hInstance;
{

	/* Create a superclass for edit controls.  Create an edit control, then use its
	   characteristics to create a new window class.  Replace the edit control's
	   windowproc with the editsuperclass windowproc, and change the class name.
	   Delete the edit control when finished.
	 */

	HWND hEditControl;
	WNDCLASS class;

	/* Create the edit control to get the information about it. */

	hEditControl = CreateWindow(L"Edit", _T(""), WS_CHILD, 0, 0, 0, 0,
		hwnd,       /* no parent */
		0, hInstance, NULL);

	/* Get the information necessary from the edit class, and create new class.
	 */

	 /* Save the edit class window proc for use later in the new super edit
		window proc.
	  */

	lpEditWndProc = (PROC)GetWindowLong(hEditControl, GWL_WNDPROC);

	/* Register the new superclass of the standard edit control.  It is the
	   same as the original edit control except for the window procedure,
	   hInstance and class name.
	 */

	class.style = GetClassWord(hEditControl, GCW_STYLE);
	class.lpfnWndProc = SuperClassEditProc;
	class.cbClsExtra = GetClassWord(hEditControl, GCW_CBCLSEXTRA);
	class.cbWndExtra = GetClassWord(hEditControl, GCW_CBWNDEXTRA);

	/* Use this application's instance to have the window procedure use the
	   application's DS.  Also, the class will be destroyed when the last
	   instance exits.
	 */

	class.hInstance = hInstance;
	class.hIcon = GetClassWord(hEditControl, GCW_HICON);
	class.hCursor = GetClassWord(hEditControl, GCW_HCURSOR);
	class.hbrBackground = GetClassWord(hEditControl, GCW_HBRBACKGROUND);
	class.lpszMenuName = NULL;          /* no menu */
	class.lpszClassName = (LPSTR)"SuperEdit";

	if (!RegisterClass(&class)) return (FALSE);

	/* Don't need edit control any more. */

	DestroyWindow(hEditControl);

	return (TRUE);
}
/*****************************************************************************/

LRESULT CALLBACK  SuperClassEditProc(hWnd, message, wParam, lParam)
HWND hWnd;
UINT message;
WPARAM wParam;
LPARAM lParam;
{

	/* Processes messages received by the "Super Edit Control" windows.
	   Trap the return key, and clear the control. Pass any other key on to
	   the default edit window proc.  When the control receives the input focus
	   clear the contents, then pass the message on to the default window proc.
	 */

	long result = 0L;
	HWND new_hwnd;
	static int special_edit = FALSE;

	switch (message) {
	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			SetFocus(NULL);
		}
		else CallWindowProc(lpEditWndProc, hWnd, message, wParam, lParam);
		break;

	case WM_SETFOCUS:
		SetWindowText(hWnd, _T(""));

	default:
		result = CallWindowProc(lpEditWndProc, hWnd, message, wParam, lParam);
		break;
	}
	return (result);
}
