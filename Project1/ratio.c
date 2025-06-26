/* Ratios Module for Windows program for SIMS data analysis. */

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <errno.h>
#include "dlgopen.h"
#include "sims.h"
#include "data_struct.h"
#include <io.h>
#include <fcntl.h>


#define PLOTX2         500
#define PLOTY2         460
#define ORIGINX         40
#define ENDX           400
#define ORIGINY        340
#define ENDY            20

extern FILE* retrieve_file();
extern HDC print_file();
extern char* filename_array[120];

extern HINSTANCE hInst;
extern HWND hwnd;                       /* handle to main window */
extern int program;
extern HANDLE Files;
extern struct fname_record* filenames;
extern int total_fnames;

extern int screensize;                             /* text sizes */
extern int start_xvalue, end_xvalue;
extern int start_yvalue, end_yvalue;
extern double onex, oney;
extern double ponex, poney;                      /* for drawing points */
extern float y_inc, x_inc;                      /* pixels to move per one y or x value */
extern int originx, originy;                    /* current plot 0,0 point */
extern HFONT hSScriptFont;
extern int symbol[MAXMASSES];
extern int colour[MAXMASSES];
extern TCHAR labels[MAXMASSES][MAXLABELLENGTH];     /* mass labels */

HWND hRatiosWnd, hRatiosGraphPopup;        /* child window for ratio info */
HWND hRatiosFilenameTxt, hRatiosFilenameWnd;
HWND hRatiosMatrixTxt, hRatiosMatrixWnd;
HWND hRatiosFilenameBox, hRatiosMatrixBox;
HWND hRatiosFnameBox, hRatiosFnameWnd;
HWND hRatiosMLabelBox, hRatiosMLabelWnd;
HWND hRatiosDepthTxt, hRatiosDepthWnd;
HWND hRatiosAlphaTxt, hRatiosAlphaWnd;
HWND hRatiosDepthBox, hRatiosAlphaBox;
HWND hRatiosMassesTxt, hRatiosFnameTxt;
HWND hRatiosBgRatioTxt, hRatiosBgPPMTxt;
HWND hRatiosBgRatioBox, hRatiosBgPPMBox;
HWND hRatiosBgRatioWnd, hRatiosBgPPMWnd;
HWND hRatiosMaxRatioTxt, hRatiosMaxPPMTxt;
HWND hRatiosMaxRatioBox, hRatiosMaxPPMBox;
HWND hRatiosMaxRatioWnd, hRatiosMaxPPMWnd;
HWND hRatiosSlopeTxt, hRatiosInterceptTxt;
HWND hRatiosSlopeBox, hRatiosInterceptBox;
HWND hRatiosSlopeWnd, hRatiosInterceptWnd;
HWND hRatiosMassesBox[MAXMASSES];
HWND hRatiosMassesWnd[MAXMASSES];
HWND hRatiosRetrieve, hRatiosBatch;
HWND hRatiosCancel, hRatiosStandard;
HWND hRatiosPopupCancel, hRatiosPopupPrint;
HWND hRatiosSELabelBox, hRatiosSELabelWnd;
HWND hRatiosSIsoAbunBox, hRatiosSIsoAbunWnd;
HWND hRatiosRELabelBox, hRatiosRELabelWnd;
HWND hRatiosRIsoAbunBox, hRatiosRIsoAbunWnd;
HWND hRatiosSElementBox, hRatiosSElementWnd;
HWND hRatiosSElementTxt, hRatiosRElementTxt;
HWND hRatiosRElementBox, hRatiosRElementWnd;
HWND hHighYValueTxt, hHighYValueBox;
HWND hLowYValueTxt, hLowYValueBox;
HWND hHighYValueWnd, hLowYValueWnd;
HWND hHighXValueTxt, hHighXValueBox;
HWND hLowXValueTxt, hLowXValueBox;
HWND hHighXValueWnd, hLowXValueWnd;
HWND hRatiosPopupOK, hRatiosPrint, hRatiosSave;
HWND hRatioLabelTxt, hRatioLabelBox, hRatioLabelWnd;

float abundances[239][3];               /* table of isotope abundances */
HANDLE Block[MAXMASSES];
struct data* points, * pts;
int num_masses, num_cycles;
int time_period, num;
unsigned long total_counts[MAXMASSES];
float mass[MAXMASSES];
int offset[MAXMASSES];
float matrix_mass, selement_mass;
float depth, alpha, relement_mass;
float slope, intercept, relement_abundance;
float ratio[MAXFILES], ppm[MAXFILES];
float bg_ratio, max_ratio, bg_ppm, max_ppm;
float matrix_abundance, selement_abundance;
TCHAR selabel[5], relabel[5], mlabel[5];
TCHAR ratiolabel[5];
FILE* outfn;
int batch_num, bad_points;           /* counter for batch process */
int pick_matrix, pick_element;       /* flag for choosing matrix,element mass */
int matrix, selement, relement;       /* matrix,element mass index */
TCHAR date_string[20], time_string[20];
static int printersize;
static TCHAR filename[128], fname[128];
static TCHAR string[128], outfilename[128];

extern void plot_point(HDC, int, int, int, int, int, double, double);


/*****************************************************************************/

static void read_abundances()
{

    /* Read the table of mass abundances, and store it for use in modifying
       counts.
     */

    FILE* fn;
    int i, j, dummy;
    errno_t err = fopen_s(&fn, "c:\\lima\\periodic.tbl", "rt");

    if (err!=0) {
        /*MessageBox(NULL, "Cannot open periodic.tbl", "Error", MB_OK);*/
        return (FALSE);
    }
    for (i = 1; i < 239; i++) {
        fgets(string, 40, fn);
        sscanf_s(string, "%d %f %f %f", &dummy, &abundances[i][0], &abundances[i][1],
            &abundances[i][2]);
        for (j = 0; j < 3; j++) abundances[i][j] /= 100.0;
    }
    fclose(fn);
}

/*****************************************************************************/

static BOOL create_ratios_graph_window(hInstance)
HANDLE hInstance;
{

    /*  Create the "Ratios Graph" child window and fill it with controls.
     */

    hRatiosGraphPopup = CreateWindow("RatiosClass", "Ratios Graph",
        WS_VISIBLE | WS_POPUP | WS_BORDER | WS_CAPTION,
        100, 10, PLOTX2, PLOTY2 + 35, hwnd, NULL, hInstance, NULL);

    hLowYValueTxt = CreateWindow("Static", "Low Y",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 10, 367, 50, 18,
        hRatiosGraphPopup, -1, hInstance, NULL);

    hLowYValueBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 10, 385, 40, 24,
        hRatiosGraphPopup, -1, hInstance, NULL);

    hLowYValueWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 12, 387, 36, 20,
        hRatiosGraphPopup, WS_LOWYEDIT, hInstance, NULL);

    hHighYValueTxt = CreateWindow("Static", "High Y",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 65, 367, 50, 18,
        hRatiosGraphPopup, -1, hInstance, NULL);

    hHighYValueBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 65, 385, 40, 24,
        hRatiosGraphPopup, -1, hInstance, NULL);

    hHighYValueWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 67, 387, 36, 20,
        hRatiosGraphPopup, WS_HIGHYEDIT, hInstance, NULL);

    hLowXValueTxt = CreateWindow("Static", "Low X",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 120, 367, 50, 18,
        hRatiosGraphPopup, -1, hInstance, NULL);

    hLowXValueBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 120, 385, 40, 24,
        hRatiosGraphPopup, -1, hInstance, NULL);

    hLowXValueWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 122, 387, 36, 20,
        hRatiosGraphPopup, WS_LOWXEDIT, hInstance, NULL);

    hHighXValueTxt = CreateWindow("Static", "High X",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 175, 367, 50, 18,
        hRatiosGraphPopup, -1, hInstance, NULL);

    hHighXValueBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 175, 385, 40, 24,
        hRatiosGraphPopup, -1, hInstance, NULL);

    hHighXValueWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 177, 387, 36, 20,
        hRatiosGraphPopup, WS_HIGHXEDIT, hInstance, NULL);

    hRatioLabelTxt = CreateWindow("Static", "Label:  x",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 310, 360, 60, 18,
        hRatiosGraphPopup, -1, hInstance, NULL);

    hRatioLabelBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 370, 358, 50, 23,
        hRatiosGraphPopup, -1, hInstance, NULL);

    hRatioLabelWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 372, 360, 46, 20,
        hRatiosGraphPopup, WS_RATIOLABELEDIT, hInstance, NULL);

    hRatiosPopupOK = CreateWindow("Button", "OK",
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 250, 385, 40, 24,
        hRatiosGraphPopup, WS_RATIOSPOPUPOK, hInstance, NULL);

    hRatiosPopupPrint = CreateWindow("Button", "Print",
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 300, 385, 60, 24,
        hRatiosGraphPopup, WS_RATIOSPOPUPPRINT, hInstance, NULL);

    hRatiosCancel = CreateWindow("Button", "Cancel",
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 370, 385, 60, 24,
        hRatiosGraphPopup, WS_RATIOSPOPUPCANCEL, hInstance, NULL);

    sprintf_s(string,128, "%3d", start_yvalue);
    SendMessage(hLowYValueWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)string);
    sprintf_s(string,128, "%3d", end_yvalue);
    SendMessage(hHighYValueWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)string);
    sprintf_s(string,128, "%3d", start_xvalue);
    SendMessage(hLowXValueWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)string);
    sprintf_s(string, 128,"%3d", end_xvalue);
    SendMessage(hHighXValueWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)string);
    SendMessage(hRatioLabelWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)ratiolabel);

    return (TRUE);
}

/*****************************************************************************/
 BOOL create_ratios_set_window(hInstance)
HANDLE hInstance;
{

    /*  Create the "Ratios Set" child window and fill it with controls.
     */

    int i, j;

    hRatiosWnd = CreateWindow("RatiosClass", "Calculate Ratios",
        WS_VISIBLE | WS_CHILD, 10, 10, 620, 430,
        hwnd, WS_RATIOS_SET, hInstance, NULL);
    if (!hRatiosWnd) return (FALSE);

    hRatiosFilenameTxt = CreateWindow("Static", "Filename",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 10, 20, 80, 20,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosFilenameBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 90, 18, 124, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosFilenameWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 92, 20, 120, 20,
        hRatiosWnd, WS_RATIOSFILENAME, hInstance, NULL);

    hRatiosFnameTxt = CreateWindow("Static", "Output File Name",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 270, 20, 120, 20,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosFnameBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 400, 18, 104, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosFnameWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 402, 20, 100, 20,
        hRatiosWnd, WS_RATIOSFNAME, hInstance, NULL);

    hRatiosMassesTxt = CreateWindow("Static", "Masses",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 10, 60, 54, 20,
        hRatiosWnd, -1, hInstance, NULL);

    for (i = 0; i < MAXMASSES / 2; i++) {
        hRatiosMassesBox[i] = CreateWindow("Static", NULL,
            SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 76 + 48 * i, 58, 44, 24,
            hRatiosWnd, -1, hInstance, NULL);

        hRatiosMassesWnd[i] = CreateWindow("Edit", NULL,
            ES_LEFT | WS_CHILD | WS_VISIBLE, 78 + 48 * i, 60, 40, 20,
            hRatiosWnd, WS_RATIOSMASSES0 + i, hInstance, NULL);
    }
    for (j = 0, i = MAXMASSES / 2; i < MAXMASSES; j++, i++) {
        hRatiosMassesBox[i] = CreateWindow("Static", NULL,
            SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 76 + 48 * j, 88, 44, 24,
            hRatiosWnd, -1, hInstance, NULL);

        hRatiosMassesWnd[i] = CreateWindow("Edit", NULL,
            ES_LEFT | WS_CHILD | WS_VISIBLE, 78 + 48 * j, 90, 40, 20,
            hRatiosWnd, WS_RATIOSMASSES0 + i, hInstance, NULL);
    }

    hRatiosMLabelBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 46, 128, 44, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosMLabelWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 48, 130, 40, 20,
        hRatiosWnd, WS_RATIOSMLABEL, hInstance, NULL);

    hRatiosMatrixTxt = CreateWindow("Static", "Matrix Mass",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 100, 130, 100, 20,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosMatrixBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 200, 128, 64, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosMatrixWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 202, 130, 60, 20,
        hRatiosWnd, WS_RATIOSMATRIX, hInstance, NULL);

    hRatiosSElementTxt = CreateWindow("Static", "Standard Element",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 280, 130, 120, 20,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosSElementBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 400, 128, 64, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosSElementWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 402, 130, 60, 20,
        hRatiosWnd, WS_RATIOSSELEMENT, hInstance, NULL);

    hRatiosSELabelBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 470, 128, 44, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosSELabelWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 472, 130, 40, 20,
        hRatiosWnd, WS_RATIOSSELABEL, hInstance, NULL);

    hRatiosSIsoAbunBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 520, 128, 64, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosSIsoAbunWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 522, 130, 60, 20,
        hRatiosWnd, WS_RATIOSSISOABUN, hInstance, NULL);

    hRatiosRElementTxt = CreateWindow("Static", "Ratios Element",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 290, 160, 100, 20,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosRElementBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 400, 158, 64, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosRElementWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 402, 160, 60, 20,
        hRatiosWnd, WS_RATIOSRELEMENT, hInstance, NULL);

    hRatiosRELabelBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 470, 158, 44, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosRELabelWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 472, 160, 40, 20,
        hRatiosWnd, WS_RATIOSRELABEL, hInstance, NULL);

    hRatiosRIsoAbunBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 520, 158, 64, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosRIsoAbunWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 522, 160, 60, 20,
        hRatiosWnd, WS_RATIOSRISOABUN, hInstance, NULL);

    hRatiosDepthTxt = CreateWindow("Static", "Depth",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 140, 200, 60, 20,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosDepthBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 200, 198, 64, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosDepthWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 202, 200, 60, 20,
        hRatiosWnd, WS_RATIOSDEPTH, hInstance, NULL);

    hRatiosAlphaTxt = CreateWindow("Static", "Alpha",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 350, 200, 50, 20,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosAlphaBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 400, 198, 64, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosAlphaWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 402, 200, 60, 20,
        hRatiosWnd, WS_RATIOSALPHA, hInstance, NULL);

    hRatiosBgRatioTxt = CreateWindow("Static", "Bg Ratio",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 120, 240, 70, 20,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosBgRatioBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 200, 238, 74, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosBgRatioWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 202, 240, 70, 20,
        hRatiosWnd, WS_RATIOSBGRATIO, hInstance, NULL);

    hRatiosBgPPMTxt = CreateWindow("Static", "Bg ppm",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 335, 240, 50, 20,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosBgPPMBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 400, 238, 74, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosBgPPMWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 402, 240, 70, 20,
        hRatiosWnd, WS_RATIOSBGPPM, hInstance, NULL);

    hRatiosMaxRatioTxt = CreateWindow("Static", "Max Ratio",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 110, 280, 80, 20,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosMaxRatioBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 200, 278, 74, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosMaxRatioWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 202, 280, 70, 20,
        hRatiosWnd, WS_RATIOSMAXRATIO, hInstance, NULL);

    hRatiosMaxPPMTxt = CreateWindow("Static", "Max ppm",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 325, 280, 60, 20,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosMaxPPMBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 400, 278, 74, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosMaxPPMWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 402, 280, 70, 20,
        hRatiosWnd, WS_RATIOSMAXPPM, hInstance, NULL);

    hRatiosSlopeTxt = CreateWindow("Static", "Slope",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 140, 320, 60, 20,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosSlopeBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 200, 318, 74, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosSlopeWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 202, 320, 70, 20,
        hRatiosWnd, WS_RATIOSSLOPE, hInstance, NULL);

    hRatiosInterceptTxt = CreateWindow("Static", "Intercept",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 330, 320, 70, 20,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosInterceptBox = CreateWindow("Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 400, 318, 74, 24,
        hRatiosWnd, -1, hInstance, NULL);

    hRatiosInterceptWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 402, 320, 70, 20,
        hRatiosWnd, WS_RATIOSINTERCEPT, hInstance, NULL);

    hRatiosRetrieve = CreateWindow("Button", "Retrieve",
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 40, 400, 70, 24,
        hRatiosWnd, WS_RATIOSRETRIEVE, hInstance, NULL);

    hRatiosStandard = CreateWindow("Button", "Standard",
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 130, 400, 70, 24,
        hRatiosWnd, WS_RATIOSSTANDARD, hInstance, NULL);

    hRatiosBatch = CreateWindow("Button", "Batch",
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 220, 400, 70, 24,
        hRatiosWnd, WS_RATIOSBATCH, hInstance, NULL);

    hRatiosCancel = CreateWindow("Button", "Cancel",
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 310, 400, 70, 24,
        hRatiosWnd, WS_RATIOSCANCEL, hInstance, NULL);

    hRatiosPrint = CreateWindow("Button", "Print",
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 400, 400, 70, 24,
        hRatiosWnd, WS_RATIOSPRINT, hInstance, NULL);

    hRatiosSave = CreateWindow("Button", "Save",
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 490, 400, 70, 24,
        hRatiosWnd, WS_RATIOSSAVE, hInstance, NULL);

    read_abundances();
    for (i = 0; i < MAXMASSES; i++) Block[i] = NULL;
    batch_num = 0;
    pick_matrix = FALSE;
    matrix = -1;
    pick_element = FALSE;
    selement = -1;
    relement = -1;
    strcpy_s(fname,128, "noname");
    SendMessage(hRatiosFnameWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)fname);
    slope = -1.0;
    matrix_abundance = 1.0;
    selement_abundance = 1.0;
    relement_abundance = 1.0;
    mlabel[0] = '\0';
    selabel[0] = '\0';
    relabel[0] = '\0';
    start_xvalue = -6;
    end_xvalue = -2;
    start_yvalue = -1;
    end_yvalue = 3;
    strcpy_s(ratiolabel,5, "100");

    return (TRUE);
}

/*****************************************************************************/

static void reduce_E_exp(str)
TCHAR* str;                      /* string containing data in E format */
{

    /* Remove all extra zeros after the 'e' from a E format string. */

    int i, j;

    for (i = 0; str[i] != 'e' && str[i] != 'E'; i++);
    for (; str[i + 1] != '\0'; )
        if (str[i] == '0') {
            for (j = i + 1; str[j] != '\0'; j++)
                str[j - 1] = str[j];
            str[j - 1] = '\0';
        }
        else i++;
}

/*****************************************************************************/

static void draw_calibration_plot(plotDC, xsize, ysize)
HDC plotDC;
int xsize;                              /* Custom width if not NULL. */
int ysize;                              /* Custom height if not NULL. */
{

    /* Draw the axes for the calibration plot, and fill in all the default labels. */

    HFONT hLabelFont, hOldFont;
    HFONT hYAxisFont, hTitleFont;
    HPEN hDottedPen, hOldPen, hColouredPen;
    HBRUSH hBrush;
    HRGN hRegion;
    RECT frame;
    float x_plot_range;             /* x axis range to be plotted over */
    float y_plot_range;             /* y axis range to be plotted over */
    float x_range, y_range;          /* value ranges to plot over */
    float step_xvalue, temp, y;
    int i, j, num_lines, locx, locy, xtick, num_xticks;
    int lsize, yaxsize, ssize1, ssize2, tsize;
    int endx, endy, intervals, width, ox, oy;
    int lastx, lasty;
    TCHAR str[15];

    lsize = screensize;
    ssize1 = screensize * 0.8;
    tsize = lsize * 1.5;
    yaxsize = tsize * 0.9;
    originx = ORIGINX;
    endx = ENDX;
    originy = ORIGINY;
    endy = ENDY;
    onex = oney = 1.0;
    ponex = onex;
    poney = oney;

    if (xsize) {
        lsize = printersize;
        ssize1 = printersize * 0.8;
        tsize = lsize * 1.5;
        yaxsize = tsize * 0.9;
        temp = xsize;
        temp = temp / PLOTX2;
        originx = originx * temp;
        endx = endx * temp;
        onex = temp;
        temp = ysize;
        temp = temp / PLOTY2;
        originy = originy * temp;
        endy = endy * temp;
        oney = temp;
        ponex = onex * 0.5;
        poney = oney * 0.5;
    }

    hLabelFont = CreateFont(lsize, 0, 0, 0, FW_NORMAL, FALSE,
        FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_SWISS, "Arial");
    hYAxisFont = CreateFont(yaxsize, 0, 900, 900, FW_NORMAL, FALSE, FALSE,
        FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_SWISS, "Arial");
    hSScriptFont = CreateFont(ssize1, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_SWISS, "Arial");
    hTitleFont = CreateFont(tsize, 0, 0, 0, FW_NORMAL, FALSE,
        FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_SWISS, "Arial");
    hDottedPen = CreatePen(PS_DOT, 1, (LONG)BLACK);

    SetTextAlign(plotDC, TA_TOP | TA_CENTER);           /* y axis label */
    hOldFont = SelectObject(plotDC, hYAxisFont);
    locx = originx - 30 * onex;
    locy = originy - (originy - endy) / 2;
    sprintf_s(string,128, "%s, ppm", selabel);
    TextOut(plotDC, locx, locy, string, strlen(string));
    SelectObject(plotDC, hLabelFont);
    DeleteObject(hYAxisFont);

    y_range = end_yvalue - start_yvalue;
    y_plot_range = originy - endy;
    y_inc = y_plot_range / y_range;
    SetTextAlign(plotDC, TA_TOP | TA_RIGHT);
    for (i = start_yvalue; i <= end_yvalue; i++) {
        locx = originx - 1 * onex;
        locy = originy - (i - start_yvalue) * y_inc - 5 * oney;
        sprintf_s(string,128, "%d", i);
        SelectObject(plotDC, hSScriptFont);
        TextOut(plotDC, locx, locy, string, strlen(string));
        locy += 2 * oney;

        SIZE textSize;  // Create a SIZE structure to receive the dimensions.
        GetTextExtentPoint32(plotDC, string, strlen(string), &textSize);

        width = textSize.cx;
        SelectObject(plotDC, hLabelFont);
        strcpy_s(string, 128,"10");
        TextOut(plotDC, locx - width, locy, string, 2);
    }
    SetTextAlign(plotDC, TA_TOP | TA_LEFT);
    if (xsize) {
        /*
                for (i=start_yvalue+1; i < end_yvalue; i++) {          Y axis lines
                    hOldPen = SelectObject (plotDC,hDottedPen);
                    locy = originy-(i-start_yvalue)*y_inc;
                    MoveToEx (plotDC,originx,locy);
                    LineTo (plotDC,endx,locy);
                    SelectObject (plotDC,hOldPen);
                    MoveToEx (plotDC,originx,locy);
                    locx = originx+3*onex;
                    LineTo (plotDC,locx,locy);
                    MoveToEx (plotDC,endx,locy);
                    locx = endx-3*onex;
                    LineTo (plotDC,locx,locy);
                }
        */
        for (i = start_yvalue; i < end_yvalue; i++) {         /* Y axis ticks */
            for (j = 1; j <= 9; j++) {
                locy = originy - (log10(j * pow(10, (i - start_yvalue)))) * y_inc;
                MoveToEx(plotDC, originx, locy, NULL);
                locx = originx + 2 * onex;
                LineTo(plotDC, locx, locy);
                hOldPen = SelectObject(plotDC, hDottedPen);
                LineTo(plotDC, endx, locy);
                SelectObject(plotDC, hOldPen);
            }
        }
    }

    SetTextAlign(plotDC, TA_TOP | TA_CENTER);
    x_range = end_xvalue - start_xvalue;
    x_plot_range = endx - originx;
    x_inc = x_plot_range / x_range;
    for (i = start_xvalue; i <= end_xvalue; i++) {
        strcpy_s(string,128, "10");
        locy = originy + 3 * oney;
        locx = originx + (i - start_xvalue) * x_inc;
        TextOut(plotDC, locx, locy, string, 2);

        SIZE textSize;  // Create a SIZE structure to receive the dimensions.
        GetTextExtentPoint32(plotDC, string, 2, &textSize);

        width = textSize.cx;
        sprintf_s(string, 128,"%d", i);
        SelectObject(plotDC, hSScriptFont);
        locy -= 2 * oney;
        TextOut(plotDC, locx + width, locy, string, strlen(string));
        SelectObject(plotDC, hLabelFont);
    }
    if (xsize) {
        /*
                for (i=start_xvalue+1; i < end_xvalue; i++) {          X axis lines
                    hOldPen = SelectObject (plotDC,hDottedPen);
                    locx = originx-(i-start_xvalue)*x_inc;
                    MoveToEx (plotDC,locx,originy);
                    LineTo (plotDC,locx,endy);
                    SelectObject (plotDC,hOldPen);
                    MoveToEx (plotDC,locx,originy);
                    locy = originy-3*oney;
                    LineTo (plotDC,locx,locy);
                    MoveToEx (plotDC,locx,endy);
                    locy = endy+3*oney;
                    LineTo (plotDC,locx,locy);
                }
        */
        for (i = start_xvalue; i < end_xvalue; i++) {         /* X axis ticks */
            for (j = 1; j <= 9; j++) {
                locx = originx + (log10(j * pow(10, (i - start_xvalue)))) * x_inc;
                MoveToEx(plotDC, locx, originy, NULL);
                locy = originy - 2 * onex;
                LineTo(plotDC, locx, locy);
                hOldPen = SelectObject(plotDC, hDottedPen);
                LineTo(plotDC, locx, endy);
                SelectObject(plotDC, hOldPen);
            }
        }
    }
    DeleteObject(hSScriptFont);
    DeleteObject(hDottedPen);

    MoveToEx(plotDC, originx, originy, NULL);         /* X and Y axes */
    LineTo(plotDC, endx, originy);
    LineTo(plotDC, endx, endy);
    LineTo(plotDC, originx, endy);
    LineTo(plotDC, originx, originy);

    SelectObject(plotDC, hTitleFont);
    SetTextAlign(plotDC, TA_TOP | TA_CENTER);
    locx = originx + ((endx - originx) / 2);
    locy = originy + 10 * oney;
    if (relabel[0] == '\0')
        sprintf_s(string, 128,"%s/%s x %s", selabel, mlabel, ratiolabel);
    else sprintf_s(string,128, "%s/%s x %s", relabel, mlabel, ratiolabel);
    TextOut(plotDC, locx, locy, string, strlen(string));
    strcpy_s(string,128, filename);
    for (i = 0; i < strlen(string) && string[i] != '.'; i++);
    string[i] = '\0';
    locy = endy - 22 * oney;
    TextOut(plotDC, locx, locy, string, strlen(string));
    /*
        width = parse_label (plotDC,title,hTitleFont);
        locx = (originx+(endx-originx)/2)-(width/2);
        locy = endy-10*oney;
        draw_labels (plotDC,locx,locy,title,hTitleFont);
    */
    hBrush = GetStockObject(WHITE_BRUSH);
    frame.left = originx + 10 * onex;
    frame.top = endy + 10 * oney;
    frame.right = originx + 90 * onex;
    frame.bottom = endy + 98 * oney;
    hRegion = CreateRectRgn(frame.left, frame.top, frame.right, frame.bottom);
    FillRgn(plotDC, hRegion, hBrush);
    DeleteObject(hRegion);
    hBrush = CreateSolidBrush(BLACK_BRUSH);
    FrameRect(plotDC, &frame, hBrush);
    DeleteObject(hBrush);

    SelectObject(plotDC, hLabelFont);
    DeleteObject(hTitleFont);
    SetTextAlign(plotDC, TA_TOP | TA_LEFT);
    sprintf_s(string,128, "%.0f", depth);
    locx = originx + 14 * onex;
    j = originx + 54 * onex;
    locy = endy + 14 * oney;
    TextOut(plotDC, locx, locy, "depth", strlen("depth"));
    TextOut(plotDC, j, locy, string, strlen(string));
    sprintf_s(string,128, "%.2f", alpha);
    locy = endy + 24 * oney;
    TextOut(plotDC, locx, locy, "alpha", strlen("alpha"));
    TextOut(plotDC, j, locy, string, strlen(string));
    sprintf_s(str,15, "%.2e", bg_ratio);
    reduce_E_exp(str);
    locy = endy + 34 * oney;
    TextOut(plotDC, locx, locy, "bg ratio", strlen("bg ratio"));
    TextOut(plotDC, j, locy, str, strlen(str));
    sprintf_s(str,15, "%.2e", bg_ppm);
    reduce_E_exp(str);
    locy = endy + 44 * oney;
    TextOut(plotDC, locx, locy, "bg ppm", strlen("bg ppm"));
    TextOut(plotDC, j, locy, str, strlen(str));
    sprintf_s(str,15, "%.2e", max_ratio);
    reduce_E_exp(str);
    locy = endy + 54 * oney;
    TextOut(plotDC, locx, locy, "max ratio", strlen("max ratio"));
    TextOut(plotDC, j, locy, str, strlen(str));
    if (max_ppm < 1.0) {
        sprintf_s(str,15, "%.2e", max_ppm);
        reduce_E_exp(str);
    }
    else sprintf_s(str,15, "%6.2f", max_ppm);
    locy = endy + 64 * oney;
    TextOut(plotDC, locx, locy, "max ppm", strlen("max ppm"));
    TextOut(plotDC, j, locy, str, strlen(str));
    sprintf_s(string,128, "%.3f", slope);
    locy = endy + 74 * oney;
    TextOut(plotDC, locx, locy, "slope", strlen("slope"));
    TextOut(plotDC, j, locy, string, strlen(string));
    sprintf_s(string,128, "%.3f", intercept);
    locy = endy + 84 * oney;
    TextOut(plotDC, locx, locy, "intercept", strlen("intercept"));
    TextOut(plotDC, j, locy, string, strlen(string));
    SelectObject(plotDC, hOldFont);
    DeleteObject(hLabelFont);

    for (i = 0; i < num; i++) {
        locx = originx + (log10(ratio[i]) - start_xvalue) * x_inc;
        locy = originy - (log10(ppm[i]) - start_yvalue) * y_inc;
        plot_point(plotDC, locx, locy, 0, 0, FALSE, ponex, poney);
    }
    locx = originx + (log10(ratio[0]) - start_xvalue) * x_inc;
    y = slope * log10(ratio[0]) + intercept;
    locy = originy - (y - start_yvalue) * y_inc;
    MoveToEx(plotDC, locx, locy, NULL);
    locx = originx + (log10(ratio[num - 1]) - start_xvalue) * x_inc;
    y = slope * log10(ratio[num - 1]) + intercept;
    locy = originy - (y - start_yvalue) * y_inc;
    LineTo(plotDC, locx, locy);

}

/*****************************************************************************/

static void print_calibration_plot()
{

    HDC printDC;
    POINT PageSize;
    int xsize, ysize;

    printDC = print_file(hRatiosGraphPopup, FALSE);
    if (printDC == NULL) {
        MessageBox(NULL, "print_plot: Cannot Print", "Error", MB_OK);
        return (FALSE);
    }
    Escape(printDC, STARTDOC, 17, (LPSTR)"Calibration Print", 0L);
    Escape(printDC, GETPHYSPAGESIZE, NULL, NULL, (LPSTR)&PageSize);
    xsize = PageSize.x;
    ysize = PageSize.y;
    printersize = 0.018 * PageSize.x;
    draw_calibration_plot(printDC, xsize, ysize);

    SetTextAlign(printDC, TA_TOP | TA_LEFT);
    Escape(printDC, NEWFRAME, 0, 0L, 0L);
    Escape(printDC, ENDDOC, 0, 0L, 0L);
    DeleteDC(printDC);
    printDC = NULL;
}

/******************************************************************************/

static float
find_abundance(tmass)
int tmass;                  /* use integer mass number as index */
{

    /* Find the abundance for the specified mass.  Choose the
       largest of the listed abundances.
     */

    int i, j;
    float max;

    if (tmass > 0 && tmass < 239) {
        for (i = 0, max = 0.0, j = 0; i < 3; i++)
            if (abundances[tmass][i] > max) {
                max = abundances[tmass][i];
                j = i;
            }
        if (max <= 0.0) return (1.0);
        else return (abundances[tmass][j]);
    }
}

/*****************************************************************************/

static void fill_in_label(mass_num, is_element)
float mass_num;                         /* mass number */
int is_element;                         /* TRUE if doing element mass */
{

    /* Find the default label, if any, associated with the specified mass, and
       write that in the proper label box.
     */

    int tmass;
    TCHAR label[5];
    float abundance;

    if (mass_num <= 0.0) return (FALSE);

    label[0] = '\0';
    abundance = 1.0;
    tmass = mass_num + 0.5;          /* force rounding */
    if (tmass == 197) {
        strcpy_s(label,5, "Au");
        abundance = 1.0;
    }
    else if (tmass == 107 || tmass == 109) {
        strcpy_s(label,5, "Ag");
        if (tmass == 107) abundance = 0.518;
        else abundance = 0.482;
    }
    else if (tmass == 194 || tmass == 195 || tmass == 196 || tmass == 198) {
        strcpy_s(label,5, "Pt");
        if (tmass == 194) abundance = 0.329;
        else if (tmass == 195) abundance = 0.338;
        else if (tmass == 196) abundance = 0.253;
        else abundance = 0.0721;
    }
    else if (tmass == 104 || tmass == 105 || tmass == 106 ||
        tmass == 108 || tmass == 110) {
        strcpy_s(label,5, "Pd");
        if (tmass == 104) abundance = 0.110;
        else if (tmass == 105) abundance = 0.222;
        else if (tmass == 106) abundance = 0.273;
        else if (tmass == 108) abundance = 0.267;
        else abundance = 0.118;
    }
    else if (tmass == 34) strcpy_s(label,5, "S");
    else if (tmass == 75) strcpy_s(label,5, "As");
    else abundance = find_abundance(tmass);
    if (is_element == TRUE) {
        SendMessage(hRatiosSELabelWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)label);
        sprintf_s(string,128, "%6.4f", abundance);
        SendMessage(hRatiosSIsoAbunWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)string);
    }
    else if (is_element == ALTTRUE) {
        SendMessage(hRatiosRELabelWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)label);
        sprintf_s(string,128, "%6.4f", abundance);
        SendMessage(hRatiosRIsoAbunWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)string);
    }
    else SendMessage(hRatiosMLabelWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)label);
}

/*****************************************************************************/

LRESULT CALLBACK RatiosProc(hWnd, message, wParam, lParam)
HWND hWnd;
UINT message;
WPARAM wParam;
LPARAM lParam;
{

    /* Processes messages received by the Ratios child window.
     */

    int i, err;
    TCHAR ext[5];
    FILE* fn;
    HDC ratiosDC;
    PAINTSTRUCT ratiosPaint;
    HCURSOR hHourGlass, hSaveCursor;

    switch (message) {

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case WS_RATIOSFNAME:
            if (HIWORD(wParam) == EN_CHANGE) {
                SendMessage(hRatiosFnameWnd, WM_GETTEXT, 80,
                    (DWORD)(LPSTR)fname);
            }
            break;

        case WS_RATIOSMASSES0: case WS_RATIOSMASSES0 + 1: case WS_RATIOSMASSES0 + 2:
        case WS_RATIOSMASSES0 + 3: case WS_RATIOSMASSES0 + 4: case WS_RATIOSMASSES0 + 5:
        case WS_RATIOSMASSES0 + 6: case WS_RATIOSMASSES0 + 7: case WS_RATIOSMASSES0 + 8:
        case WS_RATIOSMASSES0 + 9: case WS_RATIOSMASSES0 + 10: case WS_RATIOSMASSES0 + 11:
        case WS_RATIOSMASSES0 + 12: case WS_RATIOSMASSES0 + 13: case WS_RATIOSMASSES0 + 14:
        case WS_RATIOSMASSES0 + 15: case WS_RATIOSMASSES0 + 16: case WS_RATIOSMASSES0 + 17:
        case WS_RATIOSMASSES0 + 18: case WS_RATIOSMASSES0 + 19:
            if (HIWORD(wParam) == EN_SETFOCUS && pick_matrix) {
                int temp = LOWORD(wParam) - WS_RATIOSMASSES0;
                pick_matrix = FALSE;
                SendMessage(hRatiosMassesWnd[LOWORD(wParam) - WS_RATIOSMASSES0],
                    WM_GETTEXT, 10, (DWORD)(LPSTR)string);
                matrix_mass = atof(string);
                SendMessage(hRatiosMatrixWnd, WM_SETTEXT, 0,
                    (DWORD)(LPSTR)string);
                matrix = LOWORD(wParam) - WS_RATIOSMASSES0;
                fill_in_label(matrix_mass, FALSE);
            }
            else if (HIWORD(wParam) == EN_SETFOCUS && pick_element == TRUE) {
                pick_element = FALSE;
                SendMessage(hRatiosMassesWnd[LOWORD(wParam) - WS_RATIOSMASSES0],
                    WM_GETTEXT, 10, (DWORD)(LPSTR)string);
                selement_mass = atof(string);
                SendMessage(hRatiosSElementWnd, WM_SETTEXT, 0,
                    (DWORD)(LPSTR)string);
                selement = LOWORD(wParam) - WS_RATIOSMASSES0;
                fill_in_label(selement_mass, TRUE);
            }
            else if (HIWORD(wParam) == EN_SETFOCUS && pick_element == ALTTRUE) {
                pick_element = FALSE;
                SendMessage(hRatiosMassesWnd[LOWORD(wParam) - WS_RATIOSMASSES0],
                    WM_GETTEXT, 10, (DWORD)(LPSTR)string);
                relement_mass = atof(string);
                SendMessage(hRatiosRElementWnd, WM_SETTEXT, 0,
                    (DWORD)(LPSTR)string);
                relement = LOWORD(wParam) - WS_RATIOSMASSES0;
                fill_in_label(relement_mass, ALTTRUE);
            }
            break;

        case WS_RATIOSMATRIX:
            if (HIWORD(wParam) == EN_SETFOCUS) {
                pick_matrix = TRUE;
            }
            break;

        case WS_RATIOSMLABEL:
            if (HIWORD(wParam) == EN_CHANGE) {
                SendMessage(hRatiosMLabelWnd, WM_GETTEXT, 40,
                    (DWORD)(LPSTR)mlabel);
            }
            break;

        case WS_RATIOSSELEMENT:
            if (HIWORD(wParam) == EN_SETFOCUS) {
                pick_element = TRUE;
            }
            break;

        case WS_RATIOSSELABEL:
            if (HIWORD(wParam) == EN_CHANGE) {
                SendMessage(hRatiosSELabelWnd, WM_GETTEXT, 40,
                    (DWORD)(LPSTR)selabel);
            }
            break;

        case WS_RATIOSSISOABUN:
            if (HIWORD(wParam) == EN_CHANGE) {
                SendMessage(hRatiosSIsoAbunWnd, WM_GETTEXT, 40,
                    (DWORD)(LPSTR)string);
                selement_abundance = atof(string);
            }
            break;

        case WS_RATIOSRELEMENT:
            if (HIWORD(wParam) == EN_SETFOCUS) {
                pick_element = ALTTRUE;
            }
            else if (hRatiosRElementWnd == GetFocus() &&
                HIWORD(wParam) == EN_CHANGE) {
                pick_element = FALSE;
                SendMessage(hRatiosRElementWnd, WM_GETTEXT, 40,
                    (DWORD)(LPSTR)string);
                relement_mass = atof(string);
                fill_in_label(relement_mass, ALTTRUE);
            }
            break;

        case WS_RATIOSRELABEL:
            if (HIWORD(wParam) == EN_CHANGE) {
                SendMessage(hRatiosRELabelWnd, WM_GETTEXT, 40,
                    (DWORD)(LPSTR)relabel);
            }
            break;

        case WS_RATIOSRISOABUN:
            if (HIWORD(wParam) == EN_CHANGE) {
                SendMessage(hRatiosRIsoAbunWnd, WM_GETTEXT, 40,
                    (DWORD)(LPSTR)string);
                relement_abundance = atof(string);
            }
            break;

        case WS_RATIOSDEPTH:
            if (HIWORD(wParam) == EN_CHANGE) {
                SendMessage(hRatiosDepthWnd, WM_GETTEXT, 40,
                    (DWORD)(LPSTR)string);
                depth = atof(string);
            }
            break;

        case WS_RATIOSALPHA:
            if (HIWORD(wParam) == EN_CHANGE) {
                SendMessage(hRatiosAlphaWnd, WM_GETTEXT, 40,
                    (DWORD)(LPSTR)string);
                alpha = atof(string);
            }
            break;

        case WS_RATIOSRETRIEVE:
            if (HIWORD(wParam) == BN_CLICKED) {
                strcpy_s(ext,5, "dat");
                if ((fn = retrieve_file(hWnd, ext, filename,
                    "SIMS Retrieve")) != NULL) {
                    fclose(fn);
                    SendMessage(hRatiosFilenameWnd, WM_SETTEXT, 0,
                        (DWORD)(LPSTR)filename);
                    return_extension(ext, filename);
                    if (!strcmp(ext, "dat")) {
                        if (read_sims_binary(filename))
                            write_sims_text(filename, TRUE);
                    }
                    else read_sims_text(filename, TRUE);
                }
            }
            break;

        case WS_RATIOSSTANDARD:
            if (HIWORD(wParam) == BN_CLICKED) {
                if (selement <= -1 || matrix <= -1 ||
                    depth <= 0.0 || alpha <= 0.0) {
                    MessageBox(NULL, "Data not filled in!", "Error!", MB_OK);
                    break;
                }
                sprintf_s(outfilename, 128,"%s.rat", fname);
                if ((err = MessageBox(NULL, "Are there leading bad points?",
                    "Information Required", MB_YESNO)) == IDYES)
                    bad_points = TRUE;
                else bad_points = FALSE;
                do_standard();
            }
            break;

        case WS_RATIOSBATCH:
            if (HIWORD(wParam) == BN_CLICKED) {
                if (matrix < 0) {
                    MessageBox(NULL, "Choose Matrix Mass", "Error", MB_OK);
                    break;
                }
                strcpy_s(ext, 5,"dat");
                select_file1(hRatiosWnd, ext, filename, "Choose Data Files");
            }
            break;

        case DB_MODELESS2:
            if (LOWORD(wParam)) {
                sprintf_s(outfilename,128, "%s", fname);
                program = BATCH_RATIOS;
                batch_num = 0;
            }
            break;

        case WS_HIGHYEDIT:
            if (HIWORD(wParam) == EN_CHANGE) {
                SendMessage(hHighYValueWnd, WM_GETTEXT, 40,
                    (DWORD)(LPSTR)string);
                end_yvalue = atoi(string);
            }
            break;

        case WS_LOWYEDIT:
            if (HIWORD(wParam) == EN_CHANGE) {
                SendMessage(hLowYValueWnd, WM_GETTEXT, 40,
                    (DWORD)(LPSTR)string);
                start_yvalue = atoi(string);
            }
            break;

        case WS_HIGHXEDIT:
            if (HIWORD(wParam) == EN_CHANGE) {
                SendMessage(hHighXValueWnd, WM_GETTEXT, 40,
                    (DWORD)(LPSTR)string);
                end_xvalue = atoi(string);
            }
            break;

        case WS_LOWXEDIT:
            if (HIWORD(wParam) == EN_CHANGE) {
                SendMessage(hLowXValueWnd, WM_GETTEXT, 40,
                    (DWORD)(LPSTR)string);
                start_xvalue = atoi(string);
            }
            break;

        case WS_RATIOLABELEDIT:
            if (HIWORD(wParam) == EN_CHANGE) {
                SendMessage(hRatioLabelWnd, WM_GETTEXT, 40,
                    (DWORD)(LPSTR)ratiolabel);
            }
            break;

        case WS_RATIOSPOPUPOK:
            if (HIWORD(wParam) == BN_CLICKED) {
                InvalidateRect(hRatiosGraphPopup, 0L, TRUE);
                UpdateWindow(hRatiosGraphPopup);
            }
            break;

        case WS_RATIOSPOPUPPRINT:
            if (HIWORD(wParam) == BN_CLICKED) {
                print_calibration_plot();
            }
            break;

        case WS_RATIOSPOPUPCANCEL:
            if (HIWORD(wParam) == BN_CLICKED) {
                DestroyWindow(hRatiosGraphPopup);
            }
            break;

        case WS_RATIOSPRINT:
            if (HIWORD(wParam) == BN_CLICKED) {
                print_ratios();
            }
            break;

        case WS_RATIOSSAVE:
            if (HIWORD(wParam) == BN_CLICKED) {
                sprintf_s(outfilename,128, "%.8s.exc", fname);
                save_ratios();
            }
            break;

        case WS_RATIOSCANCEL:
            if (HIWORD(wParam) == BN_CLICKED) {
                DestroyWindow(hRatiosWnd);
                hRatiosWnd = NULL;
                program = NOPROGRAM;
            }
            break;
        }
        break;

    case WM_PAINT:
        if (hWnd == hRatiosGraphPopup) {
            ratiosDC = BeginPaint(hRatiosGraphPopup, &ratiosPaint);
            draw_calibration_plot(ratiosDC, 0, 0);
            EndPaint(hRatiosGraphPopup, &ratiosPaint);
        }
        else return (DefWindowProc(hWnd, message, wParam, lParam));
        break;

    default:
        return (DefWindowProc(hWnd, message, wParam, lParam));
    }

    return (NULL);
}

/*****************************************************************************/

BOOL
read_sims_binary(filename)
char* filename;                     /* SIMS data file to read */
{

    /* Read an Evans ".dat" binary file.  Extract known fields, skipping
       undecipherable bytes.  Write out an ASCII text file.
     */

    int i, j, err, done, x1, y1, x2, y2, FA, CD, Raster, turns, image;
    int time, Hall3, handle;
    unsigned int Hall6;
    unsigned long start, cycle_time, counts;
    double sputter_time, ipc, fpc, matrix_conc, ftemp;
    char internalfn[20], junk[20], species[20];
    char line[2], matrix_name[5], beam_species[3], prim, sec, ims4f[7];
    FILE* infile, * outfile;


    if ((infile = fopen(filename, "rb")) == NULL) {
        sprintf(string, "Cannot open %s\n", filename);
        MessageBox(NULL, string, "Error!", MB_OK);
        return (FALSE);
    }
    /* First 20 (?) bytes are the filename. */

    err = fread(internalfn, 1, 20, infile);

    /* Then there are 14 undecipherable bytes. */

    err = fread(junk, 1, 14, infile);

    /* Next sputter time is stored as a double (byte 35). */

    err = fread(&sputter_time, 1, 8, infile);

    /* Next, 24 undecipherable bytes. */

    err = fread(junk, 1, 20, infile);
    err = fread(junk, 1, 4, infile);

    /* Next, gates as x1, y1, x2, y2 (byte 67). */

    err = fread(&x1, 1, 1, infile);
    err = fread(&y1, 1, 1, infile);
    err = fread(&x2, 1, 1, infile);
    err = fread(&y2, 1, 1, infile);

    /* Now, 8 undecipherable bytes. */

    err = fread(junk, 1, 8, infile);

    /* Next, the initial and final primary current, both doubles (byte 79). */

    err = fread(&ipc, 1, 8, infile);
    err = fread(&fpc, 1, 8, infile);

    /* Next, 8 bytes for the matrix concentration, followed by 5 (?) bytes
       for the matrix species name (byte 95).
     */

    err = fread(&matrix_conc, 1, 8, infile);
    err = fread(matrix_name, 1, 5, infile);

    /* Now, 11 (?) undecipherable bytes. */

    err = fread(junk, 1, 11, infile);

    /* Next, the start time as a clock tick value (long), followed by FA,
       CD, Raster and number of turns (all as int), and then beam species
       (ASCII text string, 3 (?) bytes (byte 119));
     */

    err = fread(&start, 1, 4, infile);
    err = fread(&FA, 1, 2, infile);
    err = fread(&CD, 1, 2, infile);
    err = fread(&Raster, 1, 2, infile);
    err = fread(&turns, 1, 2, infile);
    err = fread(beam_species, 1, 3, infile);

    /* Now, 53 undecipherable bytes. */

    err = fread(junk, 1, 20, infile);
    err = fread(junk, 1, 20, infile);
    err = fread(junk, 1, 13, infile);

    /* Next, the image field (int), primary secondary polarities (chars), and
       the text string "IMF 4F" (byte 187).
     */

    err = fread(&image, 1, 2, infile);
    err = fread(&prim, 1, 1, infile);
    err = fread(&sec, 1, 1, infile);
    err = fread(ims4f, 1, 7, infile);

    /* Now, 317 bytes of junk. */

    for (i = 0; i < 15; i++)
        err = fread(junk, 1, 20, infile);
    err = fread(junk, 1, 17, infile);

    /* Next, the number of masses (int) (byte 535), followed by 16 bytes
       of junk.
     */

    err = fread(&num_masses, 1, 2, infile);
    err = fread(junk, 1, 16, infile);

    /* Next, the table of depth profile information.  Fields are: the
       species name (ASCII text, 5 to 16 (?) bytes), mass (double), offset
       (int), junk (10 bytes), time (int), Hall6 (int), Hall3 (int), junk
       (18 bytes) (byte 563).
     */

    for (i = 0; i < num_masses; i++) {
        err = fread(species, 1, 6, infile);
        err = fread(junk, 1, 10, infile);
        err = fread(&ftemp, 1, 8, infile);
        mass[i] = ftemp;
        err = fread(&offset[i], 1, 2, infile);
        err = fread(junk, 1, 10, infile);
        err = fread(&time, 1, 2, infile);
        err = fread(&Hall6, 1, 2, infile);
        err = fread(&Hall3, 1, 2, infile);
        err = fread(junk, 1, 18, infile);
    }

    for (i = 0; i < MAXMASSES; i++)
        Block[i] = GlobalFree(Block[i]);
    for (i = 0; i < num_masses; i++) {
        if ((Block[i] = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
            sizeof(struct data) * (long)MAXCYCLES)) == NULL) {
            sprintf(string, "read_sims_binary: GlobalAlloc %d failed", i);
            MessageBox(NULL, string, "Error!", MB_OK);
            return (FALSE);
        }
        total_counts[i] = 0;
        symbol[i] = i % MAXSYMBOLS;
        colour[i] = i % MAXCOLOURS;
        sprintf(labels[i], "%5.1f", mass[i]);
    }

    /* Next the data points.  The counts for each mass (as long) are stored
       in order, followed by a time, in clock ticks, for each cycle (as long).
     */

    force_extension("qpr", filename);
    if ((outfile = fopen(filename, "wt")) == NULL) {
        sprintf(string, "Cannot open %s\n", filename);
        MessageBox(NULL, string, "Error!", MB_OK);
        return (FALSE);
    }
    for (num_cycles = 0, done = FALSE; !done && num_cycles < MAXCYCLES; num_cycles++) {
        for (i = 0; i < num_masses && !done; i++) {
            if ((points = (struct data far*) GlobalLock(Block[i])) == NULL) {
                sprintf(string, "read_sims_binary: GlobalLock1 %d failed", i);
                MessageBox(NULL, string, "Error!", MB_OK);
                return (FALSE);
            }
            err = fread(&counts, 1, 4, infile);
            if (err == 0) done = TRUE;
            else {
                points[num_cycles].counts = counts;
                total_counts[i] += counts;
                if (i == 0) fprintf(outfile, "%ld", counts);
                else fprintf(outfile, ",%ld", counts);
            }
            GlobalUnlock(Block[i]);
        }
        if (!done) {
            fprintf(outfile, "\n");
            fread(&cycle_time, 1, 4, infile);
            for (i = 0; i < num_masses; i++) {
                if ((points = (struct data far*) GlobalLock(Block[i])) == NULL) {
                    sprintf(string, "read_sims_binary: GlobalLock2 %d failed", i);
                    MessageBox(NULL, string, "Error!", MB_OK);
                    return (FALSE);
                }
                points[num_cycles].time = (cycle_time - start) / 18;
                GlobalUnlock(Block[i]);
            }
        }
    }
    num_cycles--;

    fclose(outfile);
    fclose(infile);
    return (TRUE);
}

/*****************************************************************************/
BOOL write_sims_text(filename, from_ratios)
TCHAR* filename;                         /* SIMS data file name */
int from_ratios;                        /* TRUE if coming from ratios module */
{

    /* Write out stored sims data to a text file. */

    int i, j;
    FILE* outfile;

    force_extension("txt", filename);
    errno_t err = fopen_s(&outfile, filename, "wt");
    if (err!=0) {
        sprintf_s(string, 128,"Cannot open %s\n", filename);
        MessageBox(NULL, string, "Error!", MB_OK);
        return (FALSE);
    }

    if (from_ratios)
        for (i = 0; i < MAXMASSES; i++)
            SendMessage(hRatiosMassesWnd[i], WM_SETTEXT, 0, (DWORD)(LPSTR)"");
    fprintf(outfile, "%d masses\n", num_masses);
    fprintf(outfile, "%d cycles\n", num_cycles);

    fprintf(outfile, "mass   offset\n");
    for (i = 0; i < num_masses; i++) {
        fprintf(outfile, "%5.1f  %4d\n", mass[i], offset[i]);
        sprintf_s(string,128, "%5.1f", mass[i]);
        if (from_ratios) SendMessage(hRatiosMassesWnd[i], WM_SETTEXT, 0,
            (DWORD)(LPSTR)string);
    }

    for (i = 0; i < num_masses; i++) {
        if ((points = (struct data*) GlobalLock(Block[i])) == NULL) {
            sprintf_s(string, 128,"write_sims_text: GlobalLock %d failed", i);
            MessageBox(NULL, string, "Error!", MB_OK);
            return (FALSE);
        }
        fprintf(outfile, "\nmass %5.1f\n", mass[i]);
        for (j = 0; j < num_cycles; j++) {
            fprintf(outfile, " %5d %9lu", points[j].time, points[j].counts);
            if (j % 4 == 3 && j != num_cycles - 1) fprintf(outfile, "\n");
        }
        fprintf(outfile, "\n");
        GlobalUnlock(Block[i]);
    }
    fprintf(outfile, "\n");
    for (i = 0; i < num_masses; i++)
        fprintf(outfile, "total counts for mass %5.1f: %15ld\n",
            mass[i], total_counts[i]);
    fclose(outfile);
    return (TRUE);
}

/******************************************************************************/

BOOL read_sims_text(filename, from_ratios)
TCHAR* filename;                     /* SIMS data file to read */
int from_ratios;                    /* TRUE if coming from ratios module */
{

    /* Read the text (".txt") version of an Evans file.
     */

    int i, j, err, done;
    TCHAR chr;
    FILE* infile, * outfile;

    errno_t error = fopen_s(&infile, filename, "wt");
    if (error!=0) {
        sprintf_s(string, 128, "Cannot open %s\n", filename);
        MessageBox(NULL, string, "Error!", MB_OK);
        return (FALSE);
    }

    fgets(string, 78, infile);           /* read number of masses */
    num_masses = atoi(string);
    fgets(string, 78, infile);           /* read the number of cycles */
    num_cycles = atoi(string);
    fgets(string, 78, infile);           /* read mass setup header */
    for (i = 0; i < num_masses; i++) {    /* read mass setup data */
        fscanf_s(infile, " %f %d%c", &mass[i], &offset[i], &chr);
        if (from_ratios) {
            sprintf_s(string,128, "%5.1f", mass[i]);
            SendMessage(hRatiosMassesWnd[i], WM_SETTEXT, 0, (DWORD)(LPSTR)string);
        }
    }

    for (i = 0; i < MAXMASSES; i++)
        if (Block[i] != NULL) {
            GlobalFree(Block[i]);
            Block[i] = NULL;
        }

    for (i = 0; i < num_masses; i++) {
        fgets(string, 78, infile);           /* read blank line */
        fgets(string, 78, infile);           /* read mass header */
        if ((Block[i] = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
            sizeof(struct data) * num_cycles)) == NULL) {
            sprintf_s(string, 128, "read_sims_text: GlobalAlloc %d failed", i);
            MessageBox(NULL, string, "Error!", MB_OK);
            return (FALSE);
        }
        if ((points = (struct data*) GlobalLock(Block[i])) == NULL) {
            sprintf_s(string,128, "read_sims_text: GlobalLock %d failed", i);
            MessageBox(NULL, string, "Error!", MB_OK);
            return (FALSE);
        }
        for (j = 0, total_counts[i] = 0; j < num_cycles; j++) {
            fscanf_s(infile, " %d %lu", &points[j].time, &points[j].counts);
            total_counts[i] += points[j].counts;
        }
        fgets(string, 10, infile);           /* read last new line */
        GlobalUnlock(Block[i]);
    }
    fclose(infile);
    return (TRUE);
}

/*****************************************************************************/

static BOOL do_standard()
{

    /* Use the selected file for a "standard" calculation.
       Throw out bad points at the beginning.  Find the point the curve flattens
       out and average the background counts starting at that point.  Adjust
       the depth and total counts for the cycles thrown out, and adjust the
       total counts by the background counts.  Calculate calibration points
       from the maximum to the beginning of the background.  Do a least
       squares fit of the calibration points to get calibration coefficients.
     */

    int i, j, success, old_num_cycles, max, bg, start;
    float total_bg, ave_bg, element_total;
    float raster = 0.0;
    float final_time = 0.0;
    float final_depth;
    float ave_matrix_counts, logppm, logratio;
    float fconst, delta, sumx, sumy, sumxy, sumx2;
    FILE* outfn;
    errno_t err = fopen_s(&outfn, outfilename, "at");
    if (err!= 0) {
        sprintf_s(string,128, "Cannot open %s\n", filename);
        MessageBox(NULL, string, "Error!", MB_OK);
        return (FALSE);
    }
    fprintf(outfn, "%s\n", filename);

    old_num_cycles = num_cycles;
    if ((points = (struct data*) GlobalLock(Block[selement])) == NULL) {
        sprintf_s(string,128, "do_standard: GlobalLock failed");
        MessageBox(NULL, string, "Error!", MB_OK);
        return (FALSE);
    }

    /* Find maximum, skipping leading bad points. */

    if (bad_points) {
        for (start = 0; points[start].counts >= points[start + 1].counts ||
            points[start + 1].counts >= points[start + 2].counts; start++);
        for (j = start + 1, max = start; j < num_cycles; j++)
            if (points[j].counts > points[max].counts) max = j;

        /* Replace leading bad points with symmetrical
           ones from the other side of the maximum.
         */

        for (--start; start >= 0; start--) {
            total_counts[selement] -= points[start].counts;
            points[start].counts = points[max + (max - start)].counts;
            total_counts[selement] += points[start].counts;
        }
    }
    else
        for (i = 1, max = 0; i < num_cycles; i++)
            if (points[i].counts > points[max].counts) max = i;
    /*
    sprintf (string,"max is %d",max);
    MessageBox (NULL,string,"testing",MB_OK);
    */
    ave_matrix_counts = total_counts[matrix] / num_cycles;
    /*
    sprintf (string,"num_cycles %d, ave_matrix_counts %f",num_cycles,ave_matrix_counts);
    MessageBox (NULL,string,"testing",MB_OK);
    */
    fprintf(outfn, "num_cycles %d\n", num_cycles);

    fprintf(outfn, "raster %.2f\nfinal time %.2f\n", raster, final_time);
    fprintf(outfn, "final depth %.2f\nalpha %6.2f\n", depth, alpha);
    /*
    sprintf (string,"final depth %.2f\nalpha %6.2f\n",depth,alpha);
    MessageBox (NULL,string,"testing",MB_OK);
    */

    /* Find the beginning of the background. */

    for (j = max + 1, bg = 0; !bg && j < num_cycles; j++)
        if (points[j].counts < points[max].counts * 0.10)
            if (points[j].counts > points[j - 1].counts ||
                points[j].counts < 4) bg = j - 1;
    fprintf(outfn, "max index %d, max counts %lu\n", max, points[max].counts);
    fprintf(outfn, "background index %d, background counts %lu\n", bg, points[bg].counts);
    /*
    sprintf (string,"max index %d, max counts %lu\n",max,points[max].counts);
    MessageBox (NULL,string,"testing",MB_OK);
    sprintf (string,"background index %d, background counts %lu\n",bg,points[bg].counts);
    MessageBox (NULL,string,"testing",MB_OK);
    */

    /* Find the average background counts, the background over all cycles
       and the corrected element total counts.
     */

    for (j = bg, num = 0, total_bg = 0.0; j < num_cycles; j++, num++)
        total_bg += points[j].counts;
    /*
    sprintf (string,"num is %d",num);
    MessageBox (NULL,string,"testing",MB_OK);
    */
    ave_bg = total_bg / num;
    total_bg = ave_bg * num_cycles;
    element_total = total_counts[selement] - total_bg;
    fprintf(outfn, "ave_bg %.2f\ntotal background %.2f\nelement total %.2f\n",
        ave_bg, total_bg, element_total);
    /* Find the element/matrix ratio from the maximum to the background,
       and the element ppm for the same range.
     */

    fconst = alpha / (depth * element_total) * 10000.0;
    bg_ratio = ratio[0] = ave_bg / ave_matrix_counts;
    bg_ppm = ppm[0] = total_bg * fconst / selement_abundance;
    for (j = bg - 1, num = 1; j >= max; j--, num++) {
        ratio[num] = points[j].counts / ave_matrix_counts;
        if (ratio[num] <= 0.0) ratio[num] = 1e-20;
        ppm[num] = points[j].counts * num_cycles * fconst / selement_abundance;
        if (ppm[num] <= 0.0) ppm[num] = 1e-20;
    }
    max_ratio = ratio[num - 1];
    max_ppm = ppm[num - 1];
    fprintf(outfn, "ratio[max] %.2e, ppm[max] %.2f\n", max_ratio, max_ppm);
    fprintf(outfn, "ratio[background] %.2e, ppm[background] %.2f\n", ratio[0], ppm[0]);
    sprintf_s(string, 128, "%.2e", ratio[0]);
    SendMessage(hRatiosBgRatioWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)string);
    sprintf_s(string, 128, "%.2f", ppm[0]);
    SendMessage(hRatiosBgPPMWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)string);
    sprintf_s(string,128, "%.2e", max_ratio);
    SendMessage(hRatiosMaxRatioWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)string);
    sprintf_s(string,128, "%.2f", max_ppm);
    SendMessage(hRatiosMaxPPMWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)string);
    GlobalUnlock(Block[selement]);

    /* Do a least squares fit of ratio (x) vs. ppm (y). */

    for (i = 0, sumx = 0.0, sumy = 0.0, sumxy = 0.0, sumx2 = 0.0; i < num; i++) {
        logratio = log10(ratio[i]);
        logppm = log10(ppm[i]);
        sumx += logratio;
        sumy += logppm;
        sumxy += logratio * logppm;
        sumx2 += logratio * logratio;
    }
    delta = num * sumx2 - sumx * sumx;
    intercept = (sumx2 * sumy - sumx * sumxy) / delta;
    slope = (num * sumxy - sumx * sumy) / delta;
    fprintf(outfn, "slope %.3f\nintercept %.3f\n\n", slope, intercept);
    sprintf_s(string,128, "%7.5f", slope);
    SendMessage(hRatiosSlopeWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)string);
    sprintf_s(string,128, "%7.5f", intercept);
    SendMessage(hRatiosInterceptWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)string);
    fclose(outfn);

    create_ratios_graph_window(hInst);
    InvalidateRect(hRatiosGraphPopup, 0L, TRUE);
}

/******************************************************************************/

static BOOL remove_points(partial, filename)
int partial;                        /* TRUE if only removing points > 1E7 */
TCHAR* filename;                     /* SIMS data filename to modify */
{

    /* Remove all points which in the matrix mass vary by more than 10% from the
       average of all points after the first one.  First remove all points
       with counts greater than 1E7 as counts should never get that high and
       they distort the average.
     */

    int i, j, k, response;
    long difference;
    float average, ten_percent;

    if (matrix < 0) return (FALSE);
    if ((points = (struct data*) GlobalLock(Block[matrix])) == NULL) {
        sprintf_s(string,128, "remove_points: GlobalLock %d failed", matrix);
        MessageBox(NULL, string, "Error!", MB_OK);
        return (FALSE);
    }
    for (j = 0; j < num_cycles; ) {
        if (points[j].counts > 1E7 || points[j].counts <= 0) {
            total_counts[matrix] -= points[j].counts;
            for (k = j; k < num_cycles - 1; k++)
                points[k] = points[k + 1];
            for (i = 0; i < num_masses; i++) {
                if (i != matrix) {
                    if ((pts = (struct data*) GlobalLock(Block[i])) == NULL) {
                        sprintf_s(string,128, "remove_points: GlobalLock %d failed", i);
                        MessageBox(NULL, string, "Error!", MB_OK);
                        GlobalUnlock(Block[matrix]);
                        return (FALSE);
                    }
                    total_counts[i] -= pts[j].counts;
                    for (k = j; k < num_cycles - 1; k++)
                        pts[k] = pts[k + 1];
                    GlobalUnlock(Block[i]);
                }
            }
            num_cycles--;
        }
        else j++;
    }
    if (partial) {
        GlobalUnlock(Block[matrix]);
        return (TRUE);
    }

    difference = labs(points[1].counts - points[num_cycles - 1].counts);
    if (difference > points[1].counts * 0.2) {
        response = MessageBox(NULL, "count variation exceeds 20%", filename,
            MB_OKCANCEL);
        GlobalUnlock(Block[matrix]);
        if (response == IDCANCEL) return (FALSE);
        else return (TRUE);
    }

    average = (total_counts[matrix] - points[0].counts) / (num_cycles - 1);
    ten_percent = average * 0.1;
    for (j = 0; j < num_cycles; ) {
        if (fabs(average - points[j].counts) > ten_percent) {
            total_counts[matrix] -= points[j].counts;
            for (k = j; k < num_cycles - 1; k++)
                points[k] = points[k + 1];
            for (i = 0; i < num_masses; i++) {
                if (i != matrix) {
                    if ((pts = (struct data*) GlobalLock(Block[i])) == NULL) {
                        sprintf_s(string,128, "remove_points: GlobalLock %d failed", i);
                        MessageBox(NULL, string, "Error!", MB_OK);
                        GlobalUnlock(Block[matrix]);
                        return (FALSE);
                    }
                    total_counts[i] -= pts[j].counts;
                    for (k = j; k < num_cycles - 1; k++)
                        pts[k] = pts[k + 1];
                    GlobalUnlock(Block[i]);
                }
            }
            num_cycles--;
        }
        else j++;
    }
    GlobalUnlock(Block[matrix]);
    return (TRUE);
}

/******************************************************************************/

static BOOL write_ratios_file(filename)
TCHAR* filename;                     /* SIMS data filename to modify */
{

    /* For all the masses, calculate the ratio of total counts to the matrix mass
       total counts.  First throw out points which are more than 10% off the
       average of the matrix mass counts.  Then recalculate the total counts.
       Then do the ratios.
     */

    int i, j, element;
    float tratio, element_mass;
    FILE* outfn;

    if (relement_mass > 0) element_mass = relement_mass;
    else element_mass = selement_mass;

    for (i = 0; i < num_masses; i++) {
        if (fabs(mass[i] - matrix_mass) < 0.2) {
            matrix = i;
            break;
        }
    }
    if (i >= num_masses) {
        for (i = 0; i < num_masses; i++) {
            if (fabs(mass[i] - matrix_mass) < 0.5) {
                matrix = i;
                sprintf_s(string,128, "In %s, matrix mass is %5.1f (standard %5.1f)",
                    filename, mass[i], matrix_mass);
                MessageBox(NULL, string, "Warning", MB_OK);
                break;
            }
        }
    }
    if (i >= num_masses) {
        sprintf_s(string,128, "In %s, unable to match matrix mass", filename);
        MessageBox(NULL, string, "Error", MB_OK);
        ppm[batch_num] = -1;
        ratio[batch_num] = -1;
        return (TRUE);
    }
    for (i = 0; i < num_masses; i++) {
        if (fabs(mass[i] - element_mass) < 0.2) {
            element = i;
            break;
        }
    }
    if (i >= num_masses) {
        for (i = 0; i < num_masses; i++) {
            if (fabs(mass[i] - element_mass) < 0.5) {
                element = i;
                sprintf_s(string, 128,"In %s, element mass is %5.1f (standard %5.1f)",
                    filename, mass[i], element_mass);
                MessageBox(NULL, string, "Warning", MB_OK);
                break;
            }
        }
    }
    if (i >= num_masses) {
        sprintf_s(string,128, "In %s, unable to match element mass", filename);
        MessageBox(NULL, string, "Error", MB_OK);
        ppm[batch_num] = -1;
        ratio[batch_num] = -1;
        return (TRUE);
    }

    if (!remove_points(FALSE, filename)) {
        ppm[batch_num] = -1;
        ratio[batch_num] = -1;
        return (TRUE);
    }
    
    errno_t err = fopen_s(&outfn, outfilename, "at");

    if (err != 0) {
        sprintf_s(string,128, "Cannot open %s\n", filename);
        MessageBox(NULL, string, "Error!", MB_OK);
        return (FALSE);
    }

    char* filename_start, * filename_end;
    char* filename_without_extension = NULL;
    filename_start = strrchr(filename, '\\');
    if (filename_start != NULL) {
        filename_start++;  // Move past the backslash
    }
    else {
        filename_start = filename;  // No backslash found, the path is the filename
    }
    filename_end = strrchr(filename_start, '.');
    if (filename_end != NULL) {
        filename_without_extension = (char*)malloc(filename_end - filename_start + 1);
        if (filename_without_extension != NULL) {
            strncpy(filename_without_extension, filename_start, filename_end - filename_start);
            filename_without_extension[filename_end - filename_start] = '\0';  // Null terminate the string
            printf("Filename without extension: %s\n", filename_without_extension);
            
        }
        else {
            printf("Memory allocation failed.\n");
        }
    }
    else {
        printf("Filename has no extension: %s\n", filename_start);
    }
    

    fprintf(outfn, "%s\n", filename_without_extension);
    


    for (i = 0; i < num_masses; i++) {
        if (i != matrix) {
            if (relement_mass > 0)
                tratio = (float)total_counts[i] / (float)total_counts[matrix]
                * (selement_abundance / relement_abundance);
            else tratio = (float)total_counts[i] / (float)total_counts[matrix];
            if (i == element && slope != 0.0) {
                ppm[batch_num] = pow(10, slope * log10(tratio) + intercept);
                fprintf(outfn, "mass %5.1f: ratio %.2e, ppm %.2f\n", mass[i],
                    tratio, ppm[batch_num]);
                ratio[batch_num] = tratio;
            }
            else fprintf(outfn, "mass %5.1f: ratio %.2e\n", mass[i], tratio);
        }
    }

    /*for (i = 0; i < num_masses; i++) {
        if (i != matrix) {
            if (relement_mass > 0)
                tratio = (float)total_counts[i] / (float)total_counts[matrix]
                * (selement_abundance / relement_abundance);
            else tratio = (float)total_counts[i] / (float)total_counts[matrix];
            if (i == element && slope != 0.0) {
                ppm[batch_num] = pow(10, slope * log10(tratio) + intercept);
                fprintf(outfn, " %.2f\n",ppm[batch_num]);
                ratio[batch_num] = tratio;
            }
            
        }
    }*/
    fprintf(outfn, "\n");


    fclose(outfn);

    free(filename_without_extension);  
    return (TRUE);
}

/******************************************************************************/

void batch_ratios()
{

    /* Read each selected data file, do the smoothing and calculate the ratios.
     */

    int i, success;
    TCHAR ext[5];

    /*if ((filenames = (struct fname_record*) GlobalLock(Files)) == NULL) {
        MessageBox(NULL, "batch_integrate: filenames GlobalLock failed", "Error", MB_OK);
        program = NOPROGRAM;
        return TRUE;
    }*/
    
    strcpy_s(filename,128, filename_array[batch_num]);
    /*GlobalUnlock(Files);*/
    SendMessage(hRatiosFilenameWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)filename);
    return_extension(ext, filename);
    if (!strcmp(ext, "dat")) {
        if (success = read_sims_binary(filename))
            write_sims_text(filename, TRUE);
    }
    else success = read_sims_text(filename, TRUE);
    if (success) {
        success = write_ratios_file(filename);
        if (!success) {
            MessageBox(NULL, "Error writing ratios file, aborting",
                "Error!", MB_OK);
            program = NOPROGRAM;
        }
    }
    else program = NOPROGRAM;
    batch_num++;
    if (batch_num == total_fnames) {
        MessageBox(NULL, "Done", "", MB_OK);
        program = NOPROGRAM;
    }
}

/*****************************************************************************/

static BOOL print_ratios()
{

    /* Print the data file names with their corresponding masses ratios and ppms. */

    HDC printDC;
    POINT PageSize;
    TEXTMETRIC TextMetric;
    int i, j, LineSpace, LinesPerPage, CurrentLine, width;

    printDC = print_file(hRatiosWnd, FALSE);
    if (printDC == NULL) {
        MessageBox(NULL, "print_ratios: Cannot Print", "Error", MB_OK);
        return (FALSE);
    }
    Escape(printDC, STARTDOC, 12, (LPSTR)"Ratios Print", 0L);
    Escape(printDC, GETPHYSPAGESIZE, NULL, NULL, (LPSTR)&PageSize);
    GetTextMetrics(printDC, &TextMetric);
    LineSpace = TextMetric.tmHeight + TextMetric.tmExternalLeading;
    LinesPerPage = PageSize.y / LineSpace;
    SetTextAlign(printDC, TA_TOP | TA_LEFT);

    CurrentLine = 1;
    sprintf_s(string, 128,"              %s/%s         %s,wt%", selabel, mlabel, selabel);
    TextOut(printDC, 0, CurrentLine++ * LineSpace, string, strlen(string));
    sprintf_s(string,128, "background  %.2e      %.2f", bg_ratio, bg_ppm);
    TextOut(printDC, 0, CurrentLine++ * LineSpace, string, strlen(string));
    sprintf_s(string,128, "       max  %.2e      %.2f", max_ratio, max_ppm);
    TextOut(printDC, 0, CurrentLine++ * LineSpace, string, strlen(string));
    CurrentLine++;
    sprintf_s(string,128, "slope %8.6f, intercept %8.6f", slope, intercept);
    TextOut(printDC, 0, CurrentLine++ * LineSpace, string, strlen(string));
    CurrentLine++;

    sprintf_s(string,128, " Grain       %s/%s       %s,wt%", relabel, mlabel, relabel);
    TextOut(printDC, 0, CurrentLine++ * LineSpace, string, strlen(string));

    if ((filenames = (struct fname_record*) GlobalLock(Files)) == NULL) {
        MessageBox(NULL, "batch_integrate: filenames GlobalLock failed", "Error", MB_OK);
        return TRUE;
    }
    for (i = 0; i < total_fnames; i++) {
        strcpy_s(filename,128, filenames[i].fname);
        for (j = 0; j < strlen(filename) && filename[j] != '.'; j++);
        filename[j] = '\0';
        sprintf_s(string,128, "%s   %.2e      %.2f", filename, ratio[i], ppm[i]);
        TextOut(printDC, 0, CurrentLine++ * LineSpace, string, strlen(string));
        if (CurrentLine >= LinesPerPage - 3) {
            Escape(printDC, NEWFRAME, 0, 0L, 0L);
            SetTextAlign(printDC, TA_TOP | TA_LEFT);
            CurrentLine = 1;
        }
    }
    GlobalUnlock(Files);

    Escape(printDC, NEWFRAME, 0, 0L, 0L);
    Escape(printDC, ENDDOC, 0, 0L, 0L);
    DeleteDC(printDC);
    printDC = NULL;
}

/*****************************************************************************/
static BOOL save_ratios()
{

    /* Save the data file names with their corresponding masses ratios and ppms. */
    /* Use the same filename as specified for the ".rat" file, but use ".exc". */

    int i, j;
    FILE* fn;
    errno_t err = fopen_s(&fn, outfilename, "wt");
    if (err != 0) {
        MessageBox(NULL, "Cannot open exc file!", "Error!", MB_OK);
        return (FALSE);
    }
    fprintf(fn, " Grain,%s/%s,%s-wt%\n", relabel, mlabel, relabel);

    if ((filenames = (struct fname_record*) GlobalLock(Files)) == NULL) {
        MessageBox(NULL, "batch_integrate: filenames GlobalLock failed", "Error", MB_OK);
        return (TRUE);
    }
    for (i = 0; i < total_fnames; i++) {
        strcpy_s(filename,128, filenames[i].fname);
        for (j = 0; j < strlen(filename) && filename[j] != '.'; j++);
        filename[j] = '\0';
        fprintf(fn, "%s,%.2e,%.2f\n", filename, ratio[i], ppm[i]);
    }
    GlobalUnlock(Files);
    fclose(fn);
}

