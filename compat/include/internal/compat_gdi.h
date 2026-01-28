#pragma once

DECLARE_HANDLE(HGDIOBJ);

#define LF_FACESIZE 32

#define MONITOR_DEFAULTTONULL       0x00000000
#define MONITOR_DEFAULTTOPRIMARY    0x00000001
#define MONITOR_DEFAULTTONEAREST    0x00000002

#define MONITORINFOF_PRIMARY        0x00000001

#define CCHDEVICENAME 32
#define CCHFORMNAME   32

typedef struct tagMONITORINFO
{
  DWORD cbSize;
  RECT rcMonitor;
  RECT rcWork;
  DWORD dwFlags;
} MONITORINFO, *LPMONITORINFO;

typedef struct tagMONITORINFOEXA
{ /* the 4 first entries are the same as MONITORINFO */
  DWORD cbSize;
  RECT rcMonitor;
  RECT rcWork;
  DWORD dwFlags;
  CHAR szDevice[CCHDEVICENAME];
} MONITORINFOEXA, *LPMONITORINFOEXA;

typedef struct tagMONITORINFOEXW
{ /* the 4 first entries are the same as MONITORINFO */
  DWORD cbSize;
  RECT rcMonitor;
  RECT rcWork;
  DWORD dwFlags;
  WCHAR szDevice[CCHDEVICENAME];
} MONITORINFOEXW, *LPMONITORINFOEXW;

typedef struct
{
  INT bmType;
  INT bmWidth;
  INT bmHeight;
  INT bmWidthBytes;
  WORD bmPlanes;
  WORD bmBitsPixel;
  LPVOID bmBits;
} BITMAP, *PBITMAP, *LPBITMAP;

HBITMAP LoadImageA(
  HINSTANCE hinst,
  LPCSTR    lpszName,
  UINT      uType,
  INT       cxDesired,
  INT       cyDesired,
  UINT      fuLoad
);

#define IMAGE_BITMAP 0
#define LR_LOADFROMFILE 0x00000010
#define LR_CREATEDIBSECTION 0x00002000

BOOL BitBlt(
  HDC   hdcDest,
  INT   xDest,
  INT   yDest,
  INT   wDest,
  INT   hDest,
  HDC   hdcSrc,
  INT   xSrc,
  INT   ySrc,
  DWORD rop
);

#define SRCCOPY         0xcc0020

HDC CreateCompatibleDC(HDC hdc);
BOOL DeleteDC(HDC hdc);
int GetObjectA(HGDIOBJ h, int c, LPVOID pv);
HGDIOBJ SelectObject(HDC hdc, HGDIOBJ hgdiobj);
BOOL DeleteObject(HGDIOBJ hObject);

HBITMAP CreateBitmap(
  INT         nWidth,
  INT         nHeight,
  UINT        cPlanes,
  UINT        cBitsPerPel,
  const VOID  *lpvBits
);

HICON CreateIconIndirect(const ICONINFO *piconinfo);

typedef struct
{
  DWORD cb;
  WCHAR DeviceName[32];
  WCHAR DeviceString[128];
  DWORD StateFlags;
  WCHAR DeviceID[128];
  WCHAR DeviceKey[128];
} DISPLAY_DEVICEW, *PDISPLAY_DEVICEW, *LPDISPLAY_DEVICEW;

#define	DISPLAY_DEVICE_PRIMARY_DEVICE		0x00000004
#define DISPLAY_DEVICE_ATTACHED_TO_DESKTOP  0x00000001

#define DISP_CHANGE_SUCCESSFUL 0

#define DM_POSITION             0x00000020
#define DM_DISPLAYORIENTATION   0x00000080
#define DM_BITSPERPEL           0x00040000
#define DM_PELSWIDTH            0x00080000
#define DM_PELSHEIGHT           0x00100000
#define DM_DISPLAYFLAGS         0x00200000
#define DM_DISPLAYFREQUENCY     0x00400000

#define DM_INTERLACED           2

typedef struct
{
    WCHAR  dmDeviceName[CCHDEVICENAME];
    WORD   dmSpecVersion;
    WORD   dmDriverVersion;
    WORD   dmSize;
    WORD   dmDriverExtra;
    DWORD  dmFields;
    union {
      struct {
	short  dmOrientation;
	short  dmPaperSize;
	short  dmPaperLength;
	short  dmPaperWidth;
        short  dmScale;
        short  dmCopies;
        short  dmDefaultSource;
        short  dmPrintQuality;
      } DUMMYSTRUCTNAME1;
      struct {
        POINTL dmPosition;
        DWORD dmDisplayOrientation;
        DWORD dmDisplayFixedOutput;
      } DUMMYSTRUCTNAME2;
    } DUMMYUNIONNAME1;
    short  dmColor;
    short  dmDuplex;
    short  dmYResolution;
    short  dmTTOption;
    short  dmCollate;
    WCHAR  dmFormName[CCHFORMNAME];
    WORD   dmLogPixels;
    DWORD  dmBitsPerPel;
    DWORD  dmPelsWidth;
    DWORD  dmPelsHeight;
    union {
      DWORD dmDisplayFlags;
      DWORD dmNup;
    } DUMMYUNIONNAME2;
    DWORD  dmDisplayFrequency;
    DWORD  dmICMMethod;
    DWORD  dmICMIntent;
    DWORD  dmMediaType;
    DWORD  dmDitherType;
    DWORD  dmReserved1;
    DWORD  dmReserved2;
    DWORD  dmPanningWidth;
    DWORD  dmPanningHeight;
} DEVMODEW, *PDEVMODEW, *LPDEVMODEW;

BOOL EnumDisplayDevicesW(
  LPCWSTR          lpDevice,
  DWORD            iDevNum,
  PDISPLAY_DEVICEW lpDisplayDevice,
  DWORD            dwFlags
);

BOOL EnumDisplaySettingsW(
  LPCWSTR        lpszDeviceName,
  DWORD          iModeNum,
  LPDEVMODEW    lpDevMode
);

BOOL EnumDisplaySettingsExW(
  LPCWSTR         lpszDeviceName,
  DWORD           iModeNum,
  LPDEVMODEW      lpDevMode,
  DWORD           dwFlags
);

#define ENUM_CURRENT_SETTINGS  ((DWORD) -1)
#define ENUM_REGISTRY_SETTINGS ((DWORD) -2)

LONG ChangeDisplaySettingsExW(
  LPCWSTR     lpszDeviceName,
  LPDEVMODEW  lpDevMode,
  HWND        hwnd,
  DWORD       dwflags,
  LPVOID      lParam
);

#define CDS_FULLSCREEN              0x00000004

HDC CreateDCW(
  LPCWSTR         pwszDriver,
  LPCWSTR         pwszDevice,
  LPCWSTR         pszPort,
  const DEVMODEW *pdw
);

BOOL SetDeviceGammaRamp(
  HDC          hdc,
  LPVOID       lpRamp
);

BOOL GetDeviceGammaRamp(
  HDC          hdc,
  LPVOID       lpRamp
);

#define DMDO_DEFAULT            0
#define DMDO_90                 1
#define DMDO_180                2
#define DMDO_270                3

HICON LoadIconA(HINSTANCE hInstance, LPCSTR lpIconName);
HCURSOR LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName);

#define IDI_WINLOGO           MAKEINTRESOURCEA(32517)
#define IDC_ARROW             MAKEINTRESOURCEA(32512)