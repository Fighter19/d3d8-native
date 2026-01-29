#include <windows.h>

#include "ntstatus.h"
#include "ddk/d3dkmthk.h"

BOOL IsWindowUnicode(HWND hWnd)
{
  STUBBED();
  return FALSE;
}

BOOL IsWindowVisible(HWND hWnd)
{
  STUBBED();
  return FALSE;
}

BOOL ShowWindow(HWND hWnd, int nCmdShow)
{
  STUBBED();
  return FALSE;
}

DWORD GetWindowThreadProcessId(HWND hWnd, LPDWORD lpdwProcessId)
{
  STUBBED();
  return 0;
}

LONG_PTR SetWindowLongPtrA(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
{
  STUBBED();
  return 0;
}

LONG_PTR SetWindowLongPtrW(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
{
  STUBBED();
  return 0;
}

LONG_PTR GetWindowLongPtrA(HWND hWnd, int nIndex)
{
  STUBBED();
  return 0;
}

LONG_PTR GetWindowLongPtrW(HWND hWnd, int nIndex)
{
  STUBBED();
  return 0;
}

BOOL EnumDisplayDevicesW(
    LPCWSTR lpDevice,
    DWORD iDevNum,
    PDISPLAY_DEVICEW lpDisplayDevice,
    DWORD dwFlags)
{
  STUBBED();
  return FALSE;
}

BOOL EnumDisplaySettingsW(LPCWSTR lpszDeviceName, DWORD iModeNum, LPDEVMODEW lpDevMode)
{
  STUBBED();
  return 0;
}

BOOL EnumDisplaySettingsExW(LPCWSTR lpszDeviceName, DWORD iModeNum, LPDEVMODEW lpDevMode, DWORD dwFlags)
{
  STUBBED();
  return 0;
}

LONG ChangeDisplaySettingsExW(LPCWSTR lpszDeviceName, LPDEVMODEW lpDevMode, HWND hwnd, DWORD dwflags, LPVOID lParam)
{
  STUBBED();
  return 0;
}

HMONITOR MonitorFromWindow(HWND hwnd, DWORD dwFlags)
{
  STUBBED();
  return NULL;
}

BOOL GetMonitorInfoA(HMONITOR hMonitor, LPMONITORINFO lpmi)
{
  STUBBED();
  return FALSE;
}

BOOL GetMonitorInfoW(HMONITOR hMonitor, LPMONITORINFO lpmi)
{
  STUBBED();
  return FALSE;
}

BOOL EnumDisplayMonitors(
    HDC hdc,
    const LPRECT lprcClip,
    BOOL(CALLBACK *lpfnEnum)(HMONITOR, HDC, LPRECT, LPARAM),
    LPARAM dwData)
{
  STUBBED();
  return FALSE;
}

HDC CreateDCW(LPCWSTR pwszDriver, LPCWSTR pwszDevice, LPCWSTR pszPort, const DEVMODEW *pdw)
{
  STUBBED();
  return NULL;
}

BOOL SetDeviceGammaRamp(HDC hdc, LPVOID lpRamp)
{
  STUBBED();
  return FALSE;
}

BOOL GetDeviceGammaRamp(HDC hdc, LPVOID lpRamp)
{
  STUBBED();
  return FALSE;
}

NTSTATUS WINAPI D3DKMTCreateDevice(D3DKMT_CREATEDEVICE *desc)
{
  STUBBED();
  return 0;
}

NTSTATUS WINAPI D3DKMTDestroyDevice(const D3DKMT_DESTROYDEVICE *desc)
{
  STUBBED();
  return 0;
}

NTSTATUS WINAPI D3DKMTDestroyDCFromMemory(const D3DKMT_DESTROYDCFROMMEMORY *desc)
{
  STUBBED();
  return 0;
}

NTSTATUS WINAPI D3DKMTCloseAdapter(const D3DKMT_CLOSEADAPTER *desc)
{
  STUBBED();
  return 0;
}

NTSTATUS WINAPI D3DKMTCreateDCFromMemory(D3DKMT_CREATEDCFROMMEMORY *desc)
{
  STUBBED();
  return 0;
}

NTSTATUS WINAPI D3DKMTOpenAdapterFromLuid(D3DKMT_OPENADAPTERFROMLUID *desc)
{
  STUBBED();
  return 0;
}

NTSTATUS WINAPI D3DKMTSetVidPnSourceOwner(const D3DKMT_SETVIDPNSOURCEOWNER *desc)
{
  STUBBED();
  return 0;
}

NTSTATUS WINAPI D3DKMTOpenAdapterFromGdiDisplayName(D3DKMT_OPENADAPTERFROMGDIDISPLAYNAME *desc)
{
  STUBBED();
  return 0;
}

NTSTATUS WINAPI D3DKMTQueryVideoMemoryInfo(D3DKMT_QUERYVIDEOMEMORYINFO *desc)
{
  STUBBED();
  return 0;
}

HCURSOR SetCursor(HCURSOR hCursor)
{
  STUBBED();
  return NULL;
}

BOOL SetCursorPos(INT X, INT Y)
{
  STUBBED();
  return FALSE;
}

BOOL GetCursorPos(LPPOINT lpPoint)
{
  STUBBED();
  return FALSE;
}

void DestroyCursor(HCURSOR hCursor)
{
  STUBBED();
}

HICON CreateIconIndirect(const ICONINFO *piconinfo)
{
  STUBBED();
  return NULL;
}

HICON LoadIconA(HINSTANCE hInstance, LPCSTR lpIconName)
{
  return NULL;
}

HCURSOR LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName)
{
  return NULL;
}

HBITMAP LoadImageA(
    HINSTANCE hinst,
    LPCSTR lpszName,
    UINT uType,
    INT cxDesired,
    INT cyDesired,
    UINT fuLoad)
{
  STUBBED();
  return NULL;
}

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
)
{
  STUBBED();
  return FALSE;
}

HBITMAP CreateBitmap(
    INT nWidth,
    INT nHeight,
    UINT cPlanes,
    UINT cBitsPerPel,
    const VOID *lpvBits)
{
  STUBBED();
  return NULL;
}

UINT SetDIBColorTable(HDC hdc, UINT iStart, UINT cEntries, const RGBQUAD *pcr)
{
  STUBBED();
  return 0;
}

HDC CreateCompatibleDC(HDC hdc)
{
  STUBBED();
  return NULL;
}

BOOL DeleteDC(HDC hdc)
{
  STUBBED();
  return FALSE;
}

int GetObjectA(HGDIOBJ h, int c, LPVOID pv)
{
  STUBBED();
  return 0;
}

HGDIOBJ SelectObject(HDC hdc, HGDIOBJ hgdiobj)
{
  STUBBED();
  return NULL;
}

BOOL DeleteObject(HGDIOBJ hObject)
{
  STUBBED();
  return FALSE;
}

BOOL RegisterClassA(const WNDCLASSA *lpWndClass)
{
  STUBBED();
  return FALSE;
}

BOOL UnregisterClassA(LPCSTR lpClassName, HINSTANCE hInstance)
{
  STUBBED();
  return FALSE;
}

BOOL SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
  return 0;
}

LRESULT DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  STUBBED();
  return 0;
}

LRESULT DefWindowProcW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  STUBBED();
  return 0;
}

LRESULT CallWindowProcA(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  STUBBED();
  return 0;
}

LRESULT CallWindowProcW(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  STUBBED();
  return 0;
}

BOOL ScreenToClient(HWND hWnd, LPPOINT lpPoint)
{
  STUBBED();
  return FALSE;
}

BOOL OffsetRect(LPRECT lprc, int dx, int dy)
{
  lprc->left += dx;
  lprc->right += dx;
  lprc->top += dy;
  lprc->bottom += dy;
  return TRUE;
}

INT MapWindowPoints(HWND hWndFrom, HWND hWndTo, LPPOINT lpPoints, UINT cPoints)
{
  STUBBED();
  return 0;
}