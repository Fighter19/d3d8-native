#pragma once
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <assert.h>
#include <stdbool.h>
#include <ctype.h>
#include <wctype.h>
#include <basetsd.h>
typedef wchar_t WCHAR;
typedef int HRESULT;
typedef int INT;

typedef int32_t INT;
typedef uint32_t UINT;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;

typedef size_t SIZE_T;

typedef int32_t LONG;
typedef uint32_t ULONG;
typedef uint64_t ULONG64;
typedef uintptr_t ULONG_PTR;
typedef uint32_t DWORD;
typedef unsigned char BYTE;
typedef BYTE *LPBYTE;
typedef char CHAR;
typedef char* PCHAR;
typedef unsigned char UCHAR;
typedef BYTE byte;
typedef uint16_t WORD;
typedef int16_t SHORT;
typedef uint16_t USHORT;

typedef LONG LRESULT;

typedef LONG_PTR LPARAM;
typedef size_t WPARAM;

typedef float FLOAT;

typedef CHAR *LPSTR;
typedef const CHAR* LPCSTR;

typedef wchar_t* LPWSTR;
typedef const wchar_t* LPCWSTR;


typedef int64_t LONGLONG;
typedef uint64_t ULONGLONG;

typedef DWORD LCID;
typedef int BOOL;

typedef DWORD *LPDWORD;

typedef void VOID;
typedef VOID* LPVOID;

typedef VOID* HANDLE;
typedef HANDLE* LPHANDLE;

#define DECLARE_HANDLE(name) typedef struct name##__ { int unused; } *name;
DECLARE_HANDLE(HGLOBAL)
DECLARE_HANDLE(HTASK)
DECLARE_HANDLE(HBITMAP)
DECLARE_HANDLE(HICON)
DECLARE_HANDLE(HDC)
DECLARE_HANDLE(HENHMETAFILE)
DECLARE_HANDLE(HWND)
DECLARE_HANDLE(HINSTANCE)
DECLARE_HANDLE(HMONITOR)
DECLARE_HANDLE(HMODULE)
DECLARE_HANDLE(HCURSOR)
DECLARE_HANDLE(HGLRC)
DECLARE_HANDLE(HRSRC)
DECLARE_HANDLE(HHOOK)
DECLARE_HANDLE(HKEY)
DECLARE_HANDLE(HBRUSH)
DECLARE_HANDLE(HRGN)

typedef struct LUID {
  DWORD LowPart;
  LONG  HighPart;
} LUID;

typedef struct POINT {
  LONG x;
  LONG y;
} POINT;

typedef POINT* LPPOINT;

typedef struct _POINTL
{
    LONG x;
    LONG y;
} POINTL, *PPOINTL;

typedef struct RECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT,*PRECT,*NPRECT,*LPRECT;

typedef union {
  struct {
    DWORD LowPart;
    LONG HighPart;
  };

  struct {
    DWORD LowPart;
    LONG HighPart;
  } u;

  LONGLONG QuadPart;
} LARGE_INTEGER;

typedef union {
  struct {
    DWORD LowPart;
    DWORD HighPart;
  };

  struct {
    DWORD LowPart;
    DWORD HighPart;
  } u;

  ULONGLONG QuadPart;
} ULARGE_INTEGER;

typedef struct RGNDATAHEADER {
  DWORD dwSize;
  DWORD iType;
  DWORD nCount;
  DWORD nRgnSize;
  RECT  rcBound;
} RGNDATAHEADER;

typedef struct RGNDATA {
  RGNDATAHEADER rdh;
  char          Buffer[1];
} RGNDATA,*PRGNDATA,*NPRGNDATA,*LPRGNDATA;

typedef struct RGBQUAD {
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
} RGBQUAD;

typedef DWORD COLORREF;

typedef struct SIZE {
  LONG cx;
  LONG cy;
} SIZE;

typedef struct _GLYPHMETRICSFLOAT GLYPHMETRICSFLOAT, *PGLYPHMETRICSFLOAT, *LPGLYPHMETRICSFLOAT;
typedef struct tagPIXELFORMATDESCRIPTOR
{
  WORD nSize;
  WORD nVersion;
  DWORD dwFlags;
  BYTE iPixelType;
  BYTE cColorBits;
  BYTE cRedBits;
  BYTE cRedShift;
  BYTE cGreenBits;
  BYTE cGreenShift;
  BYTE cBlueBits;
  BYTE cBlueShift;
  BYTE cAlphaBits;
  BYTE cAlphaShift;
  BYTE cAccumBits;
  BYTE cAccumRedBits;
  BYTE cAccumGreenBits;
  BYTE cAccumBlueBits;
  BYTE cAccumAlphaBits;
  BYTE cDepthBits;
  BYTE cStencilBits;
  BYTE cAuxBuffers;
  BYTE iLayerType;
  BYTE bReserved;
  DWORD dwLayerMask;
  DWORD dwVisibleMask;
  DWORD dwDamageMask;
} PIXELFORMATDESCRIPTOR, *PPIXELFORMATDESCRIPTOR, *LPPIXELFORMATDESCRIPTOR;

#define PFD_TYPE_RGBA        0
#define PFD_TYPE_COLORINDEX  1

#define PFD_MAIN_PLANE       0
#define PFD_OVERLAY_PLANE    1
#define PFD_UNDERLAY_PLANE   (-1)

#define PFD_DOUBLEBUFFER          0x00000001
#define PFD_STEREO                0x00000002
#define PFD_DRAW_TO_WINDOW        0x00000004
#define PFD_DRAW_TO_BITMAP        0x00000008
#define PFD_SUPPORT_GDI           0x00000010
#define PFD_SUPPORT_OPENGL        0x00000020
#define PFD_GENERIC_FORMAT        0x00000040
#define PFD_NEED_PALETTE          0x00000080
#define PFD_NEED_SYSTEM_PALETTE   0x00000100
#define PFD_SWAP_EXCHANGE         0x00000200
#define PFD_SWAP_COPY             0x00000400
#define PFD_SWAP_LAYER_BUFFERS    0x00000800
#define PFD_GENERIC_ACCELERATED   0x00001000
#define PFD_SUPPORT_COMPOSITION   0x00008000 /* Vista stuff */

#define PFD_DEPTH_DONTCARE        0x20000000
#define PFD_DOUBLEBUFFER_DONTCARE 0x40000000
#define PFD_STEREO_DONTCARE       0x80000000

// TODO: I have no idea what this is
typedef void* hyper;

typedef struct _SECURITY_ATTRIBUTES SECURITY_ATTRIBUTES;
typedef struct SECURITY_DESCRIPTOR *PSECURITY_DESCRIPTOR;
typedef struct _RPC_AUTHZ_HANDLE RPC_AUTHZ_HANDLE;

typedef struct _OBJECT_ATTRIBUTES OBJECT_ATTRIBUTES;
typedef struct _LAYERPLANEDESCRIPTOR LAYERPLANEDESCRIPTOR;

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif


// Defined in guiddef.h
// typedef struct _GUID
// {
//     unsigned long  Data1;
//     unsigned short Data2;
//     unsigned short Data3;
//     unsigned char  Data4[8];
// } GUID;
#include "guiddef.h"

#define S_OK ((HRESULT)0L)
#define S_FALSE ((HRESULT)1L)
#define E_NOTIMPL ((HRESULT)0x80004001L)
#define E_NOINTERFACE ((HRESULT)0x80004002L)
#define E_POINTER ((HRESULT)0x80004003L)
#define E_ABORT ((HRESULT)0x80004004L)
#define E_FAIL ((HRESULT)0x80004005L)
#define E_OUTOFMEMORY ((HRESULT)0x8007000EL)
#define E_INVALIDARG ((HRESULT)0x80070057L)
#define DXGI_ERROR_ALREADY_EXISTS ((HRESULT)0x887A0036L)
#define DXGI_ERROR_NOT_FOUND ((HRESULT)0x887A0002L)
#define DXGI_ERROR_MORE_DATA ((HRESULT)0x887A0003L)
#define DXGI_ERROR_UNSUPPORTED ((HRESULT)0x887A0004L)
#define DXGI_ERROR_NOT_CURRENTLY_AVAILABLE ((HRESULT)0x887a0022)

#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define DECL_WINELIB_TYPE_AW(x)
#define DECLSPEC_IMPORT
#define DECLSPEC_EXPORT
#define DECLSPEC_NORETURN __attribute__((noreturn))

#define CONST const
#define CONST_VTBL const

#define TRUE 1
#define FALSE 0

#define interface struct

// There's no stdcall on non-Windows platforms
#define __stdcall
#define STDMETHODCALLTYPE
#define CALLBACK
#define WINAPI
#define STDAPI HRESULT STDMETHODCALLTYPE
#define CDECL __cdecl

#define FIELD_OFFSET(type, field)    (LONG)offsetof(type, field)

#define MAKE_HRESULT(sev,fac,code) \
    ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )

typedef struct IAgileReference IAgileReference;

// My guess is that these are for annotating memory allocation functions for sanitizers
#define __WINE_ALLOC_SIZE(x)
#define __WINE_DEALLOC(x)
#define __WINE_MALLOC

#ifdef NONAMELESSUNION
#define DUMMYSTRUCTNAME s
#define DUMMYSTRUCTNAME1 s1
#define DUMMYSTRUCTNAME2 s2
#define DUMMYSTRUCTNAME3 s3

#define DUMMYUNIONNAME u
#define DUMMYUNIONNAME1 u1
#define DUMMYUNIONNAME2 u2
#define DUMMYUNIONNAME3 u3
#else
#define DUMMYSTRUCTNAME
#define DUMMYSTRUCTNAME1
#define DUMMYSTRUCTNAME2
#define DUMMYSTRUCTNAME3

#define DUMMYUNIONNAME
#define DUMMYUNIONNAME1
#define DUMMYUNIONNAME2
#define DUMMYUNIONNAME3
#endif

#define __C89_NAMELESS

#define BEGIN_INTERFACE
#define END_INTERFACE

#define CONTAINING_RECORD(address, type, field) \
  ((type *)((PCHAR)(address) - offsetof(type, field)))

#include "rpc.h"
#include "wtypes.h"

typedef LRESULT (*WNDPROC)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef int (*PROC)();

#define C_ASSERT(e) _Static_assert(e, #e)
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

#include "internal/compat_atomic.h"
#include "internal/compat_mutex.h"
#include "internal/compat_thread.h"

HGLRC wglCreateContext(HDC hdc);
BOOL wglMakeCurrent(HDC hdc, HGLRC hglrc);
HDC wglGetCurrentDC(void);
HGLRC wglGetCurrentContext(void);
BOOL wglDeleteContext(HGLRC hglrc);
BOOL wglShareLists(HGLRC hglrc1, HGLRC hglrc2);
HWND GetDesktopWindow(void);
HWND WindowFromDC(HDC dc);
HDC GetDC(HWND hWnd);
HDC GetDCEx(HWND hWnd, HRGN hrgnClip, DWORD flags);
#define DCX_USESTYLE 0x00000010
#define DCX_CACHE    0x00000002
HWND CreateWindowA(
  LPCSTR               lpClassName,
  LPCSTR               lpWindowName,
  DWORD                dwStyle,
  int                  X,
  int                  Y,
  int                  nWidth,
  int                  nHeight,
  HWND                 hWndParent,
  HANDLE               hMenu,
  HINSTANCE            hInstance,
  LPVOID               lpParam
);
void DestroyWindow(HWND hWnd);

INT ReleaseDC(HWND hWnd, HDC hDC);

// TODO: This is a placeholder
#define WS_OVERLAPPEDWINDOW 0
#define WS_POPUP          0x80000000
#define WS_SYSMENU        0x00080000
#define WS_CAPTION       0x00C00000
#define WS_THICKFRAME    0x00040000
#define WS_EX_WINDOWEDGE 0x00000100
#define WS_EX_CLIENTEDGE 0x00000200
#define WS_EX_TOPMOST    0x00000008

int ChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd);
int GetPixelFormat(HDC hdc);
int SetPixelFormat(HDC hdc, int format, const PIXELFORMATDESCRIPTOR *ppfd);
int DescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, PIXELFORMATDESCRIPTOR *ppfd);

PROC wglGetProcAddress(const char *lpszProc);

PROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
HMODULE GetModuleHandleA(LPCSTR lpModuleName);
BOOL GetModuleHandleExA(DWORD dwFlags, LPCSTR lpModuleName, HMODULE *phModule);
DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize);

#define GET_MODULE_HANDLE_EX_FLAG_PIN                 1
#define GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT  2
#define GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS        4

static inline unsigned int GetLastError()
{
  // Retrieves the calling thread's last-error code value
  assert(false && "GetLastError is not implemented yet");

  return 0;
}

HCURSOR SetCursor(HCURSOR hCursor);
BOOL SetCursorPos(INT X, INT Y);
BOOL GetCursorPos(LPPOINT lpPoint);
void DestroyCursor(HCURSOR hCursor);

BOOL GetClientRect(HWND hWnd, LPRECT lpRect);

#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))
#define I64_MAX 0x7FFFFFFFFFFFFFFF

BOOL QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency);
BOOL QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount);
DWORD GetTickCount(void);

INT MapWindowPoints(HWND hWndFrom, HWND hWndTo, LPPOINT lpPoints, UINT cPoints);

#define INFINITE      0xFFFFFFFF

static inline BOOL WINAPI SetRect(LPRECT rect, INT left, INT top, INT right, INT bottom)
{
    if (!rect) return FALSE;
    rect->left   = left;
    rect->right  = right;
    rect->top    = top;
    rect->bottom = bottom;
    return TRUE;
}

static inline BOOL WINAPI SetRectEmpty(LPRECT rect)
{
    if (!rect) return FALSE;
    rect->left = rect->right = rect->top = rect->bottom = 0;
    return TRUE;
}

static inline BOOL WINAPI IsRectEmpty(const RECT *rect)
{
    if (!rect) return TRUE;
    return (rect->left >= rect->right) || (rect->top >= rect->bottom);
}

static inline BOOL IntersectRect(LPRECT lprcDst, const RECT *lprc1, const RECT *lprc2)
{
    if (!lprc1 || !lprc2 || !lprcDst) return FALSE;

    lprcDst->left   = max(lprc1->left,   lprc2->left);
    lprcDst->top    = max(lprc1->top,    lprc2->top);
    lprcDst->right  = min(lprc1->right,  lprc2->right);
    lprcDst->bottom = min(lprc1->bottom, lprc2->bottom);

    return (lprcDst->left < lprcDst->right && lprcDst->top < lprcDst->bottom);
}

BOOL CloseHandle(HANDLE hObject);

LPVOID HeapCreate(DWORD flOptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize);
BOOL HeapDestroy(LPVOID hHeap);
LPVOID HeapAlloc(LPVOID hHeap, DWORD dwFlags, SIZE_T dwBytes);
BOOL HeapFree(LPVOID hHeap, DWORD dwFlags, LPVOID lpMem);

UINT SetDIBColorTable(HDC hdc, UINT iStart, UINT cEntries, const RGBQUAD *pcr);

typedef struct _ICONINFO {
	BOOL		fIcon;
	DWORD		xHotspot;
	DWORD		yHotspot;
	HBITMAP	hbmMask;
	HBITMAP	hbmColor;
} ICONINFO, *PICONINFO;

#include "internal/compat_gdi.h"

BOOL SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);

#define SWP_NOSIZE          0x0001
#define SWP_NOMOVE          0x0002

// SystemParametersInfoW
#define SPI_GETSCREENSAVEACTIVE 0x0010
#define SPI_SETSCREENSAVEACTIVE 0x0011
BOOL SystemParametersInfoW(UINT uiAction, UINT uiParam, LPVOID lpvParam, UINT fWinIni);

#define WM_DESTROY             0x0002
#define WM_DISPLAYCHANGE       0x007e
#define WM_ACTIVATEAPP         0x001c
#define WM_SYSCOMMAND          0x0112

#define SC_RESTORE             0xf120
LRESULT DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT DefWindowProcW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CallWindowProcA(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CallWindowProcW(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

#define stricmp strcasecmp

BOOL ScreenToClient(HWND hWnd, LPPOINT lpPoint);
BOOL OffsetRect(LPRECT lprc, int dx, int dy);

static inline LPWSTR WINAPI lstrcpyW( LPWSTR dst, LPCWSTR src )
{
    WCHAR *p = dst;
    while ((*p++ = *src++));
    return dst;
}

static inline bool lstrcmpiW(LPCWSTR str1, LPCWSTR str2)
{
    while (*str1 && *str2)
    {
        WCHAR c1 = towlower(*str1++);
        WCHAR c2 = towlower(*str2++);
        if (c1 != c2)
            return c1 - c2;
    }
    return *str1 - *str2;
}

void *_recalloc(void *ptr, size_t num, size_t size);

#define MAKEINTRESOURCEA(i) (LPSTR)((ULONG_PTR)((WORD)(i)))
#define MAKEINTRESOURCEW(i) (LPWSTR)((ULONG_PTR)((WORD)(i)))
#define MAKEINTRESOURCE(i)  MAKEINTRESOURCEA(i)
//#define RT_BITMAP MAKEINTRESOURCEA(2)
#define RT_RCDATA         MAKEINTRESOURCE(10)

HRSRC FindResourceA(HMODULE hModule, LPCSTR lpName, LPCSTR lpType);
#ifndef USE_HGLOBAL_FOR_RESOURCES
// HGLOBAL is only used on Windows for backward compatibility
HRSRC LoadResource(HMODULE hModule, HRSRC hResInfo);
LPVOID LockResource(HRSRC hResData);
BOOL FreeResource(HRSRC hResData);
#else
HGLOBAL LoadResource(HMODULE hModule, HRSRC hResInfo);
LPVOID LockResource(HGLOBAL hResData);
BOOL FreeResource(HGLOBAL hResData);
#endif

typedef LONG LSTATUS;

DWORD SizeofResource(HMODULE hModule, HRSRC hResInfo);
LSTATUS RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType,
                      LPBYTE lpData, LPDWORD lpcbData);

LSTATUS RegCloseKey(HKEY hKey);
LSTATUS RegOpenKeyA(HKEY hKey, LPCSTR lpSubKey, HKEY *phkResult);

#define REG_DWORD          4
#define HKEY_CURRENT_USER  ((HKEY)(ULONG_PTR)((LONG)0x80000001))

typedef struct _WNDCLASSA
{
  UINT style;
  WNDPROC lpfnWndProc;
  int cbClsExtra;
  int cbWndExtra;
  HINSTANCE hInstance;
  HICON hIcon;
  HCURSOR hCursor;
  HBRUSH hbrBackground;
  LPCSTR lpszMenuName;
  LPCSTR lpszClassName;
} WNDCLASSA;

#define CS_VREDRAW      0x0001
#define CS_HREDRAW      0x0002

BOOL RegisterClassA(const WNDCLASSA *lpWndClass);
BOOL UnregisterClassA(LPCSTR lpClassName, HINSTANCE hInstance);

#define ERROR_FILE_NOT_FOUND 2L
#define ERROR_CALL_NOT_IMPLEMENTED                         120
#define MAX_PATH           260

BOOL UnhookWindowsHookEx(HHOOK hhk);

HMONITOR MonitorFromWindow(HWND hwnd, DWORD dwFlags);
BOOL GetMonitorInfoA(HMONITOR hMonitor, LPMONITORINFO lpmi);
BOOL GetMonitorInfoW(HMONITOR hMonitor, LPMONITORINFO lpmi);

BOOL EnumDisplayMonitors(
  HDC             hdc,
  const LPRECT    lprcClip,
  BOOL            (CALLBACK *lpfnEnum)(HMONITOR, HDC, LPRECT, LPARAM),
  LPARAM          dwData
);

typedef struct _MSG {
  HWND   hwnd;
  UINT   message;
  WPARAM wParam;
  LPARAM lParam;
} MSG, *PMSG, *NPMSG, *LPMSG;

LONG_PTR SetWindowLongPtrA(HWND hWnd, int nIndex, LONG_PTR dwNewLong);
LONG_PTR SetWindowLongPtrW(HWND hWnd, int nIndex, LONG_PTR dwNewLong);

LONG_PTR GetWindowLongPtrA(HWND hWnd, int nIndex);
LONG_PTR GetWindowLongPtrW(HWND hWnd, int nIndex);

#define GWLP_HINSTANCE       (-6)
#define GWLP_WNDPROC         (-4)

LONG GetWindowLongW(HWND hWnd, int nIndex);

#define GWL_STYLE           (-16)
#define GWL_EXSTYLE         (-20)

#define KF_ALTDOWN          0x2000
#define HC_ACTION           0
#define WM_SYSKEYDOWN       0x0104
#define VK_RETURN           0x0D



BOOL ClientToScreen(HWND hWnd, LPPOINT lpPoint);
BOOL GetWindowRect(HWND hWnd, LPRECT lpRect);

#define SWP_NOACTIVATE    0x0010
#define SWP_NOZORDER      0x0004

#define SW_MINIMIZE 6

LRESULT CallNextHookEx(HHOOK hhk, int nCode, WPARAM wParam, LPARAM lParam);

typedef LRESULT (CALLBACK *HOOKPROC)(int code, WPARAM wParam, LPARAM lParam);
HHOOK SetWindowsHookExW(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId);
#define WH_GETMESSAGE        3

BOOL IsWindowUnicode(HWND hWnd);
BOOL IsWindowVisible(HWND hWnd);

BOOL ShowWindow(HWND hWnd, int nCmdShow);

DWORD GetWindowThreadProcessId(HWND hWnd, LPDWORD lpdwProcessId);

#define    DLL_PROCESS_DETACH      0
#define    DLL_PROCESS_ATTACH      1
#define    DLL_THREAD_ATTACH       2
#define    DLL_THREAD_DETACH       3

typedef struct MEMORYSTATUSEX {
  DWORD dwLength;
  DWORD dwMemoryLoad;
  ULONGLONG ullTotalPhys;
  ULONGLONG ullAvailPhys;
  ULONGLONG ullTotalPageFile;
  ULONGLONG ullAvailPageFile;
  ULONGLONG ullTotalVirtual;
  ULONGLONG ullAvailVirtual;
  ULONGLONG ullAvailExtendedVirtual;
} MEMORYSTATUSEX, *LPMEMORYSTATUSEX;

typedef struct OSVERSIONINFOW {
  DWORD dwOSVersionInfoSize;
  DWORD dwMajorVersion;
  DWORD dwMinorVersion;
  DWORD dwBuildNumber;
  DWORD dwPlatformId;
  WCHAR szCSDVersion[128];
} OSVERSIONINFOW, *LPOSVERSIONINFOW;

BOOL GetVersionExW(LPOSVERSIONINFOW lpVersionInformation);
BOOL GlobalMemoryStatusEx(LPMEMORYSTATUSEX lpBuffer);

BOOL AllocateLocallyUniqueId(LUID *pluid);



BOOL StretchBlt(
  HDC     hdcDest,
  int     xDest,
  int     yDest,
  int     wDest,
  int     hDest,
  HDC     hdcSrc,
  int     xSrc,
  int     ySrc,
  int     wSrc,
  int     hSrc,
  DWORD   rop
);

#include <stdio.h>
#define STUBBED(x) fprintf(stderr, "STUBBED: %s\n", __FUNCTION__);