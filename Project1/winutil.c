#include <windows.h>
#include <stdio.h>
#include <ctype.h>
#include <drivinit.h>
#include "dlgopen.h"
#include <stdlib.h>
#include <strsafe.h>
#include <tchar.h>
#include "data_struct.h"
#include <string.h>
#include <ctype.h>


#define MAX_FILES 100
#define MAX_FILENAME_LENGTH 260

extern TCHAR* _fmalloc();

//char* filename_array[MAX_FILES][MAX_FILENAME_LENGTH];
char* filename_array[120];
extern int total_fnames;


TCHAR newfilename[128];
TCHAR newext[128];

OFSTRUCT filestruct;
static TCHAR string[128];
static TCHAR lstring[128];

/*****************************************************************************/
MyOpenFile(lpFileName, lpReOpenBuff, wStyle)
LPSTR lpFileName;           /* Pointer to filename of file to be opened */
LPOFSTRUCT lpReOpenBuff;    /* Pointer to file data structure */
WORD wStyle;                /* Flags */
{
    /* Since OpenFile checks more than the current directory for the specified
    file if the filename is not a full pathname, make sure the filename is
     a full pathname.
     */

    int fh;
    TCHAR* ptr;
    TCHAR full[128];

    StringCchCopy(string, 128, lpFileName);
    for (ptr = string; *ptr != '\0'; ptr++) {
        if (*ptr == ':' || *ptr == '\\') {
            fh = OpenFile(lpFileName, lpReOpenBuff, wStyle);
            return (fh);
        }
    }
    /* not a full pathname, create it */

    sprintf_s(full, 128, ".\\%s", string);
    StringCchCopy(lstring,128, full);
    fh = OpenFile(lstring, lpReOpenBuff, wStyle);
    return (fh);
}

/*****************************************************************************/


check_filename_length(filename)
TCHAR* filename;
{

    /* Find and return the length of the filename, without the extension. */

    int i;

    StringCchCopy(string, 128, filename);
    for (i = 0; string[i] != '\0' && string[i] != '.'; i++);
    string[i] = '\0';
    return (strlen(string));
}

/*****************************************************************************/

add_extension(ext, filename)
TCHAR* ext;                  /* filename default extension */
TCHAR* filename;             /* specified filename */
{

    /* Add the specified extension to the filename if the filename has no
       extension already.
     */

    int i;

    for (i = 0; filename[i] != '\0'; i++)
        if (filename[i] == '.')
            if (filename[i + 1] != '\0') break;
            else {
                strcat_s(filename,128, ext);
                break;
            }
    if (filename[i] != '.' && i <= 8)
        sprintf_s(&filename[i], ".%s", ext);
}

/*****************************************************************************/

force_extension(ext, filename)
TCHAR* ext;                  /* filename default extension */
TCHAR* filename;             /* specified filename */
{

    /* Force the specified extension onto the filename, overwriting any previous
       extension.
     */

    //int i;

    //for (i = 0; filename[i] != '\0' && filename[i] != '.' && i < (sizeof(filename) -5); i++);
    //sprintf_s(filename + i,sizeof(filename) - i, ".%s", ext);

    char* lastDot = strrchr(filename, '.');
    if (lastDot != NULL) {
        size_t remainingSpace = 127 - (lastDot - filename);
        if (strlen(ext) < remainingSpace) {
            strcpy_s(lastDot + 1, remainingSpace, ext);
        }
    }
    else {
        size_t len = strlen(filename);
        if (len + strlen(ext) + 1 < 128) {
            filename[len] = '.';
            strcpy_s(filename + len + 1, 128 - len - 1, ext);
        }
    }
}

/*****************************************************************************/

return_extension(ext, filename)
TCHAR* ext;                  /* current filename extension */
TCHAR* filename;             /* specified filename */
{

    /* Find and retrieve the extension on the filename. */

    //int i, j;

    //for (i = 0; filename[i] != '\0' && filename[i] != '.' && i < 8; i++);
    //if (filename[i] == '.') {
    //    strcpy_s(ext, 128, &filename[i + 1]);
    //    for (j = 0; j < strlen(ext); j++) ext[j] = tolower(ext[j]);
    //}
    //else ext[0] = '\0';

    const char* lastDot = strrchr(filename, '.');
    strncpy(ext, lastDot + 1, 4);
    ext[4] = '\0';
}

/*****************************************************************************/

FILE*
save_file(hWnd, ext, filename, caption)
HWND hWnd;                  /* current window */
TCHAR* ext;                  /* filename default extension */
TCHAR* filename;             /* specified filename */
TCHAR* caption;              /* error message box caption */
{

    /* Generic routine for opening files to save data to.  Check the filename for
       an extension.  If there isn't one, add the default extension.  Pass to the
       open file dialog box the extension so that it can find all files with that
       extension, and the filename so it can display that as the default choice.
       Test for existance of the given filename and allow the user to back out if
       it isn't to be overwritten.  Display an error if the file cannot be opened.
       If everything is OK, return the file handle of the opened file.  Otherwise,
       return NULL.
     */

    int save, cancel, response, fh;
    FILE* fn = NULL;

    add_extension(ext, filename);
    sprintf_s(string, "*.%s", ext);
    do {
        save = TRUE;
        cancel = FALSE;
        if ((fh = DlgOpenFile(hWnd, "Select File Name",
            (LONG)OF_SAVE, string, filename)) != 0) {
            StringCchCopy(lstring, 128, filename);
            if ((fh = MyOpenFile(lstring, &filestruct, OF_EXIST)) > 0) {
                response = MessageBox(GetFocus(),
                    "File Already Exists, Overwrite?", caption, MB_OKCANCEL);
                if (response == IDCANCEL) save = FALSE;
            }
            if (save)
                if ((fn = fopen_s(filename,128, "wt")) == NULL) {
                    MessageBox(GetFocus(), "Cannot open file", caption,
                        MB_ICONASTERISK | MB_OK);
                    save = FALSE;
                }
                else save = TRUE;
        }
        else cancel = TRUE;
    } while (!save && !cancel);
    return (fn);
}

char* mystrlwr(char* str) {
    if (str == NULL) {
        return NULL;
    }
    char* p = str;
    while (*p) {
        *p = tolower((unsigned char)*p);
        p++;
    }
    return str;
}

/*****************************************************************************/

FILE*
retrieve_file(hWnd, ext, filename, caption)
HWND hWnd;                  /* current window */
TCHAR* ext;                  /* filename default extension */
TCHAR* filename;             /* specified filename */
TCHAR* caption;              /* error message box caption */
{

    /* Generic routine for retrieving files.  List all files using default
   extension.  Allow user to cancel out at this point.  If user selects a file,
   try to open it.  If opening the file fails, return NULL, otherwise return
   the file handle.
 */

    //int fh;
    //FILE* fn = NULL;
    //char* str1, * str2;

    //sprintf(string, "*.%s", ext);
    //fh = DlgOpenFile(hWnd, "Select a File", (LONG)OF_OPEN, string, filename);
    //if (fh > 0) {
    //    /*
    //            lstrcpy (lstring,filename);
    //            if ((fh = MyOpenFile (lstring,&filestruct,OF_EXIST)) > 0)
    //    */
    //    if ((fn = fopen(filename, "rt")) == NULL)
    //        MessageBox(GetFocus(), "Cannot Open File", caption, MB_OK);
    //    else;
    //    /*
    //            else MessageBox (GetFocus (),"Cannot Open File",caption,MB_OK);
    //    */
    //}
    ///*
    //    strcpy (filename,lstring);
    //*/
    //str1 = mystrlwr(filename);
    //str2 = strrchr(str1, '\\');
    //if (str2 != NULL) strcpy(filename, ++str2);
    //return (fn);



    OPENFILENAME ofn;
    FILE* fn = NULL;
    char* str1, * str2;
    char file_name_[4096];

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = file_name_;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(file_name_);

    ofn.lpstrFilter = _T("All files\0*.*\0Data Files\0*.dat\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

    GetOpenFileName(&ofn);
    StringCchCopy(filename, 128, ofn.lpstrFile);
    //MessageBox(NULL, filename, "", MB_OK);
    if ((fn = fopen(ofn.lpstrFile, "rt"))  == NULL) {
        MessageBox(GetFocus(), "Cannot open file", caption, MB_OK);
    }

    return (fn);

    //
}


/*****************************************************************************/

HWND
save_file21(hWnd, ext, filename, caption)
HWND hWnd;                  /* current window */
TCHAR* ext;                  /* filename default extension */
TCHAR* filename;             /* specified filename */
TCHAR* caption;              /* error message box caption */
{

    /* First half of generic routine for saving files, for use with modeless
       dialog boxes.  Check the filename for an extension.  If there isn't one
       add the default extension.  Pass to the save file dialog box the extension
       so that it can find all files with that extension, and the filename so it
       can display that as the default choice.  User may cancel out or choose a
       filename.  The filename chosen will be returned in the global variable
       newfilename.  Success or failure will be sent to the window function.
     */

    HWND err;

    add_extension(ext, filename);
    sprintf_s(string, "*.%s", ext);
    err = DlgOpenFile2(hWnd, "Select a File", (LONG)OF_SAVE, string, filename);
    if (err == NULL)
        MessageBox(GetFocus(), "Dialog box not created", caption, MB_OK);
    return (err);
}


/*****************************************************************************/

FILE*
save_file22(filename, caption, err)
TCHAR* filename;
TCHAR* caption;
TCHAR* err;
{

    /* Second half of save_file for use with modeless dialog boxes.  Take the
       filename chosen, which is in the global variable newfilename, and allow
       the user to back out if it isn't to be overwritten.  Display an error if
       the file cannot be opened. If everything is OK, return the file handle of
       the opened file.  Otherwise, return NULL.
     */

    FILE* fn = NULL;


    strcpy_s(lstring, 128, newfilename);
    if ((fn = fopen_s(newfilename, 128, "wt")) == NULL) {
        *err = -1;
        return (NULL);
    }

    TCHAR* lastBackslash = strrchr(newfilename, '\\');
    if (lastBackslash != NULL) {
        strcpy_s(filename, 128, lastBackslash + 1);
    }
    else {
        strcpy_s(filename,128, newfilename);
    }

    err = 0;
    return (fn);
}

/*****************************************************************************/

HWND
retrieve_file21(hWnd, ext, filename, caption)
HWND hWnd;                  /* current window */
TCHAR* ext;                  /* filename default extension */
TCHAR* filename;             /* specified filename */
TCHAR* caption;              /* error message box caption */
{

    /* First half of generic routine for retrieving files, for use with modeless
       dialog boxes.  Use a modeless dialog box to list all files using default
       extension.  User may cancel out or select a file.  The filename selected
       with be returned in the global variable newfilename.  Success or failure
       will be sent to the window function.
     */

    HWND err;

    sprintf_s(string, "*.%s", ext);
    err = DlgOpenFile2(hWnd, "Select a File", (LONG)OF_OPEN, string, filename);
    if (err == NULL)
        MessageBox(GetFocus(), "Dialog box not created", caption, MB_OK);
    return (err);
}

/*****************************************************************************/

HWND
select_file1(hWnd, ext, filename, caption)
HWND hWnd;                  /* current window */
TCHAR* ext;                  /* filename default extension */
TCHAR* filename;             /* specified filename */
TCHAR* caption;              /* error message box caption */
{

    OPENFILENAME ofn;

    char file_name_[4096];

    ZeroMemory(&ofn, sizeof(OPENFILENAME));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = file_name_;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = 4096;
    ofn.lpstrFilter = _T("All files\0*.*\0Data Files\0*.dat\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

    GetOpenFileName(&ofn);
    StringCchCopy(filename, 128, ofn.lpstrFile);




    char* p = file_name_;
    const char* directory = p;
    p += strlen(p) + 1;
    int index_i = 0;
    while (*p) {
        char filePath[MAX_PATH];
        sprintf(filePath, "%s\\%s", directory, p);
        
        filename_array[index_i]=malloc(strlen(filePath) + 1);
        if (filename_array[index_i] != NULL) {
            strcpy(filename_array[index_i], filePath);
        }

        //filename_array[index_i] = filePath;


        index_i++;
        p += strlen(p) + 1;
    }
   
    total_fnames = index_i;

    long ltemp = MAKELONG(TRUE, BN_CLICKED);
    SendMessage(hWnd, WM_COMMAND, DB_MODELESS2, ltemp);

}

/*****************************************************************************/

HDC
print_file(hWnd, landscape)
HWND hWnd;                          /* current graph window */
int landscape;
{

    /* Do all the preliminary stuff for printing. */

    TCHAR pPrintInfo[80];
    LPSTR lpTemp;
    LPSTR lpPrintType;
    LPSTR lpPrintDriver;
    LPSTR lpPrintPort;
    FARPROC lpfnExtDeviceMode;
    DEVMODE FAR* printDM;
    DEVMODE FAR* happyfunctionDM = NULL;
    HANDLE hDriver;
    HDC printDC;
    LONG printDMlen;

    pPrintInfo[0] = '\0';
    GetProfileString("windows", "device", pPrintInfo, pPrintInfo, 80);

    lpTemp = pPrintInfo;
    lpPrintType = pPrintInfo;
    lpPrintDriver = 0;
    lpPrintPort = 0;
    while (*lpTemp) {
        if (*lpTemp == ',') {
            *lpTemp = 0;
            lpTemp++;
            while (*lpTemp == ' ')
                lpTemp++;
            if (!lpPrintDriver)
                lpPrintDriver = lpTemp;
            else {
                lpPrintPort = lpTemp;
                break;
            }
        }
        else
            lpTemp = AnsiNext(lpTemp);
    }

    sprintf_s(lstring, "%s.DRV", lpPrintDriver);
    hDriver = LoadLibrary(lstring);
    lpfnExtDeviceMode = GetProcAddress(hDriver, "ExtDeviceMode");
    if (lpfnExtDeviceMode != NULL) {
        printDMlen = (*lpfnExtDeviceMode) (hWnd, hDriver,
            happyfunctionDM, lpPrintType, lpPrintPort, happyfunctionDM, 0L, 0);
        printDM = (DEVMODE FAR*) malloc(printDMlen);
        printDMlen = (*lpfnExtDeviceMode) (hWnd, hDriver,
            printDM, lpPrintType, lpPrintPort, happyfunctionDM, 0L, DM_COPY);
        if (landscape) printDM->dmOrientation = DMORIENT_LANDSCAPE;
        else printDM->dmOrientation = DMORIENT_PORTRAIT;
        printDC = CreateDC(lpPrintDriver, lpPrintType, lpPrintPort, (LPSTR)printDM);
        free(printDM);
    }
    else printDC = CreateDC(lpPrintDriver, lpPrintType, lpPrintPort, (LPSTR)NULL);
    FreeLibrary(hDriver);
    return (printDC);
}

/*****************************************************************************/

is_OK_float(str)
TCHAR* str;
{

    /* Take the given string and check that it is something atof won't blow up on.
     */

    int i;
    int count = 0;
    int eflag = FALSE;

    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] == 'e' || str[i] == 'E')
            if (eflag) return FALSE;
            else eflag = TRUE;
        else if (eflag) {
            if (isdigit(str[i])) count++;
            if (count == 3) return FALSE;
        }
    }
    return TRUE;
}

/*****************************************************************************/

find_decimal_places(str)
TCHAR* str;
{

    /* Determine the number of decimal places in the string, set the decimal places
       variable to that amount.
     */

    int i, j;
    int decimal_places;

    decimal_places = 0;
    for (i = strlen(str) - 1, j = 0; i >= 0; i--, j++)
        if (str[i] == '.') break;
    if (i >= 0) decimal_places = j;
    return (decimal_places);
}