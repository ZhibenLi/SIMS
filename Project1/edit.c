#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "dlgopen.h"
#include "sims.h"
#include <strsafe.h>
#include <tchar.h>
#include "data_struct.h"



HWND hEditSetWnd, hEditGraphWnd;

#define PLOTX2         430
#define PLOTY2         420
#define ORIGINX         40
#define S2ORIGINX      460
#define ENDX           380
#define S2ENDX         800
#define ORIGINY        380
#define ENDY            30

extern FILE* retrieve_file();


extern HINSTANCE hInst;
extern HWND hwnd;                       /* handle to main window */
extern HANDLE Files;
extern struct fname_record* filenames;
extern int total_fnames;
extern char* filename_array[120];

extern HWND hRatiosWnd;                        /* child window for ratio info */


HWND hEditSetWnd, hEditGraphWnd;
HWND hEditCancel, hEditRedraw;
HWND hEditPrint, hEditRetrieve;
HWND hEditNext, hEditFilenameTxt;
HWND hEditFilenameBox, hEditFilenameWnd;
HWND hEditPopup, hEditMassTxt;
HWND hEditMass, hEditEditOK;
HWND hEditEditCancel, hEditEdit;
HWND hEditSymbols, hEditLabels;
HWND hEditTimeTxt, hEditTimeBox, hEditTimeWnd;
HWND hPrintOptionsPopup;
HWND hLandscape, hPrintOptionsOK;
HWND hFullSize, hSmaller, hSmallest;
HWND hPlotWnd, hScatterWnd, hDouble;
HWND hPlotFontEditTxt, hPlotFontEditBox;
HWND hPlotFontEditWnd, hEditPrintOptions;
HWND hEditTitleTxt, hEditTitleBox, hEditTitleWnd;
HWND hSymbolPopup, hOutputSym[MAXSYMBOLS];
HWND hOutputCol[MAXCOLOURS], hOutputMassWnd;
HWND hOutputSymTxt, hOutputColTxt, hOutputOK;
HWND hEditLabelsPopup, hLabelMassTxt, hLabelMass;
HWND hLabelEditBox, hLabelEditWnd, hLabelCancel, hLabelOK;


extern HANDLE Block[MAXMASSES];
extern struct data* points, * pts;
extern int num_masses, num_cycles;
extern int time_period;
extern float mass[MAXMASSES];
extern int batch_num;                       /* counter for batch process */

int screensize = 10;                        /* text sizes */
int symbol[MAXMASSES];
int colour[MAXMASSES];
TCHAR labels[MAXMASSES][MAXLABELLENGTH];     /* mass labels */
int curr_mass, edit, draw;
int start_xvalue, end_xvalue;
int start_yvalue, end_yvalue;
double onex, oney;
double ponex, poney;                      /* for drawing points */
float y_inc, x_inc;                      /* pixels to move per one y or x value */
int originx, originy;                    /* current plot 0,0 point */
int shift;                         /* print double second plot x shift amount */
HFONT hSScriptFont;
static int redraw_index, redraw_x;          /* point redrawing parameters */
static int x, y;
static int landscape, fullsize, print_double;     /* printing flags */
static int printersize, smaller, smallest;
static int printer_override, choice;
static int print_size, plotnum;
static HDC plotDC;
static TCHAR title[128], filename[128];
static TCHAR string[128];

static long col[7] = { BLACK,RED,GREEN,BLUE,AQUA,PURPLE,ORANGE };
static char colourtxt[7][7] = { "black","red","green","blue","aqua","purple","orange" };
extern BOOL read_sims_binary(TCHAR*);
extern BOOL read_sims_text(TCHAR*, int);
extern BOOL write_sims_text(TCHAR*, int);

/*****************************************************************************/

static BOOL create_print_opts_window(hInstance)
HANDLE hInstance;
{

    /*  Create the "Print Options" popup window and fill it with controls.
     */

    hPrintOptionsPopup = CreateWindow("EditClass", "Print Options",
        WS_VISIBLE | WS_POPUP | WS_BORDER | WS_CAPTION, 200, 80, 310, 210,
        hwnd, NULL, hInstance, NULL);
    if (!hPrintOptionsPopup) return (FALSE);

    hLandscape = CreateWindow(L"Button", "Landscape",
        BS_CHECKBOX | WS_CHILD | WS_VISIBLE, 60, 20, 90, 24,
        hPrintOptionsPopup, WS_PRINTLANDSCAPE, hInstance, NULL);

    hDouble = CreateWindow(L"Button", "Double",
        BS_CHECKBOX | WS_CHILD | WS_VISIBLE, 180, 20, 80, 24,
        hPrintOptionsPopup, WS_PRINTDOUBLE, hInstance, NULL);

    hFullSize = CreateWindow(L"Button", "Full Size",
        BS_CHECKBOX | WS_CHILD | WS_VISIBLE, 20, 60, 90, 24,
        hPrintOptionsPopup, WS_PRINTFULLSIZE, hInstance, NULL);

    hSmaller = CreateWindow(L"Button", "Smaller",
        BS_CHECKBOX | WS_CHILD | WS_VISIBLE, 120, 60, 80, 24,
        hPrintOptionsPopup, WS_PRINTSMALLER, hInstance, NULL);

    hSmallest = CreateWindow(L"Button", "Smallest",
        BS_CHECKBOX | WS_CHILD | WS_VISIBLE, 210, 60, 90, 24,
        hPrintOptionsPopup, WS_PRINTSMALLEST, hInstance, NULL);

    hPlotFontEditTxt = CreateWindow(L"Static", "Font Size",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 60, 100, 100, 20,
        hPrintOptionsPopup, -1, hInstance, NULL);

    hPlotFontEditBox = CreateWindow(L"Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 138, 98, 44, 24,
        hPrintOptionsPopup, -1, hInstance, NULL);

    hPlotFontEditWnd = CreateWindow(L"Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 140, 100, 40, 20,
        hPrintOptionsPopup, WS_PLOTFONTEDIT, hInstance, NULL);

    hPrintOptionsOK = CreateWindow(L"Button", "OK",
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 140, 150, 40, 24,
        hPrintOptionsPopup, WS_PRINTOPTIONSOK, hInstance, NULL);

    return (TRUE);
}

/*****************************************************************************/

static BOOL create_symbol_window(hInstance)
HANDLE hInstance;
{

    /*  Create the "Symbol" popup window and fill it with controls.
     */

    int i, j;

    hSymbolPopup = CreateWindow("EditClass", "Symbols", WS_POPUP |
        WS_VISIBLE | WS_BORDER | WS_CAPTION, 100, 160, 380, 240,
        NULL, NULL, hInstance, NULL);
    if (!hSymbolPopup) return (FALSE);

    hOutputMassWnd = CreateWindow(L"Static", NULL,
        SS_LEFT | WS_CHILD | WS_VISIBLE, 170, 10, 80, 20,
        hSymbolPopup, -1, hInstance, NULL);

    hOutputSymTxt = CreateWindow(L"Static", "Shapes",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 10, 40, 60, 20,
        hSymbolPopup, -1, hInstance, NULL);

    for (i = 0; i < MAXSYMBOLS / 2; i++) {
        hOutputSym[i] = CreateWindow(L"Button", NULL,
            BS_CHECKBOX | WS_CHILD | WS_VISIBLE, 74 + 50 * i, 40, 20, 20,
            hSymbolPopup, WS_OUTPUTSYM0 + i, hInstance, NULL);
    }

    for (i = MAXSYMBOLS / 2, j = 0; i < MAXSYMBOLS; i++, j++) {
        hOutputSym[i] = CreateWindow(L"Button", NULL,
            BS_CHECKBOX | WS_CHILD | WS_VISIBLE, 74 + 50 * j, 70, 20, 20,
            hSymbolPopup, WS_OUTPUTSYM0 + i, hInstance, NULL);
    }

    hOutputColTxt = CreateWindow(L"Static", "Colours",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 10, 100, 60, 20,
        hSymbolPopup, -1, hInstance, NULL);

    for (i = 0; i < MAXCOLOURS / 2; i++) {
        hOutputCol[i] = CreateWindow(L"Button", colourtxt[i],
            BS_CHECKBOX | WS_CHILD | WS_VISIBLE, 74 + 72 * i, 100, 64, 20,
            hSymbolPopup, WS_OUTPUTCOL0 + i, hInstance, NULL);
    }

    for (i = MAXCOLOURS / 2, j = 0; i < MAXCOLOURS; i++, j++) {
        hOutputCol[i] = CreateWindow(L"Button", colourtxt[i],
            BS_CHECKBOX | WS_CHILD | WS_VISIBLE, 74 + 72 * j, 130, 64, 20,
            hSymbolPopup, WS_OUTPUTCOL0 + i, hInstance, NULL);
    }

    hOutputOK = CreateWindow(L"Button", "OK",
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 160, 170, 40, 24,
        hSymbolPopup, WS_OUTPUTOK, hInstance, NULL);

    curr_mass = 0;
    for (i = 0; i < MAXSYMBOLS; i++)
        if (i == symbol[curr_mass])
            SendMessage(hOutputSym[i], BM_SETCHECK, 1, 0L);
        else SendMessage(hOutputSym[i], BM_SETCHECK, 0, 0L);
    for (i = 0; i < MAXCOLOURS; i++)
        if (i == colour[curr_mass])
            SendMessage(hOutputCol[i], BM_SETCHECK, 1, 0L);
        else SendMessage(hOutputCol[i], BM_SETCHECK, 0, 0L);
    sprintf_s(string,128, "Mass %5.1f", mass[curr_mass]);
    SendMessage(hOutputMassWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)string);
}

/*****************************************************************************/


static BOOL create_labels_window(hInstance)
HANDLE hInstance;
{

    /*  Create the "Labels" popup window and fill it with controls.
     */

    hEditLabelsPopup = CreateWindow("EditClass", "Labels",
        WS_POPUP | WS_BORDER | WS_CAPTION, 40, 160, 160, 140,
        NULL, NULL, hInstance, NULL);
    if (!hEditLabelsPopup) return (FALSE);

    hLabelMassTxt = CreateWindow(L"Static", "Mass",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 10, 10, 40, 20,
        hEditLabelsPopup, -1, hInstance, NULL);

    hLabelMass = CreateWindow(L"Static", NULL,
        SS_CENTER | WS_CHILD | WS_VISIBLE, 60, 10, 50, 20,
        hEditLabelsPopup, -1, hInstance, NULL);

    hLabelEditBox = CreateWindow(L"Static", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 18, 38, 124, 24,
        hEditLabelsPopup, -1, hInstance, NULL);

    hLabelEditWnd = CreateWindow("SuperEdit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 20, 40, 120, 20,
        hEditLabelsPopup, WS_LABELEDIT, hInstance, NULL);

    hLabelCancel = CreateWindow(L"Button", "Cancel",
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 20, 90, 60, 24,
        hEditLabelsPopup, WS_LABELCANCEL, hInstance, NULL);

    hLabelOK = CreateWindow(L"Button", "OK",
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 90, 90, 40, 24,
        hEditLabelsPopup, WS_LABELOK, hInstance, NULL);
}

/*****************************************************************************/


static BOOL create_edit_popup(hInstance)
HANDLE hInstance;
{

    /*  Create the "Edit" popup window and fill it with controls.
     */

    hEditPopup = CreateWindow("EditClass", "Edit Points",
        WS_POPUP | WS_BORDER | WS_CAPTION, 30, 120, 100, 160,
        hEditSetWnd, NULL, hInstance, NULL);
    if (!hEditPopup) return (FALSE);

    hEditMassTxt = CreateWindow("Static", "Mass",
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 30, 10, 40, 20,
        hEditPopup, -1, hInstance, NULL);

    hEditMass = CreateWindow("Static", NULL,
        SS_LEFT | WS_CHILD | WS_VISIBLE, 30, 40, 40, 20,
        hEditPopup, -1, hInstance, NULL);

    hEditEditCancel = CreateWindow("Button", "Cancel",
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 20, 80, 60, 24,
        hEditPopup, WS_EDITEDITCANCEL, hInstance, NULL);

    hEditEditOK = CreateWindow("Button", "OK",
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 30, 110, 40, 24,
        hEditPopup, WS_EDITEDITOK, hInstance, NULL);
}

/*****************************************************************************/


BOOL create_edit_windows(hInstance)
HANDLE hInstance;
{

    /*  Create the "Edit Graph" child window and the "Edit Set" child window
        and fill them with controls.
     */

    hEditGraphWnd = CreateWindow("EditClass", "Edit Graph",
        WS_VISIBLE | WS_CHILD | WS_BORDER, 200, 10, PLOTX2, PLOTY2,
        hwnd, WS_EDIT_GRAPH, hInstance, NULL);

    hEditSetWnd = CreateWindow("EditClass", _T("Edit Set"),
        WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 10, 180, PLOTY2,
        hwnd, WS_EDIT_SET, hInstance, NULL);

    hEditFilenameTxt = CreateWindow("STATIC", _T("File"),
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 20, 20, 40, 20,
        hEditSetWnd, -1, hInstance, NULL);

    hEditFilenameBox = CreateWindow("STATIC", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 60, 18, 94, 24,
        hEditSetWnd, -1, hInstance, NULL);

    hEditFilenameWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 62, 20, 90, 20,
        hEditSetWnd, WS_EDITFILENAME, hInstance, NULL);

    hEditTitleTxt = CreateWindow("STATIC", _T("Title"),
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 20, 50, 40, 20,
        hEditSetWnd, -1, hInstance, NULL);

    hEditTitleBox = CreateWindow("STATIC", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 20, 78, 144, 24,
        hEditSetWnd, -1, hInstance, NULL);

    hEditTitleWnd = CreateWindow("Edit", NULL, ES_AUTOHSCROLL |
        ES_LEFT | WS_CHILD | WS_VISIBLE, 22, 80, 140, 20,
        hEditSetWnd, WS_EDITTITLE, hInstance, NULL);

    hEditTimeTxt = CreateWindow("STATIC", _T("Time Period"),
        SS_SIMPLE | WS_CHILD | WS_VISIBLE, 10, 170, 100, 20,
        hEditSetWnd, -1, hInstance, NULL);

    hEditTimeBox = CreateWindow("STATIC", NULL,
        SS_BLACKFRAME | WS_CHILD | WS_VISIBLE, 100, 168, 54, 24,
        hEditSetWnd, -1, hInstance, NULL);

    hEditTimeWnd = CreateWindow("Edit", NULL,
        ES_LEFT | WS_CHILD | WS_VISIBLE, 102, 170, 50, 20,
        hEditSetWnd, WS_EDITTIMEPERIOD, hInstance, NULL);

    hEditSymbols = CreateWindow("BUTTON", "Symbols",
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 20, 260, 65, 24,
        hEditSetWnd, WS_EDITSYMBOLS, hInstance, NULL);

    hEditLabels = CreateWindow("Button", _T("Labels"),
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 95, 260, 65, 24,
        hEditSetWnd, WS_EDITLABELS, hInstance, NULL);

    hEditRetrieve = CreateWindow("Button", _T("Retrieve"),
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 20, 220, 65, 24,
        hEditSetWnd, (HMENU)WS_EDITRETRIEVE, hInstance, NULL);

    hEditNext = CreateWindow("Button", _T("Next"),
        BS_PUSHBUTTON | WS_CHILD, 95, 220, 65, 24,
        hEditSetWnd, WS_EDITNEXT, hInstance, NULL);

    hEditPrint = CreateWindow("Button", _T("Print"),
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 20, 300, 65, 24,
        hEditSetWnd, WS_EDITPRINT, hInstance, NULL);

    hEditEdit = CreateWindow("Button", _T("Edit"),
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 95, 300, 65, 24,
        hEditSetWnd, WS_EDITEDIT, hInstance, NULL);

    hEditCancel = CreateWindow("Button", _T("Cancel"),
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 20, 340, 65, 24,
        hEditSetWnd, WS_EDITCANCEL, hInstance, NULL);

    hEditRedraw = CreateWindow("Button", _T("Redraw"),
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 95, 340, 65, 24,
        hEditSetWnd, WS_EDITREDRAW, hInstance, NULL);

    hEditPrintOptions = CreateWindow("Button", _T("Print Options"),
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 20, 380, 140, 24,
        hEditSetWnd, WS_EDITPRINTOPTIONS, hInstance, NULL);

    edit = FALSE;
    draw = FALSE;
    time_period = 200;
    SendMessage(hEditTimeWnd, WM_SETTEXT, 0, (DWORD)(LPSTR)"200");
    title[0] = '\0';
    landscape = FALSE;
    fullsize = TRUE;
    smaller = FALSE;
    smallest = FALSE;
    print_double = FALSE;
    plotnum = 0;
    print_size = 0;
    printer_override = FALSE;
    plotDC = NULL;
    choice = 2;

    return (TRUE);
}

/*****************************************************************************/

void plot_point(plotDC, x, y, sindex, cindex, set_pen_colour, ponex, poney)
HDC plotDC;
int x, y, sindex, cindex;
int set_pen_colour;             /* TRUE if pen colour needs to be set */
double ponex, poney;
{

    /* Draw a symbol for a point.  Choose the symbol and the colour based on the
       sindex and cindex values respectively.
     */
    
    POINT vertex[5];                    /* plot point coordinates */
    HBRUSH hBrush = NULL, hOldBrush = NULL;
    HPEN hPen = NULL, hOldPen = NULL;
    HRGN hRegion;
    int f3x, f3y, f4x, f4y;

    if (set_pen_colour) {
        hPen = CreatePen(PS_SOLID, 1, col[cindex % 7]);
        hOldPen = SelectObject(plotDC, hPen);
    }
    if (sindex % 12 >= 0 && sindex % 12 <= 4) {
        hBrush = CreateSolidBrush(col[cindex % 7]);
        hOldBrush = SelectObject(plotDC, hBrush);
    }
    f3x = 3 * ponex;
    f3y = 3 * poney;
    f4x = 4 * ponex;
    f4y = 4 * poney;

    switch (sindex % 12) {
    case 0:                                     /* filled rectangle */
        hRegion = CreateRectRgn(x - f3x, y - f3y, x + f3x, y + f3y);
        FillRgn(plotDC, hRegion, hBrush);
        DeleteObject(hRegion);
        break;
    case 1:                             /* filled rightsideup triangle */
        vertex[0].x = x; vertex[0].y = y - f4y;
        vertex[1].x = x + f4x; vertex[1].y = y + f4y;
        vertex[2].x = x - f4x; vertex[2].y = y + f4y;
        vertex[3].x = x; vertex[3].y = y - f4y;
        hRegion = CreatePolygonRgn(vertex, 4, WINDING);
        FillRgn(plotDC, hRegion, hBrush);
        DeleteObject(hRegion);
        break;
    case 2:                                    /* filled diamond */
        vertex[0].x = x; vertex[0].y = y - f4y;
        vertex[1].x = x + f4x; vertex[1].y = y;
        vertex[2].x = x; vertex[2].y = y + f4y;
        vertex[3].x = x - f4x; vertex[3].y = y;
        vertex[4].x = x; vertex[4].y = y - f4y;
        hRegion = CreatePolygonRgn(vertex, 5, WINDING);
        FillRgn(plotDC, hRegion, hBrush);
        DeleteObject(hRegion);
        break;
    case 3:                                     /* filled circle */
        hRegion = CreateEllipticRgn(x - f3x, y - f3y, x + f3x, y + f3y);
        FillRgn(plotDC, hRegion, hBrush);
        DeleteObject(hRegion);
        break;
    case 4:                            /* filled upsidedown triangle */
        vertex[0].x = x - f4x; vertex[0].y = y - f4y;
        vertex[1].x = x + f4x; vertex[1].y = y - f4y;
        vertex[2].x = x; vertex[2].y = y + f4y;
        vertex[3].x = x - f4x; vertex[3].y = y - f4y;
        hRegion = CreatePolygonRgn(vertex, 4, WINDING);
        FillRgn(plotDC, hRegion, hBrush);
        DeleteObject(hRegion);
        break;
    case 5:
        MoveToEx(plotDC, x - f3x, y + f3y, NULL);                /* x */
        LineTo(plotDC, x + f3x, y - f3y);
        MoveToEx(plotDC, x + f3x, y + f3y, NULL);
        LineTo(plotDC, x - f3x, y - f3y);
        break;
    case 6:                                     /* circle */
        Ellipse(plotDC, x - f3x, y - f3y, x + f3x, y + f3y);
        break;
    case 7:                                    /* cross */
        MoveToEx(plotDC, x - f3x, y, NULL);
        LineTo(plotDC, x + f3x, y);
        MoveToEx(plotDC, x, y - f3y, NULL);
        LineTo(plotDC, x, y + f3y);
        break;
    case 8:                                     /* square */
        Rectangle(plotDC, x - f3x, y - f3y, x + f3x, y + f3y);
        break;
    case 9:                                     /* upright triangle */
        MoveToEx(plotDC, x - f3x, y + f3y, NULL);
        LineTo(plotDC, x, y - f3y);
        LineTo(plotDC, x + f3x, y + f3y);
        LineTo(plotDC, x - f3x, y + f3y);
        break;
    case 10:                                     /* diamond */
        MoveToEx(plotDC, x, y - f3y, NULL);
        LineTo(plotDC, x + f3x, y);
        LineTo(plotDC, x, y + f3y);
        LineTo(plotDC, x - f3x, y);
        LineTo(plotDC, x, y - f3y);
        break;
    case 11:                                     /* upsidedown triangle */
        MoveToEx(plotDC, x - f3x, y - f3y, NULL);
        LineTo(plotDC, x + f3x, y - f3y);
        LineTo(plotDC, x, y + f3y);
        LineTo(plotDC, x - f3x, y - f3y);
        break;
    default:
        break;
    }
    if (sindex % 12 >= 0 && sindex % 12 <= 4) {
        SelectObject(plotDC, hOldBrush);
        DeleteObject(hBrush);
    }
    if (set_pen_colour) {
        SelectObject(plotDC, hOldPen);
        DeleteObject(hPen);
    }
}


/*****************************************************************************/

static void draw_output_choices(outputDC)
HDC outputDC;
{

    /* Draw the symbol shape and colour choices in the output window. */

    int i, j;
    RECT frame;
    HBRUSH hBrush;
    float tonex, toney;

    tonex = toney = 2.0;
    for (i = 0; i < MAXSYMBOLS / 2; i++) {
        plot_point(outputDC, 100 + 50 * i, 50, i, 0, FALSE, tonex, toney);
    }
    for (i = MAXSYMBOLS / 2, j = 0; i < MAXSYMBOLS; i++, j++) {
        plot_point(outputDC, 100 + 50 * j, 80, i, 0, FALSE, tonex, toney);
    }

    for (i = 0; i < MAXCOLOURS / 2; i++) {
        frame.left = 72 + 72 * i;
        frame.top = 98;
        frame.right = 72 + 72 * i + 68;
        frame.bottom = 122;
        hBrush = CreateSolidBrush(col[i]);
        FrameRect(outputDC, &frame, hBrush);
        DeleteObject(hBrush);
    }
    for (i = MAXCOLOURS / 2, j = 0; i < MAXCOLOURS; i++, j++) {
        frame.left = 72 + 72 * j;
        frame.top = 128;
        frame.right = 72 + 72 * j + 68;
        frame.bottom = 152;
        hBrush = CreateSolidBrush(col[i]);
        FrameRect(outputDC, &frame, hBrush);
        DeleteObject(hBrush);
    }
}

/*****************************************************************************/

static void reduce_E_string(str)
TCHAR* str;                      /* string containing data in E format */
{

    /* Remove all extra zeros from a E format string. */

    int i, j;

    for (i = 0; str[i + 1] != '\0'; )
        if (str[i] == '0') {
            for (j = i + 1; str[j] != '\0'; j++)
                str[j - 1] = str[j];
            str[j - 1] = '\0';
            if (str[i - 1] == '.') {
                i--;
                for (j = i + 1; str[j] != '\0'; j++)
                    str[j - 1] = str[j];
                str[j - 1] = '\0';
            }
            if (str[i] != '0') i++;
        }
        else i++;
}

/*****************************************************************************/

static float
return_counts(locy)
int locy;                   /* y coordinate of user positioned line */
{

    /* Given a y coordinate, return the current counts value. */

    float counts;

    counts = pow(10, start_yvalue - ((locy - originy) / y_inc));
    return (counts);
}

/*****************************************************************************/

static BOOL colour_point(x, y)
int x, y;                        /* mouse cursor coordinates */
{

    /* Locate the point with the x and y values closest to the cursor x and y
       values, and redraw the point red.
     */

    int j, locy, locx, found;
    HDC plotDC;
    int red = 1;                    /* index of red colour in col array */

    if ((points = (struct data far*) GlobalLock(Block[curr_mass])) == NULL) {
        sprintf_s(string, 128,"colour_point: GlobalLock %d failed", curr_mass);
        MessageBox(NULL, string, "Error!", MB_OK);
        return (FALSE);
    }
    for (j = 0, found = FALSE; j < num_cycles; j++) {
        locx = originx + points[j].time * x_inc;
        if (points[j].counts >= pow(10, start_yvalue))
            locy = originy - (log10(points[j].counts) - start_yvalue) * y_inc;
        else locy = 0;
        if (locy < y + 0.02 * y && locy > y - 0.02 * y &&
            locx > x - x * 0.02 && locx < x + x * 0.02) {
            found = TRUE;
            break;
        }
    }
    if (found) {
        redraw_index = j;
        redraw_x = locx;
        plotDC = GetDC(hEditGraphWnd);
        plot_point(plotDC, redraw_x, locy, curr_mass, red, TRUE, ponex, poney);
        ReleaseDC(hEditGraphWnd, plotDC);
    }
    GlobalUnlock(Block[curr_mass]);
    if (found) return (TRUE);
    else return (FALSE);
}

/*****************************************************************************/

static void redraw_point(y)
int y;                      /* mouse cursor y coordinate */
{

    /* Having located the point with the y value closest to the cursor y value,
       redraw it in the new cursor y position.
     */

    HDC plotDC;
    int green = 2;

    if ((points = (struct data far*) GlobalLock(Block[curr_mass]))
        == NULL) {
        sprintf_s(string,128, "redraw_point: GlobalLock %d failed", curr_mass);
        MessageBox(NULL, string, "Error!", MB_OK);
        return (FALSE);
    }
    points[redraw_index].counts = return_counts(y);
    plotDC = GetDC(hEditGraphWnd);
    plot_point(plotDC, redraw_x, y, curr_mass, green, TRUE, ponex, poney);
    ReleaseDC(hEditGraphWnd, plotDC);
    GlobalUnlock(Block[curr_mass]);
}

/*****************************************************************************/

static int parse_label(plotDC, label, font)
HDC plotDC;                     /* device context */
TCHAR* label;                    /* label or title string */
HFONT font;                     /* font for non sub or superscripted chars */
{

    /* Go through the label string, adding the widths of all characters in their
       proper font, thus calculating the length of the string so it can be
       centred.
     */

    int i, width;
    HFONT hOldFont;
    SIZE size;

    hOldFont = SelectObject(plotDC, font);
    for (i = 0, width = 0; label[i] != '\0'; i++)
        if (label[i] == '^' || label[i] == '_')
            SelectObject(plotDC, hSScriptFont);
        else if (label[i] == '>')
            SelectObject(plotDC, font);
        else { 
            GetTextExtentPoint32(plotDC, &label[i], 1, &size); 
            width += size.cx;
        }
    return (width);
}

/*****************************************************************************/

static void draw_labels(plotDC, x, y, label, font)
HDC plotDC;                     /* device context */
int x, y;                        /* label start location */
TCHAR* label;                    /* label or title string */
HFONT font;                     /* font for non sub or superscripted chars */
{

    /* Draw the mass label beside the point at the edge of the plot.  Parse
       the string and write superscripts and subscripts as required.
     */

    int i, locx, width = 0, offset = 0;
    HFONT hOldFont;
    SIZE size;

    SetTextAlign(plotDC, TA_TOP | TA_LEFT);
    hOldFont = SelectObject(plotDC, font);
    locx = x;
    for (i = 0; i < strlen(label); i++) {
        if (label[i] == '^') {
            SelectObject(plotDC, hSScriptFont);
            offset += oney * -3;
        }
        else if (label[i] == '_') {
            SelectObject(plotDC, hSScriptFont);
            offset += oney * 4;
        }
        else if (label[i] == '>') {
            SelectObject(plotDC, font);
            if (offset < 0) offset += oney * 3;
            else offset += oney * -4;
        }
        else {
            TextOut(plotDC, locx, y + offset, &label[i], 1);
            GetTextExtentPoint(plotDC, &label[i], 1, &size);
            width += size.cx;
            locx += width;
        }
    }
    SelectObject(plotDC, hOldFont);
}

/*****************************************************************************/

static void draw_plot(plotDC, xsize, ysize)
HDC plotDC;
int xsize;                              /* Custom width if not NULL. */
int ysize;                              /* Custom height if not NULL. */
{

    /* Draw the axes for the dp plot, and fill in all the default labels. */

    HFONT hLabelFont, hOldFont;
    HFONT hYAxisFont, hTitleFont;
    HPEN hDottedPen, hOldPen = NULL, hColouredPen = NULL;
    float x_plot_range;             /* x axis range to be plotted over */
    float y_plot_range;             /* y axis range to be plotted over */
    float x_range, y_range;          /* value ranges to plot over */
    float step_xvalue, temp;
    int i, j, num_lines, locx, locy, xtick, num_xticks;
    int lsize, yaxsize, ssize1, ssize2, tsize;
    int endx, endy, intervals, width = 0, ox, oy;
    int lastx, lasty;
    long oldcolour;

    lsize = screensize;
    yaxsize = screensize * 0.9;
    ssize1 = screensize * 0.8;
    tsize = lsize * 1.5;
    originy = ORIGINY;
    endy = ENDY;
    originx = ORIGINX;
    endx = ENDX;
    onex = 1.0;
    oney = 1.0;
    ponex = onex;
    poney = oney;

    if (xsize) {
        lsize = printersize;
        yaxsize = printersize * 0.9;
        ssize1 = printersize * 0.8;
        tsize = lsize * 1.2;
        if (smallest && plotnum % 2 == 0) {
            originx += 2;
            endx += 2;
        }
        if (print_double && (smaller || smallest) && plotnum % 2 == 1) {
            originx = S2ORIGINX;
            endx = S2ENDX;
        }
        temp = xsize;
        temp = temp / PLOTX2;
        originx = originx * temp;
        endx = endx * temp;
        onex = temp;
        ponex = onex * 0.5;
        temp = ysize;
        temp = temp / PLOTY2;
        originy = originy * temp;
        endy = endy * temp;
        oney = temp;
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
    TextOut(plotDC, locx, locy, "INTENSITY (cps)", 15);
    SelectObject(plotDC, hLabelFont);
    DeleteObject(hYAxisFont);

    MoveToEx(plotDC, originx, originy, NULL);         /* X and Y axes */
    LineTo(plotDC, endx, originy);
    LineTo(plotDC, endx, endy);
    LineTo(plotDC, originx, endy);
    LineTo(plotDC, originx, originy);

    start_yvalue = 0;
    end_yvalue = 7;
    y_range = end_yvalue - start_yvalue;
    y_plot_range = originy - endy;
    y_inc = y_plot_range / y_range;
    SetTextAlign(plotDC, TA_TOP | TA_RIGHT);
    for (i = start_yvalue; i <= end_yvalue; i++) {
        locx = originx - 1 * onex;
        locy = originy - (i - start_yvalue) * y_inc - 5 * oney;
        sprintf_s(string, 128,"%d", i);
        SelectObject(plotDC, hSScriptFont);
        TextOut(plotDC, locx, locy, string, strlen(string));
        locy += 2 * oney;
        SIZE size;
        GetTextExtentPoint32(plotDC, string, strlen(string), &size);
        width += size.cx;
        SelectObject(plotDC, hLabelFont);
        StringCchCopy(string, 128, TEXT("10"));
        TextOut(plotDC, locx - width, locy, string, 2);
    }
    SetTextAlign(plotDC, TA_TOP | TA_LEFT);
    if (xsize) {
        for (i = start_yvalue + 1; i < end_yvalue; i++) {       /* Y axis lines */
            hOldPen = SelectObject(plotDC, hDottedPen);
            locy = originy - (i - start_yvalue) * y_inc;
            MoveToEx(plotDC, originx, locy, NULL);
            LineTo(plotDC, endx, locy);
            SelectObject(plotDC, hOldPen);
            MoveToEx(plotDC, originx, locy, NULL);
            locx = originx + 3 * onex;
            LineTo(plotDC, locx, locy);
            MoveToEx(plotDC, endx, locy, NULL);
            locx = endx - 3 * onex;
            LineTo(plotDC, locx, locy);
        }
        for (i = start_yvalue; i < end_yvalue; i++) {         /* Y axis ticks */
            for (j = 2; j <= 9; j++) {
                locy = originy - (log10(j * pow(10, (i - start_yvalue)))) * y_inc;
                MoveToEx(plotDC, originx, locy, NULL);
                locx = originx + 2 * onex;
                LineTo(plotDC, locx, locy);
            }
        }
    }
    DeleteObject(hSScriptFont);

    x_range = time_period;                          /* X axis ticks */
    x_plot_range = endx - originx;
    x_inc = x_plot_range / x_range;
    step_xvalue = time_period / 5.0;
    intervals = 4;
    num_lines = x_range / step_xvalue + 0.5;            /* force rounding */
    num_xticks = num_lines * intervals;
    xtick = step_xvalue / intervals;

    SelectObject(plotDC, hLabelFont);
    SetTextAlign(plotDC, TA_TOP | TA_CENTER);
    locx = originx;
    locy = originy + 2 * oney;
    TextOut(plotDC, locx, locy, "0", 1);
    sprintf_s(string,128, "%4.0f", x_range);
    TextOut(plotDC, endx, locy, string, strlen(string));
    for (i = 1; i < num_xticks; i++) {                    /* ticks, lines and */
        locx = originx + i * xtick * x_inc;             /* middle labels */
        if (!(i % intervals)) {
            if (xsize) {
                hOldPen = SelectObject(plotDC, hDottedPen);
                MoveToEx(plotDC, locx, originy, NULL);
                LineTo(plotDC, locx, endy);
                SelectObject(plotDC, hOldPen);
            }
            MoveToEx(plotDC, locx, originy, NULL);
            locy = originy - 3 * oney;
            LineTo(plotDC, locx, locy);
            sprintf_s(string,128, "%4.0f", step_xvalue * (i / intervals));
            locy = originy + 2 * oney;
            TextOut(plotDC, locx, locy, string, strlen(string));
        }
        else if (xsize) {
            MoveToEx(plotDC, locx, originy, NULL);
            locy = originy - 2 * oney;
            LineTo(plotDC, locx, locy);
        }
        if (xsize) {
            MoveToEx(plotDC, locx, endy, NULL);
            if (i % intervals) {
                locy = endy + 3 * oney;
                LineTo(plotDC, locx, locy);
            }
            else {
                locy = endy + 2 * oney;
                LineTo(plotDC, locx, locy);
            }
        }
    }
    DeleteObject(hDottedPen);

    SelectObject(plotDC, hLabelFont);
    SetTextAlign(plotDC, TA_TOP | TA_CENTER);
    locx = originx + ((endx - originx) / 2);
    locy = originy + 10 * oney;
    TextOut(plotDC, locx, locy, "TIME (seconds)", 14);
    StringCchCopy(string,128, filename);
    for (i = 0; i < strlen(string) && string[i] != '.'; i++);
    string[i] = '\0';
    SelectObject(plotDC, hTitleFont);
    locy = endy - 22 * oney;
    TextOut(plotDC, locx, locy, string, strlen(string));
    width = parse_label(plotDC, title, hTitleFont);
    locx = (originx + (endx - originx) / 2) - (width / 2);
    locy = endy - 10 * oney;
    draw_labels(plotDC, locx, locy, title, hTitleFont);
    if (print_double && !xsize) {
        SetTextAlign(plotDC, TA_TOP | TA_RIGHT);
        locy = originy + 10 * oney;
        sprintf_s(string,128, "%d printed", plotnum % 2);
        oldcolour = SetTextColor(plotDC, (LONG)RED);
        TextOut(plotDC, endx, locy, string, strlen(string));
        SetTextColor(plotDC, oldcolour);
    }
    SelectObject(plotDC, hOldFont);
    DeleteObject(hTitleFont);
    SetTextAlign(plotDC, TA_TOP | TA_LEFT);

    ox = 3 * onex;
    oy = 2 * oney;
    for (i = 0; i < num_masses; i++) {
        if (!edit || (edit && i == curr_mass)) {
            if ((points = (struct data far*) GlobalLock(Block[i])) == NULL) {
                sprintf_s(string,128, "GlobalLock %d failed", i);
                MessageBox(NULL, string, "draw_plot", MB_OK);
            }
            else {
                if (!edit) {
                    hColouredPen = CreatePen(PS_SOLID, 1, col[colour[i]]);
                    hOldPen = SelectObject(plotDC, hColouredPen);
                }
                for (j = 0; j < num_cycles && points[j].time < time_period; j++) {
                    locx = originx + points[j].time * x_inc;
                    if (points[j].counts >= pow(10, start_yvalue))
                        locy = originy -
                        (log10(points[j].counts) - start_yvalue) * y_inc;
                    else locy = -1;
                    if (locy != -1) {
                        if (edit)
                            plot_point(plotDC, locx, locy, i, 0, FALSE, ponex, poney);
                        else plot_point(plotDC, locx, locy, symbol[i], colour[i],
                            FALSE, ponex, poney);
                        if (j > 0 && lasty != -1) {
                            MoveToEx(plotDC, lastx, lasty, NULL);
                            LineTo(plotDC, locx, locy);
                        }
                    }
                    lastx = locx;
                    lasty = locy;
                    if (!edit && j == num_cycles - 1) {
                        if (locy == -1 || locy >= originy) locy = originy - 2 * oney;
                        draw_labels(plotDC, locx + ox, locy - oy, labels[i],
                            hLabelFont);
                    }
                }
                locx = endx + 4 * onex;
                locy = endy + 10 * (i + 1) * oney;
                if (edit) plot_point(plotDC, locx, locy, i, 0, FALSE, ponex, poney);
                else plot_point(plotDC, locx, locy, symbol[i], colour[i], FALSE,
                    ponex, poney);
                draw_labels(plotDC, locx + ox, locy - oy, labels[i], hLabelFont);
                GlobalUnlock(Block[i]);
                if (!edit) {
                    SelectObject(plotDC, hOldPen);
                    DeleteObject(hColouredPen);
                }
            }
        }
    }
    DeleteObject(hLabelFont);
}
/*****************************************************************************/
int is_filename_array_empty() {
    // Loop through each element in the array
    for (int i = 0; i < 120; i++) {
        // Check if the current element is not NULL and not an empty string
        if (filename_array[i] != NULL && filename_array[i][0] != '\0') {
            return 0; // Not empty
        }
    }
    return 1; // Empty
}
/*****************************************************************************/

static void print_plot()
{

    POINT PageSize;
    int xsize, ysize;

    if (fullsize || !print_double ||
        (print_double && (smaller || smallest) && plotnum % 2 == 0)) {
        plotDC = print_file(hPlotWnd, landscape);
        if (plotDC == NULL) {
            MessageBox(NULL, "print_plot: Cannot Print", "Error", MB_OK);
            return (FALSE);
        }
        Escape(plotDC, STARTDOC, 8, (LPSTR)"Dp Print", 0L);
    }
    Escape(plotDC, GETPHYSPAGESIZE, NULL, NULL, (LPSTR)&PageSize);
    xsize = PageSize.x * 0.98;
    ysize = PageSize.y;
    if (printer_override) printersize = print_size;
    else {
        printersize = 0.018 * PageSize.x;
        print_size = printersize;
    }
    if (smaller && landscape) {
        xsize = PageSize.x * 0.49;
        ysize = PageSize.y * 0.9;
        printersize *= 0.67;
    }
    else if (smaller) {
        xsize *= 0.65;
        ysize *= 0.70;
        printersize *= 0.67;
    }
    else if (smallest) {
        xsize *= 0.5;
        ysize *= 0.5;
        printersize *= 0.5;
    }
    if (print_double && (smaller || smallest))
        shift = (S2ORIGINX - ORIGINX) * (plotnum % 2);

    draw_plot(plotDC, xsize, ysize);

    SetTextAlign(plotDC, TA_TOP | TA_LEFT);
    if (fullsize || !print_double ||
        (print_double && (smaller || smallest) && plotnum % 2 == 1)) {
        Escape(plotDC, NEWFRAME, 0, 0L, 0L);
        Escape(plotDC, ENDDOC, 0, 0L, 0L);
        DeleteDC(plotDC);
        plotDC = NULL;
        plotnum = 0;
    }
    else plotnum++;
}

/******************************************************************************/

LRESULT CALLBACK EditProc(hWnd, message, wParam, lParam)
HWND hWnd;
UINT message;
WPARAM wParam;
LPARAM lParam;
{

    /* Processes messages received by the Edit child window.
     */

    int i, err;
    TCHAR ext[5];
    FILE* fn;
    HDC editDC, outputDC;
    PAINTSTRUCT editPaint, outputPaint;
    HCURSOR hHourGlass, hSaveCursor;

    switch (message) {

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case WS_EDITTITLE:
            if (HIWORD(wParam) == EN_CHANGE) {
                SendMessage(hEditTitleWnd, WM_GETTEXT, 120,
                    (DWORD)(LPSTR)title);
            }
            break;

        case WS_EDITRETRIEVE:
            if (HIWORD(wParam) == BN_CLICKED) {
                StringCchCopy(ext, 5, _T("dat"));
                select_file1(hEditSetWnd, ext, filename, _T("Choose Data Files"));
            }
            break;

        case DB_MODELESS2:
            if (LOWORD(lParam)) {
                /*if ((filenames = (struct fname_record*) GlobalLock(Files)) == NULL) {
                    MessageBox(NULL, _T("filenames GlobalLock failed"), _T("Error"), MB_OK);
                    break;
                }
                StringCchCopy(filename,128, filenames[0].fname);
                GlobalUnlock(Files);*/
                if (!is_filename_array_empty()) {
                    StringCchCopy(filename, 128, filename_array[0]);
                }
                ShowWindow(hEditNext, SW_SHOW);
                SendMessage(hEditFilenameWnd, WM_SETTEXT, 0,
                    (DWORD)(LPSTR)filename);
                return_extension(ext, filename);
                if (!strcmp(ext, "dat"))
                read_sims_binary(filename);
                 else read_sims_text(filename, FALSE);
                 /*
                                     remove_points (TRUE,filename);
                 */
                write_sims_text(filename, FALSE);
                batch_num = 0;
                draw = TRUE;
                InvalidateRect(hEditGraphWnd, 0L, TRUE);
            }
            break;

        case WS_EDITTIMEPERIOD:
            if (HIWORD(wParam) == EN_CHANGE) {
                SendMessage(hEditTimeWnd, WM_GETTEXT, 40,
                    (DWORD)(LPSTR)string);
                time_period = atoi(string);
            }
            break;

        case WS_EDITEDIT:
            if (HIWORD(wParam) == BN_CLICKED) {
                create_edit_popup(hInst);
                ShowWindow(hEditPopup, SW_SHOW);
                curr_mass = 0;
                sprintf_s(string,128, "%5.1f", mass[curr_mass]);
                SendMessage(hEditMass, WM_SETTEXT, 0, (DWORD)(LPSTR)string);
                edit = TRUE;
                draw = TRUE;
                InvalidateRect(hEditGraphWnd, 0L, TRUE);
            }
            break;

        case WS_EDITEDITOK:
            if (HIWORD(wParam) == BN_CLICKED) {
                curr_mass++;
                if (curr_mass == num_masses) {
                    write_sims_text(filename, FALSE);
                    DestroyWindow(hEditPopup);
                    edit = FALSE;
                }
                else {
                    sprintf_s(string,128, "%5.1f", mass[curr_mass]);
                    SendMessage(hEditMass, WM_SETTEXT, 0,
                        (DWORD)(LPSTR)string);
                }
                InvalidateRect(hEditGraphWnd, 0L, TRUE);
            }
            break;

        case WS_EDITEDITCANCEL:
            if (HIWORD(wParam) == BN_CLICKED) {
                DestroyWindow(hEditPopup);
                edit = FALSE;
                InvalidateRect(hEditGraphWnd, 0L, TRUE);
            }
            break;

        case WS_EDITSYMBOLS:
            if (HIWORD(wParam) == BN_CLICKED) {
                create_symbol_window(hInst);
            }
            break;

        case WS_OUTPUTSYM0: case WS_OUTPUTSYM0 + 1: case WS_OUTPUTSYM0 + 2:
        case WS_OUTPUTSYM0 + 3: case WS_OUTPUTSYM0 + 4: case WS_OUTPUTSYM0 + 5:
        case WS_OUTPUTSYM0 + 6: case WS_OUTPUTSYM0 + 7: case WS_OUTPUTSYM0 + 8:
        case WS_OUTPUTSYM0 + 9: case WS_OUTPUTSYM0 + 10: case WS_OUTPUTSYM0 + 11:
            if (HIWORD(wParam) == BN_CLICKED) {
                for (i = 0; i < MAXSYMBOLS; i++)
                    if (i == wParam - WS_OUTPUTSYM0)
                        SendMessage(hOutputSym[i], BM_SETCHECK, 1, 0L);
                    else SendMessage(hOutputSym[i], BM_SETCHECK, 0, 0L);
                symbol[curr_mass] = wParam - WS_OUTPUTSYM0;
            }
            break;

        case WS_OUTPUTCOL0: case WS_OUTPUTCOL0 + 1: case WS_OUTPUTCOL0 + 2:
        case WS_OUTPUTCOL0 + 3: case WS_OUTPUTCOL0 + 4: case WS_OUTPUTCOL0 + 5:
        case WS_OUTPUTCOL0 + 6: case WS_OUTPUTCOL0 + 7:
            if (HIWORD(wParam) == BN_CLICKED) {
                for (i = 0; i < MAXCOLOURS; i++)
                    if (i == wParam - WS_OUTPUTCOL0)
                        SendMessage(hOutputCol[i], BM_SETCHECK, 1, 0L);
                    else SendMessage(hOutputCol[i], BM_SETCHECK, 0, 0L);
                colour[curr_mass] = wParam - WS_OUTPUTCOL0;
            }
            break;

        case WS_OUTPUTOK:
            if (HIWORD(wParam) == BN_CLICKED) {
                curr_mass++;
                if (curr_mass >= num_masses) {
                    DestroyWindow(hSymbolPopup);
                    hSymbolPopup = NULL;
                    InvalidateRect(hEditGraphWnd, 0L, TRUE);
                }
                else {
                    for (i = 0; i < MAXSYMBOLS; i++)
                        if (i == symbol[curr_mass])
                            SendMessage(hOutputSym[i], BM_SETCHECK, 1, 0L);
                        else SendMessage(hOutputSym[i], BM_SETCHECK, 0, 0L);
                    for (i = 0; i < MAXCOLOURS; i++)
                        if (i == colour[curr_mass])
                            SendMessage(hOutputCol[i], BM_SETCHECK, 1, 0L);
                        else SendMessage(hOutputCol[i], BM_SETCHECK, 0, 0L);
                    sprintf_s(string,128, "Mass %5.1f", mass[curr_mass]);
                    SendMessage(hOutputMassWnd, WM_SETTEXT, 0,
                        (DWORD)(LPSTR)string);
                }
            }
            break;

        case WS_EDITLABELS:
            if (HIWORD(wParam) == BN_CLICKED) {
                create_labels_window(hInst);
                ShowWindow(hEditLabelsPopup, SW_SHOW);
                curr_mass = 0;
                sprintf_s(string,128, "%5.1f", mass[curr_mass]);
                SendMessage(hLabelMass, WM_SETTEXT, 0, (DWORD)(LPSTR)string);
                SendMessage(hLabelEditWnd, WM_SETTEXT, 0,
                    (DWORD)(LPSTR)labels[curr_mass]);
                SetFocus(hLabelEditWnd);
                PostMessage(hLabelEditWnd, WM_KEYDOWN, VK_END, 0x014F0001);
                PostMessage(hLabelEditWnd, WM_KEYUP, VK_END, 0xC14F001);
            }
            break;

        case WS_LABELEDIT:
            if (HIWORD(wParam) == EN_CHANGE) {
                SendMessage(hLabelEditWnd, WM_GETTEXT, 15,
                    (DWORD)(LPSTR)labels[curr_mass]);
            }
            break;

        case WS_LABELOK:
            if (HIWORD(wParam) == BN_CLICKED) {
                curr_mass++;
                if (curr_mass >= num_masses) {
                    DestroyWindow(hEditLabelsPopup);
                }
                else {
                    sprintf_s(string,128, "%5.1f", mass[curr_mass]);
                    SendMessage(hLabelMass, WM_SETTEXT, 0,
                        (DWORD)(LPSTR)string);
                    SendMessage(hLabelEditWnd, WM_SETTEXT, 0,
                        (DWORD)(LPSTR)labels[curr_mass]);
                    SetFocus(hLabelEditWnd);
                    PostMessage(hLabelEditWnd, WM_KEYDOWN, VK_END, 0x014F0001);
                    PostMessage(hLabelEditWnd, WM_KEYUP, VK_END, 0xC14F001);
                }
            }
            break;

        case WS_LABELCANCEL:
            if (HIWORD(wParam) == BN_CLICKED) {
                DestroyWindow(hEditLabelsPopup);
            }
            break;

        case WS_EDITREDRAW:
            if (HIWORD(wParam) == BN_CLICKED) {
                InvalidateRect(hEditGraphWnd, 0L, TRUE);
            }
            break;

        case WS_EDITNEXT:
            if (HIWORD(wParam) == BN_CLICKED) {
                batch_num++;
                if (batch_num < total_fnames) {
                    if (IsWindow(hEditPopup)) DestroyWindow(hEditPopup);
                    /*if ((filenames = (struct fname_record*) GlobalLock(Files)) == NULL) {
                        MessageBox(NULL, "filenames GlobalLock failed", "Error", MB_OK);
                        break;
                    }*/
                    StringCchCopy(filename,128, filename_array[batch_num]);
                    SendMessage(hEditFilenameWnd, WM_SETTEXT, 0,
                        (DWORD)(LPSTR)filename);
                    GlobalUnlock(Files);
                    return_extension(ext, filename);
                    if (!strcmp(ext, "dat"))
                        read_sims_binary(filename);
                    else read_sims_text(filename, FALSE);
                    /*
                                            remove_points (TRUE,filename);
                    */
                    write_sims_text(filename, FALSE);
                    InvalidateRect(hEditGraphWnd, 0L, TRUE);
                }
                if (batch_num == total_fnames - 1)
                    ShowWindow(hEditNext, SW_HIDE);
            }
            break;

        case WS_EDITCANCEL:
            if (HIWORD(wParam) == BN_CLICKED) {
                if (IsWindow(hEditPopup)) DestroyWindow(hEditPopup);
                DestroyWindow(hEditSetWnd);
                DestroyWindow(hEditGraphWnd);
                hEditGraphWnd = NULL;
                ShowWindow(hRatiosWnd, SW_SHOW);
            }
            break;

        case WS_EDITPRINT:
            if (HIWORD(wParam) == BN_CLICKED) {
                print_plot();
                InvalidateRect(hEditGraphWnd, 0L, TRUE);
            }
            break;

        case WS_EDITPRINTOPTIONS:
            if (HIWORD(wParam) == BN_CLICKED) {
                create_print_opts_window(hInst);
                if (landscape) SendMessage(hLandscape, BM_SETCHECK, 1, 0L);
                else SendMessage(hLandscape, BM_SETCHECK, 0, 0L);
                if (print_double) SendMessage(hDouble, BM_SETCHECK, 1, 0L);
                else SendMessage(hDouble, BM_SETCHECK, 0, 0L);
                if (fullsize) {
                    SendMessage(hFullSize, BM_SETCHECK, 1, 0L);
                    SendMessage(hSmaller, BM_SETCHECK, 0, 0L);
                    SendMessage(hSmallest, BM_SETCHECK, 0, 0L);
                }
                else if (smaller) {
                    SendMessage(hFullSize, BM_SETCHECK, 0, 0L);
                    SendMessage(hSmaller, BM_SETCHECK, 1, 0L);
                    SendMessage(hSmallest, BM_SETCHECK, 0, 0L);
                }
                else if (smallest) {
                    SendMessage(hFullSize, BM_SETCHECK, 0, 0L);
                    SendMessage(hSmaller, BM_SETCHECK, 0, 0L);
                    SendMessage(hSmallest, BM_SETCHECK, 1, 0L);
                }
                if (print_size > 0) {
                    sprintf_s(string,128, "%d", print_size);
                    SendMessage(hPlotFontEditWnd, WM_SETTEXT, 0,
                        (DWORD)(LPSTR)string);
                }
            }
            break;

        case WS_PRINTLANDSCAPE:
            if (HIWORD(wParam) == BN_CLICKED) {
                if (plotDC != NULL) {
                    Escape(plotDC, NEWFRAME, 0, 0L, 0L);
                    Escape(plotDC, ENDDOC, 0, 0L, 0L);
                    DeleteDC(plotDC);
                    plotDC = NULL;
                    plotnum = 0;
                }
                if (landscape) {
                    landscape = FALSE;
                    SendMessage(hLandscape, BM_SETCHECK, 0, 0L);
                }
                else {
                    landscape = TRUE;
                    SendMessage(hLandscape, BM_SETCHECK, 1, 0L);
                }
            }
            break;

        case WS_PRINTDOUBLE:
            if (HIWORD(wParam) == BN_CLICKED) {
                if (plotDC != NULL) {
                    Escape(plotDC, NEWFRAME, 0, 0L, 0L);
                    Escape(plotDC, ENDDOC, 0, 0L, 0L);
                    DeleteDC(plotDC);
                    plotDC = NULL;
                    plotnum = 0;
                }
                if (print_double) {
                    print_double = FALSE;
                    SendMessage(hDouble, BM_SETCHECK, 0, 0L);
                }
                else {
                    print_double = TRUE;
                    SendMessage(hDouble, BM_SETCHECK, 1, 0L);
                }
            }
            break;

        case WS_PRINTFULLSIZE:
            if (HIWORD(wParam) == BN_CLICKED) {
                if (plotDC != NULL) {
                    Escape(plotDC, NEWFRAME, 0, 0L, 0L);
                    Escape(plotDC, ENDDOC, 0, 0L, 0L);
                    DeleteDC(plotDC);
                    plotDC = NULL;
                    plotnum = 0;
                }
                fullsize = TRUE;
                SendMessage(hFullSize, BM_SETCHECK, 1, 0L);
                smaller = FALSE;
                SendMessage(hSmaller, BM_SETCHECK, 0, 0L);
                smallest = FALSE;
                SendMessage(hSmallest, BM_SETCHECK, 0, 0L);
            }
            break;

        case WS_PRINTSMALLER:
            if (HIWORD(wParam) == BN_CLICKED) {
                if (plotDC != NULL) {
                    Escape(plotDC, NEWFRAME, 0, 0L, 0L);
                    Escape(plotDC, ENDDOC, 0, 0L, 0L);
                    DeleteDC(plotDC);
                    plotDC = NULL;
                    plotnum = 0;
                }
                fullsize = FALSE;
                SendMessage(hFullSize, BM_SETCHECK, 0, 0L);
                smaller = TRUE;
                SendMessage(hSmaller, BM_SETCHECK, 1, 0L);
                smallest = FALSE;
                SendMessage(hSmallest, BM_SETCHECK, 0, 0L);
            }
            break;

        case WS_PRINTSMALLEST:
            if (HIWORD(wParam) == BN_CLICKED) {
                if (plotDC != NULL) {
                    Escape(plotDC, NEWFRAME, 0, 0L, 0L);
                    Escape(plotDC, ENDDOC, 0, 0L, 0L);
                    DeleteDC(plotDC);
                    plotDC = NULL;
                    plotnum = 0;
                }
                fullsize = FALSE;
                SendMessage(hFullSize, BM_SETCHECK, 0, 0L);
                smaller = FALSE;
                SendMessage(hSmaller, BM_SETCHECK, 0, 0L);
                smallest = TRUE;
                SendMessage(hSmallest, BM_SETCHECK, 1, 0L);
            }
            break;

        case WS_PLOTFONTEDIT:
            if (HIWORD(wParam) == EN_CHANGE) {
                if (GetFocus() == hPlotFontEditWnd) {
                    SendMessage(hPlotFontEditWnd, WM_GETTEXT, 10,
                        (DWORD)(LPSTR)string);
                    print_size = atoi(string);
                    printer_override = TRUE;
                }
            }
            break;

        case WS_PRINTOPTIONSOK:
            if (HIWORD(wParam) == BN_CLICKED) {
                DestroyWindow(hPrintOptionsPopup);
            }
            break;

        }
        break;

    case WM_LBUTTONUP:
        if (edit == TRUE) {
            if (colour_point(LOWORD(lParam), HIWORD(lParam)))
                edit = ALTTRUE;
        }
        else if (edit == ALTTRUE) {
            redraw_point(HIWORD(lParam));
            edit = TRUE;
        }
        DefWindowProc(hWnd, message, wParam, lParam);
        break;

    case WM_PAINT:
        if (hWnd == hEditGraphWnd && draw) {
            editDC = BeginPaint(hEditGraphWnd, &editPaint);
            draw_plot(editDC, 0, 0);
            EndPaint(hEditGraphWnd, &editPaint);
        }
        else if (hWnd == hSymbolPopup) {
            outputDC = BeginPaint(hSymbolPopup, &outputPaint);
            draw_output_choices(outputDC);
            EndPaint(hSymbolPopup, &outputPaint);
        }
        else DefWindowProc(hWnd, message, wParam, lParam);
        break;

    default:
        return (DefWindowProc(hWnd, message, wParam, lParam));
    }

    return (NULL);
}

/*****************************************************************************/


