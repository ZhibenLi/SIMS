#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <strsafe.h>
//#include <Shlwapi.h>
#include "dlgopen.h"
#include "data_struct.h"
#include "resource.h"
#include <tchar.h>


HWND hDlg;
HWND hDlgParent;

HANDLE Files;
struct fname_record* filenames;
int total_fnames;

static TCHAR          szExt[128];
static TCHAR          szFileName[128];
static TCHAR          FileNameIn[128];
static TCHAR          szTitle[128];
static DWORD        flags;
static int          item_count;
static int          index[MAXINDICES];
static int          OK_clicks;
static TCHAR          string[80];

// forward declarations of helper functions
extern int changedir(const TCHAR*);
static void DlgCheckOkEnable(HWND, int, UINT);
static TCHAR* FillListBox(HWND, TCHAR*);


extern TCHAR  newfilename[128];
extern TCHAR  newext[5];


extern int changedir(const TCHAR* path) {
	// Check if a drive letter is specified
	if (_tcslen(path) > 2 && path[1] == ':') {
		// On Windows, set the drive as well
		TCHAR  drive[3] = { path[0], path[1], '\0' };
		if (_chdrive(tolower(drive[0]) - 'a' + 1) != 0) {
			return 0; // Error changing drive
		}
		path += 2; // Move past the drive letter and colon
	}

	// Change to directory
	if (_wchdir(path) != 0) {
		return 0; // Error changing directory
	}

	return 1; // Success
}



/*******************************************************************************
 *
 *  FUNCTION: DlgOpenFile ()
 *
 *  PURPOSE: Display dialog box for opening files. Allow user to interact
 *        with dialogbox, change directories as necessary, and try to
 *        open file if user selects one. Automatically append
 *        extension to filename if necessary.
 *        This routine correctly parses filenames containing KANJI
 *        characters.
 *
 *  RETURNS:  - File name chosen in parameter szFileNameIn
 *            - 0 if user presses <cancel>
 *            - 1 if filename entered is illegal
 *
 ******************************************************************************/

int WINAPI DlgOpenFile(HWND hwndParent, TCHAR* szTitleIn, DWORD flagsIn, TCHAR* szExtIn, TCHAR* szFileNameIn) {

	if (!szFileNameIn || !szTitleIn || !szExtIn) {
		// Handle null input pointers
		return -1;
	}

	int i, fh;
	flags = flagsIn;
	if (szFileNameIn[1] == ':') {
		for (i = strlen(szFileNameIn); szFileNameIn[i] != '\\' && szFileNameIn[i] != ':'; i--);
		StringCchCopy(FileNameIn, 128, &szFileNameIn[i + 1]);
		if (szFileNameIn[i] == ':') {
			szFileNameIn[i + 1] = '\0';
		}
		else {
			szFileNameIn[i] = '\0';
		}
		StringCchCopy(szFileName, 128, szFileNameIn);
	}
	else {
		StringCchCopy(szFileName, 128, szFileNameIn);
		StringCchCopy(FileNameIn, 128, szFileNameIn);
	}
	StringCchCopy(szExt, 128, szExtIn);
	StringCchCopy(szTitle, 128, szTitleIn);

	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwndParent, GWLP_HINSTANCE);

	fh = DialogBox(hInstance, MAKEINTRESOURCE(DLGOPENBOX), hwndParent, DlgfnOpen);

	if (fh != 0) lstrcpy(szFileNameIn, szFileName);
	return (int)fh;
}


/****************************************************************************
 *
 *  FUNCTION: DlgfnOpen (hwnd, msg, wParam, lParam)
 *
 *  PURPOSE: Dialog function for File/Open dialog.
 *
 ****************************************************************************/

LRESULT CALLBACK DlgfnOpen(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int result = -1; //assume illegal filename initially
	WORD w;
	TCHAR c;
	RECT rc, rcCtl;
	HWND hwndT;

	switch (msg) {
	case WM_INITDIALOG:
		if (szTitle[0] != NULL) {
			SetWindowText(hwnd, szTitle);
		}

		/* Set text on <OK> button according to mode (File/Open or File/Save) */

		if (flags & OF_SAVE) {
			SetDlgItemText(hwnd, IDOK, "&Save");
			OK_clicks = 1;
		}
		if (flags & OF_OPEN) {
			SetDlgItemText(hwnd, IDOK, "&Open");
		}
		if (hwndT = GetDlgItem(hwnd, DLGOPEN_SMALL)) {
			GetWindowRect(hwnd, &rc);
			GetWindowRect(GetDlgItem(hwnd, DLGOPEN_SMALL), &rcCtl);
			SetWindowPos(hwnd, NULL, 0, 0, rcCtl.left - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE);
		}

		/* fill list box with filenames that match specifications, and
		 * fill static field with path name.
		 */
		if (szFileName[1] == ':') {
			if (changedir(szFileName)) {
				StringCchCopy(szFileName, 128, szExt);
			}
			/*  Try to open path.  If successful, fill listbox with
			*  contents of new directory.    Otherwise, open datafile.
			*/
			if (FSearchSpec(szFileName)) {
				StringCchCopy(szExt, 128, FillListBox(hwnd, szFileName));
				if (flags & OF_NOSHOWSPEC) {
					SetDlgItemText(hwnd, DLGOPEN_EDIT, "");
				}
				else {
					SetDlgItemText(hwnd, DLGOPEN_EDIT, szExt);
				}
			}
		}
		else FillListBox(hwnd, szExt);

		/* If in Save mode, set the edit control with default (current)
		 * file name,and select the corresponding entry in the listbox.
		*/
		if ((flags & OF_SAVE) && *FileNameIn) {
			SetDlgItemText(hwnd, DLGOPEN_EDIT, FileNameIn);
			SendDlgItemMessage(hwnd, DLGOPEN_FILE_LISTBOX, LB_SELECTSTRING, 0, (LPARAM)FileNameIn);
		}
		else {
			// set the edit field with the default extensions
			if (flags & OF_NOSHOWSPEC) {
				SetDlgItemText(hwnd, DLGOPEN_EDIT, "");
			}
			else {
				SetDlgItemText(hwnd, DLGOPEN_EDIT, szExt);
			}
		}
		// and select all text in the edit field
		SendDlgItemMessage(hwnd, DLGOPEN_EDIT, EM_SETSEL, 0, 0x7FFF0000L);
		break;

	case WM_COMMAND:
		w = (LOWORD(wParam));
		switch (w) {
		case IDOK:
			if (IsWindowEnabled(GetDlgItem(hwnd, IDOK))) {
				/* Get contents of edit field and add search spec. if it
				 * does not contain one.
				 */
				GetDlgItemText(hwnd, DLGOPEN_EDIT, (LPWSTR)szFileName, 128);

				w = lstrlen(szFileName) - 1;
				c = szFileName[w];
				switch (c) {
				case '\\':
				case '/':
					szFileName[w] = 0;
					break;
				}
				if (changedir(szFileName)) {
					StringCchCopy(szFileName, 128, szExt);
					if (flags & OF_SAVE) {
						OK_clicks = 0;
					}
				}
				/*  Try to open path.  If successful, fill listbox with
				 *  contents of new directory.    Otherwise, open datafile.
				 */
				if (FSearchSpec(szFileName)) {
					StringCchCopy(szExt, 128, FillListBox(hwnd, szFileName));
					if ((flags & OF_SAVE) && *FileNameIn) {
						StringCchCopy(szFileName, 128, FileNameIn);
					}
					if (flags & OF_NOSHOWSPEC) {
						SetDlgItemText(hwnd, DLGOPEN_EDIT, szFileName);
					}
					else if ((flags & OF_SAVE) && *FileNameIn) {
						SetDlgItemText(hwnd, DLGOPEN_EDIT, szFileName);
						OK_clicks++;
					}
					else {
						SetDlgItemText(hwnd, DLGOPEN_EDIT, szExt);
					}
					break;
				}
				else if (flags & OF_SAVE) {
					OK_clicks++;
				}
				if ((flags & OF_SAVE) && OK_clicks != 2) break;
				result = 1;
				EndDialog(hwnd, result);
			}
			break;
		case IDCANCEL:
			/* User pressed cancel.  Just take down dialog box. */
			EndDialog(hwnd, 0);
			break;

			/*  User single clicked or doubled clicked in listbox -
			 *  Single click means fill edit box with selection.
			 *  Double click means go ahead and open the selection.
			 */
		case DLGOPEN_FILE_LISTBOX:
			switch (HIWORD(lParam)) {
				//single click case
			case 1:
				/* Get selection, which may be either a prefix to a
				 * new search path or a filename. DlgDirSelect parses
				 * selection, and appends a backslash if selection
				 * is a prefix
				 */
				DlgDirSelectEx(hwnd, szFileName, sizeof(szFileName) / sizeof(wchar_t), wParam);
				w = lstrlen(szFileName) - 1;
				c = szFileName[w];
				switch (c) {
				case ':':
					StringCchCat(szFileName, 128, TEXT("."));
					break;
				}
			case '\\':
			case '/':
				szFileName[w] = 0;
				break;

				SetDlgItemText(hwnd, DLGOPEN_EDIT, szFileName);
				break;
				/* Double click case - first click has already been
				 * processed as single click
				*/
			case 2:
				PostMessage(hwnd, WM_COMMAND, IDOK, 0L);
				break;
			}
			break;

		case DLGOPEN_EDIT:
			DlgCheckOkEnable(hwnd, DLGOPEN_EDIT, HIWORD(lParam));
			break;
		}
	default:
		return FALSE;
	}
	return TRUE;
}


/*******************************************************************************
 *
 *  FUNCTION: DlgOpenFile2 ()
 *
 *  COMMENTS: - Sets up a modeless dialogbox.
 *
 *  RETURNS:  - Uses SendMessage to return success or failure.
 *            - chosen file name returned through global variable newfilename.
 *
 ******************************************************************************/

HWND DlgOpenFile2(HWND hwndParent, TCHAR* szTitleIn, DWORD flagsIn, TCHAR* szExtIn, TCHAR* szFileNameIn) {
	int i;
	hDlgParent = hwndParent;

	flags = flagsIn;
	if (szFileNameIn[1] == ':') {
		for (i = _tcslen(szFileNameIn); szFileNameIn[i] != '\\' && szFileNameIn[i] != ':'; i--);
		StringCchCopy(FileNameIn, 128, &szFileNameIn[i + 1]);
		if (szFileNameIn[i] == ':') szFileNameIn[i + 1] = '\0';
		else szFileNameIn[i] = '\0';
		StringCchCopy(szFileName, 128, szFileNameIn);
	}
	else {
		StringCchCopy(szFileName, 128, szFileNameIn);
		StringCchCopy(FileNameIn, 128, szFileNameIn);
	}
	StringCchCopy(szExt, 128, szExtIn);
	StringCchCopy(szTitle, 128, szTitleIn);

	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwndParent, GWLP_HINSTANCE);

	hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(DLGOPENBOX2), hwndParent, DlgfnOpen2);
	return hDlg;
}

/****************************************************************************
 *
 *  FUNCTION: DlgfnOpen2 (hwnd, msg, wParam, lParam)
 *
 *  PURPOSE: Dialog function for File/Open dialog.
 *
 ****************************************************************************/

LRESULT CALLBACK DlgfnOpen2(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int result = -1;
	WORD w;
	TCHAR c;
	OFSTRUCT of;
	RECT rc, rcCtl;
	HWND hwndT;
	BOOL fEnable;

	switch (msg) {
	case WM_INITDIALOG:
		if (szTitle[0] != NULL) {
			SetWindowText(hwnd, szTitle);
		}
		/* Set text on <OK> button according to mode (File/Open or File/Save) */

		if (flags & OF_SAVE) {
			SetDlgItemText(hwnd, IDOK, "&Save");
			OK_clicks = 1;
		}
		if (flags & OF_OPEN) {
			SetDlgItemText(hwnd, IDOK, "&Open");
		}
		if (hwndT = GetDlgItem(hwnd, DLGOPEN_SMALL)) {
			GetWindowRect(hwnd, &rc);
			GetWindowRect(GetDlgItem(hwnd, DLGOPEN_SMALL), &rcCtl);
			SetWindowPos(hwnd, NULL, 0, 0, rcCtl.left - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE);
		}

		/* fill list box with filenames that match specifications, and
		 * fill static field with path name.
		 */
		if (szFileName[1] == ':') {
			if (changedir(szFileName)) {
				StringCchCopy(szFileName, 128, szExt);
			}
			/*  Try to open path.  If successful, fill listbox with
			 *  contents of new directory.    Otherwise, open datafile.
			 */
			if (FSearchSpec(szFileName)) {
				StringCchCopy(szExt, 128, FillListBox(hwnd, szFileName));
				if (flags & OF_NOSHOWSPEC) {
					SetDlgItemText(hwnd, DLGOPEN_EDIT, "");
				}
				else {
					SetDlgItemText(hwnd, DLGOPEN_EDIT, szExt);
				}
			}
		}
		else FillListBox(hwnd, szExt);
		/* If in Save mode, set the edit control with default (current)
		 * file name,and select the corresponding entry in the listbox.
		 */

		if ((flags & OF_SAVE) && *FileNameIn) {
			SetDlgItemText(hwnd, DLGOPEN_EDIT, FileNameIn);
			SendDlgItemMessage(hwnd, DLGOPEN_FILE_LISTBOX, LB_SELECTSTRING, 0, (LPARAM)FileNameIn);
		}
		else {
			//set the edit field with the default extensions
			if (flags & OF_NOSHOWSPEC) {
				SetDlgItemText(hwnd, DLGOPEN_EDIT, "");
			}
			else {
				SetDlgItemText(hwnd, DLGOPEN_EDIT, szExt);
			}
		}
		// select all text in the edit field
		SendDlgItemMessage(hwnd, DLGOPEN_EDIT, EM_SETSEL, 0, 0x7FFF0000L);
		break;
	case WM_COMMAND:
		w = wParam;
		switch (w) {
		case IDOK:
			if (IsWindowEnabled(GetDlgItem(hwnd, IDOK))) {
				/* Get contents of edit field and add search spec. if it
				 * does not contain one.
				 */
				GetDlgItemText(hwnd, DLGOPEN_EDIT, (LPWSTR)szFileName, 128);

				w = lstrlen(szFileName) - 1;
				c = szFileName[w];
				switch (c) {
				case '\\':
				case '/':
					szFileName[w] = 0;
					break;
				}
				if (changedir(szFileName)) {
					StringCchCopy(szFileName, 128, szExt);
					if (flags & OF_SAVE) {
						OK_clicks = 0;
					}
				}
				/*  Try to open path.  If successful, fill listbox with
				 *  contents of new directory.    Otherwise, open datafile.
				*/
				if (FSearchSpec(szFileName)) {
					StringCchCopy(szExt, 128, FillListBox(hwnd, szFileName));
					if ((flags & OF_SAVE) && *FileNameIn) {
						StringCchCopy(szFileName, 128, FileNameIn);
					}
					if (flags & OF_NOSHOWSPEC) {
						SetDlgItemText(hwnd, DLGOPEN_EDIT, "");
					}
					else if ((flags & OF_SAVE) && *FileNameIn) {
						SetDlgItemText(hwnd, DLGOPEN_EDIT, szFileName);
						OK_clicks++;
					}
					else {
						SetDlgItemText(hwnd, DLGOPEN_EDIT, szExt);
					}
					break;
				}
				else if (flags & OF_SAVE) {
					OK_clicks++;
				}

				if ((flags & OF_SAVE) && OK_clicks != 2) break;
				result = 1;
				StringCchCopy(newfilename, 128, szFileName);
				StringCchCopy(newext, 5, szExt);
				if (flags & OF_SAVE)
					SendMessage(hDlgParent, WM_COMMAND, DB_MODELESS_SAVE, (DWORD)TRUE);
				if (flags & OF_SAVE)
					SendMessage(hDlgParent, WM_COMMAND, DB_MODELESS_OPEN, (DWORD)TRUE);
				DestroyWindow(hwnd);
				return TRUE;
			}
			break;

		case IDCANCEL:
			// user pressed cancel, just take down dialog box
			if (flags & OF_SAVE)
				SendMessage(hDlgParent, WM_COMMAND, DB_MODELESS_SAVE, (DWORD)FALSE);
			if (flags & OF_OPEN)
				SendMessage(hDlgParent, WM_COMMAND, DB_MODELESS_OPEN, (DWORD)FALSE);
			DestroyWindow(hwnd);
			return TRUE;
			break;

			/*  User single clicked or doubled clicked in listbox -
			 *  Single click means fill edit box with selection.
			 *  Double click means go ahead and open the selection.
			 */
		case DLGOPEN_FILE_LISTBOX:
		case DLGOPEN_DIR_LISTBOX:
			switch (HIWORD(lParam)) {
			case 1:
				/* Get selection, which may be either a prefix to a
				 * new search path or a filename. DlgDirSelect parses
				 * selection, and appends a backslash if selection
				 * is a prefix
				 */
				DlgDirSelectEx(hwnd, szFileName, sizeof(szFileName) / sizeof(wchar_t), wParam);
				w = lstrlen(szFileName) - 1;
				c = szFileName[w];
				switch (c) {
				case ':':
					StringCchCat(szFileName, 128, TEXT("."));
					break;
				case '\\':
				case '/':
					szFileName[w] = 0;
					break;
				}
				SetDlgItemText(hwnd, DLGOPEN_EDIT, szFileName);
				break;

				/* Double click case - first click has already been
				 * processed as single click
				 */
			case 2:
				PostMessage(hwnd, WM_COMMAND, IDOK, 0L);
				break;
			}
			break;

		case DLGOPEN_EDIT:
			DlgCheckOkEnable(hwnd, DLGOPEN_EDIT, HIWORD(lParam));
			break;
		}
		break;

	case WM_DESTROY:
		SendMessage(hDlgParent, WM_COMMAND, DB_DESTROY, (DWORD)TRUE);
		hDlg = NULL;
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


/*******************************************************************************
 *
 *  FUNCTION: DlgChooseFiles ()
 *
 *  COMMENTS: - Sets up a modeless dialogbox, to allow the user to select a
 *              list of files, choosing them from one list box and listing
 *              them in another.
 *
 *  RETURNS:  - Uses SendMessage to return success or failure.
 *
 ******************************************************************************/

HWND DlgChooseFiles(HWND hwndParent, TCHAR* szTitleIn, DWORD flagsIn, TCHAR* szExtIn, TCHAR* szFileNameIn) {
	hDlgParent = hwndParent;
	flags = flagsIn;

	StringCchCopy(szFileName, 128, szFileNameIn);
	StringCchCopy(szExt, 128, szExtIn);
	StringCchCopy(szTitle, 128, szTitleIn);

	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwndParent, GWLP_HINSTANCE);

	WNDPROC dlgProc = (WNDPROC)DlgfnChoose;
	hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(DLGCHOOSEBOX), hwndParent, dlgProc);

	return hDlg;
}


/****************************************************************************
 *
 *  FUNCTION: DlgfnChoose (hwnd, msg, wParam, lParam)
 *
 *  PURPOSE: Dialog function for Chooseing Files dialog.
 *
/****************************************************************************/
LRESULT CALLBACK DlgfnChoose(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int result = -1;
	int i, j;
	WORD w;
	TCHAR c;
	RECT rc, rcCtl;
	HWND hwndT;
	long     ltemp;

	switch (msg) {
	case WM_INITDIALOG:
		if (szTitle[0] != NULL)
			SetWindowText(hwnd, szTitle);

		if (hwndT = GetDlgItem(hwnd, DLGOPEN_SMALL)) {
			GetWindowRect(hwnd, &rc);
			GetWindowRect(GetDlgItem(hwnd, DLGOPEN_SMALL), &rcCtl);
			SetWindowPos(hwnd, NULL, 0, 0, rcCtl.left - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE);
		}

		FillListBox(hwnd, szExt);

		SetDlgItemText(hwnd, DLGOPEN_EDIT, szExt);
		break;

	case WM_COMMAND:
		w = wParam;
		switch (w) {
		case IDOK:
			if (IsWindowEnabled(GetDlgItem(hwnd, IDOK))) {
				GetDlgItemText(hwnd, DLGOPEN_EDIT, (LPWSTR)szFileName, 128);

				w = lstrlen(szFileName) - 1;
				c = szFileName[w];

				switch (c) {
				case'\\':
				case '/':
					szFileName[w] = 0;
					break;
				}
				if (changedir(szFileName))
					StringCchCopy(szFileName, 128, szExt);
				if (FSearchSpec(szFileName)) {
					StringCchCopy(szExt, 128, FillListBox(hwnd, szFileName));
					if (flags & OF_NOSHOWSPEC)
						SetDlgItemText(hwnd, DLGOPEN_EDIT, "");
					else
						SetDlgItemText(hwnd, DLGOPEN_EDIT, szExt);
					break;
				}

				StringCchCopy(newext, 5, szExt);
				total_fnames = SendDlgItemMessage(hwnd, DLGOPEN_FILE_LISTBOX, LB_GETSELITEMS, MAXINDICES, (DWORD)(LPSTR)index);
				if (GlobalReAlloc(Files, 0L, GMEM_MODIFY | GMEM_DISCARDABLE) == NULL) {
					MessageBox(NULL, _T("DlgChoose: Files GlobalReAlloc1 failed"),
						_T("Error"), MB_OK);
					DestroyWindow(hwnd);
					ltemp = MAKELONG(FALSE, -1);
					SendMessage(hDlgParent, WM_COMMAND, DB_MODELESS2, ltemp);
					return TRUE;
				}
				if (GlobalDiscard(Files) == NULL) {
					MessageBox(NULL, _T("DlgChoose: Files GlobalDiscard failed"),
						_T("Error"), MB_OK);
					DestroyWindow(hwnd);
					ltemp = MAKELONG(FALSE, -1);
					SendMessage(hDlgParent, WM_COMMAND, DB_MODELESS2, ltemp);
					return TRUE;
				}
				if ((Files = GlobalReAlloc(Files,
					(DWORD)sizeof(struct fname_record) * total_fnames,
					GMEM_MOVEABLE | GMEM_ZEROINIT)) == NULL) {
					MessageBox(NULL, _T("DlgChoose: Files GlobalReAlloc2 failed"),
						_T("Error"), MB_OK);
					DestroyWindow(hwnd);
					ltemp = MAKELONG(FALSE, -1);
					SendMessage(hDlgParent, WM_COMMAND, DB_MODELESS2, ltemp);
					return TRUE;
				}
				for (i = 0; i < total_fnames; i++) {
					SendDlgItemMessage(hwnd, DLGOPEN_FILE_LISTBOX,
						LB_GETTEXT, index[i], (DWORD)(LPSTR)filenames[i].fname);
				}
				GlobalUnlock(Files);

				DestroyWindow(hwnd);
				ltemp = MAKELONG(TRUE, BN_CLICKED);
				SendMessage(hDlgParent, WM_COMMAND, DB_MODELESS2, ltemp);
				return TRUE;
			}
			break;

		case IDCANCEL:

			ltemp = MAKELONG(FALSE, BN_CLICKED);
			SendMessage(hDlgParent, WM_COMMAND, DB_MODELESS2, ltemp);
			DestroyWindow(hwnd);
			return TRUE;
			break;

		case DLGOPEN_DIR_LISTBOX:
			switch (HIWORD(wParam)) {
				/* Single click case */
			case LBN_SELCHANGE:
				/* Get selection, which is a new search path.
				 * DlgDirSelect parses the selection, and appends a
				 * backslash if selection is a prefix.
				 */
				DlgDirSelectEx(hwnd, szFileName, sizeof(szFileName) / sizeof(TCHAR), wParam);
				w = lstrlen(szFileName) - 1;
				int length = lstrlen(szFileName);
				c = szFileName[length-1];
				switch (c) {
				case ':':
					lstrcat(szFileName, _T("."));
					break;
				case '\\':
				case '/':
					szFileName[w] = 0;
					break;
				}
				SetDlgItemText(hwnd, DLGOPEN_EDIT, szFileName);
				break;
				/* Double click case - first click has already been
				 * processed as single click
				 */
			case LBN_DBLCLK:
				PostMessage(hwnd, WM_COMMAND, IDOK, 0L);
				break;
			}
			break;

		case DLGOPEN_FILE_LISTBOX:
			switch (HIWORD(lParam)) {
				/* Single click case */
			case LBN_SELCHANGE:
				/* Get selection, which is a filename.  Copy that
				 * filename to the selected files listbox.

				 */
				SendDlgItemMessage(hwnd, DLGOPEN_FILE_LISTBOX,
					LB_GETSELITEMS, MAXINDICES, (DWORD)(LPSTR)index);
				SendDlgItemMessage(hwnd, DLGOPEN_FILE_LISTBOX,
					LB_GETTEXT, index[0], (DWORD)(LPSTR)szFileName);
				SetDlgItemText(hwnd, DLGOPEN_EDIT, szFileName);
				break;

				/* Double click case - first click has already been
				 * processed as single click
				 */
			case LBN_DBLCLK:
				break;
			}
			break;

		case DLGOPEN_EDIT:
			DlgCheckOkEnable(hwnd, DLGOPEN_EDIT, HIWORD(lParam));
			break;
		}
		break;

	case WM_DESTROY:
		SendMessage(hDlgParent, WM_COMMAND, DB_DESTROY, (DWORD)TRUE);
		hDlg = NULL;
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/****************************************************************************
 *  FUNCTION: FSearchSpec (sz)
 *  PURPOSE: Checks to see if NULL-terminated strings contains a "*" or
 *           a "?".
 *  RETURNS: TRUE  - if the above characters are found in the string
 *           FALSE - otherwise.
 ****************************************************************************/

static BOOL FSearchSpec(TCHAR* sz)
{
	while (*sz) {
		if (*sz == '*' || *sz == '?') {
			return TRUE;
		}
		sz++;
	}
	return FALSE;
}


/****************************************************************************
 *  FUNCTION: static TCHAR * NEAR FillListBox (hDlg,pFile)
 *  PURPOSE: Fill list box with filenames that match specifications, and
 *           fills the static field with the path name.
 *  RETURNS: A pointer to the pathname.
 ****************************************************************************/

static TCHAR* FillListBox(HWND hDlg, TCHAR* pFile)
{
	TCHAR  ach[20];
	TCHAR* pch;
	TCHAR* pDir;    /* Directory name or path */

	pch = pFile;
	pDir = ach;

	while (*pch && *pch != ';')
		pch++;
	while ((pch > pFile) && (*pch != '/') && (*pch != '\\'))
		pch--;
	if (pch > pFile) {
		*pch = 0;
		StringCchCopy(pDir, 20, pFile);
		pFile = pch + 1;
	}
	else {
		StringCchCopy(pDir, 20, _T("."));
	}

	DlgDirList(hDlg, pDir, DLGOPEN_DIR_LISTBOX, DLGOPEN_PATH, ATTRDIRLIST);
	SendDlgItemMessage(hDlg, DLGOPEN_FILE_LISTBOX, LB_RESETCONTENT, 0, 0L);
	SendDlgItemMessage(hDlg, DLGOPEN_FILE_LISTBOX, WM_SETREDRAW, FALSE, 0L);
	pDir = pFile;         /* save pFile to return */
	while (*pFile) {
		pch = ach;
		while (*pFile == ' ')
			pFile++;
		while (*pFile && *pFile != ';')
			*pch++ = *pFile++;
		*pch = 0;
		if (*pFile)
			pFile++;
		SendDlgItemMessage(hDlg,
			DLGOPEN_FILE_LISTBOX,
			LB_DIR, ATTRFILELIST,
			(LPARAM)ach);
	}
	SendDlgItemMessage(hDlg, DLGOPEN_FILE_LISTBOX, WM_SETREDRAW, TRUE, 0L);
	InvalidateRect(GetDlgItem(hDlg, DLGOPEN_FILE_LISTBOX), NULL, TRUE);

	item_count = SendDlgItemMessage(hDlg, DLGOPEN_FILE_LISTBOX, LB_GETCOUNT, 0, 0L);
	return pDir;
}


/****************************************************************************
 *  FUNCTION: static void NEAR DlgCheckOkEnable(hwnd, idEdit, message)
 *  PURPOSE: Enables the <OK> button in a dialog box iff the edit item
 *           contains text.
 ****************************************************************************/

static void DlgCheckOkEnable(HWND hwnd, int idEdit, UINT message)
{
	if (message == EN_CHANGE) {
		BOOL hasText = SendMessage(GetDlgItem(hwnd, idEdit), WM_GETTEXTLENGTH, 0, 0L) > 0;
		EnableWindow(GetDlgItem(hwnd, IDOK), hasText);
	}
}
