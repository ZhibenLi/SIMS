
#define MAXINDICES     200



/* ID for modeless dialog boxes */

#define DB_MODELESS_OPEN        501
#define DB_MODELESS_SAVE        502
#define DB_MODELESS2            503
#define DB_DESTROY              504

/* IDs for controls in the DlgOpen dialog */

#define DLGOPEN_EDIT            101
#define DLGOPEN_FILE_LISTBOX    102
#define DLGOPEN_DIR_LISTBOX     103
#define DLGOPEN_PATH            104
#define DLGOPEN_TEXT            105
#define DLGOPEN_BIG             107
#define DLGOPEN_SMALL           108
#define DLGCHOOSE_FILE_LISTBOX  109

/*  flags:
 *     The LOWORD is the standard FileOpen() flags (OF_*)
 *     the HIWORD can be any of the following:
 */
#define OF_MUSTEXIST    0x00010000  /* file must exist if the user hits Ok */
#define OF_NOSHOWSPEC   0x00020000  /* DO NOT Show search spec in the edit box*/
#define OF_SHOWSPEC     0x00000000  /* Show the search spec in the edit box */
#define OF_SAVE         0x00040000  /* Ok button will say "Save" */
#define OF_OPEN         0x00080000  /* Ok button will say "Open" */
#define OF_NOOPTIONS    0x00100000  /* Disable the options fold out */

 /* Attributes for DlgDirLst() */
#define ATTRFILELIST    0x0000      /* include files only */
#define ATTRDIRLIST     0xC010      /* directories and drives ONLY */
#define CBEXTMAX        6           /* Number of bytes in "\*.txt" */

#define IDF(id)   ((id) & ~DLGOPEN_OPTION)  /* extracts flag from control ID */
#define FID(f)    ((f)  |  DLGOPEN_OPTION)  /* extracts control ID from flag */

/***********************************************************/
/* Declarations of functions used in the dlgopen.c module  */
/***********************************************************/

LRESULT CALLBACK DlgfnOpen(HWND, UINT, WPARAM, LPARAM);
int WINAPI DlgOpenFile(
    HWND    ,
    char* ,
    DWORD   ,
    char* ,
    char* 
);
LRESULT CALLBACK DlgfnOpen2(HWND, UINT, WPARAM, LPARAM);
HWND DlgOpenFile2(
    HWND    ,
    char* ,
    DWORD   ,
    char* ,
    char*
);
LRESULT CALLBACK DlgfnChoose(HWND, UINT, WPARAM, LPARAM);
HWND DlgChooseFiles(
    HWND    ,
    char* ,
    DWORD   ,
    char* ,
    char*
);
