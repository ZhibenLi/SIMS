#define MAXMASSES       20
#define MAXCYCLES      200
#define MAXFILES       120
#define MAXSYMBOLS      12
#define MAXCOLOURS       7
#define MAXLABELLENGTH  15


#define ALTTRUE     -1

/* Pen Colours */

#define WHITE       0x00FFFFFF
#define BLACK       0x00000000
#define RED         0x000000FF
#define GREEN       0x0000FF00
#define YELLOW      0x0000FFFF
#define BLUE        0x00FF0000
#define PURPLE      0x00FF00FF
#define AQUA        0x00FFFF00
#define ORANGE      0x00007FFF

/* Definitions required for program choices. */

#define NOPROGRAM           0
#define BATCH_RATIOS        1

/* Definitions required for windows and menus. */



/* child window ID's */

#define     WS_RATIOS_SET        1000
#define     WS_RATIOS_GRAPH      1001
#define     WS_EDIT_GRAPH        1002
#define     WS_EDIT_SET          1003

/* control ID's for ratios */

#define     WS_RATIOSMASSES0        1500

#define     WS_RATIOSFILENAME       1600
#define     WS_RATIOSFNAME          1601
#define     WS_RATIOSMATRIX         1602
#define     WS_RATIOSSELEMENT       1603
#define     WS_RATIOSDEPTH          1604
#define     WS_RATIOSALPHA          1605
#define     WS_RATIOSBGRATIO        1606
#define     WS_RATIOSBGPPM          1607
#define     WS_RATIOSMAXRATIO       1608
#define     WS_RATIOSMAXPPM         1609
#define     WS_RATIOSSLOPE          1610
#define     WS_RATIOSINTERCEPT      1611
#define     WS_RATIOSRETRIEVE       1616
#define     WS_RATIOSSTANDARD       1617
#define     WS_RATIOSBATCH          1618
#define     WS_RATIOSCANCEL         1619
#define     WS_RATIOSOK             1620
#define     WS_RATIOSPOPUPPRINT     1621
#define     WS_RATIOSPOPUPCANCEL    1622
#define     WS_RATIOSSELABEL        1623
#define     WS_RATIOSMLABEL         1624
#define     WS_RATIOSSISOABUN       1625
#define     WS_RATIOSPRINT          1626
#define     WS_HIGHYEDIT            1627
#define     WS_LOWYEDIT             1628
#define     WS_HIGHXEDIT            1629
#define     WS_LOWXEDIT             1630
#define     WS_RATIOSPOPUPOK        1631
#define     WS_RATIOSRELEMENT       1632
#define     WS_RATIOSRISOABUN       1633
#define     WS_RATIOSRELABEL        1634
#define     WS_RATIOLABELEDIT       1635
#define     WS_RATIOSSAVE           1636

/* control ID's for edit */

#define     WS_EDITNEXT             1700
#define     WS_EDITCANCEL           1701
#define     WS_EDITREDRAW           1702
#define     WS_EDITPRINT            1703
#define     WS_EDITFILENAME         1704
#define     WS_EDITEDIT             1705
#define     WS_EDITEDITOK           1706
#define     WS_EDITEDITCANCEL       1707
#define     WS_EDITRETRIEVE         1708
#define     WS_EDITTIMEPERIOD       1709
#define     WS_EDITPRINTOPTIONS     1710
#define     WS_EDITTITLE            1711
#define     WS_EDITSYMBOLS          1712
#define     WS_EDITLABELS           1713
#define     WS_OUTPUTOK             1714
#define     WS_LABELEDIT            1715
#define     WS_LABELCANCEL          1716
#define     WS_LABELOK              1717

#define     WS_PRINTFULLSIZE        1720
#define     WS_PRINTSMALLER         1721
#define     WS_PRINTSMALLEST        1722
#define     WS_PRINTDOUBLE          1723
#define     WS_PRINTOPTIONSOK       1724
#define     WS_PLOTFONTEDIT         1725
#define     WS_PRINTLANDSCAPE       1726

#define     WS_OUTPUTCOL0           1800
#define     WS_OUTPUTSYM0           1820

/* functions */
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK About(HWND, UINT, WPARAM, LPARAM);
BOOL create_editsuperclass(HANDLE);
LRESULT CALLBACK  SuperClassEditProc(HWND, UINT, WPARAM, LPARAM);
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
