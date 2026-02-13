#include <windows.h>

#include "ntstatus.h"
#include "ddk/d3dkmthk.h"

#include <SDL3/SDL.h>

#pragma clang optimize off

BOOL IsWindow(HWND hWnd)
{
  return 0;
}

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

BOOL ClientToScreen(HWND hWnd, LPPOINT lpPoint)
{
  // From my understanding this is only useful, for windows with decorations
  // which we don't have for most games, so we can just return the same point.
  // This should be sufficient for most use cases.
  return TRUE;
}

BOOL MoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint)
{
  STUBBED();
  return FALSE;
}

LONG GetWindowLongW(HWND hWnd, int nIndex)
{
  STUBBED();
  return 0;
}

LONG SetWindowLongW(HWND hWnd, int nIndex, LONG dwNewLong)
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
  if (lpDevice != 0)
    return FALSE;

  // This is one of the first functions, as such SDL needs to be initialized here
  if (!SDL_WasInit(SDL_INIT_VIDEO))
  {
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
      fprintf(stderr, "EnumDisplayDevicesW: Failed to initialize SDL video subsystem: %s\n", SDL_GetError());
      return FALSE;
    }
  }
  
  // Collect all display devices via SDL
  int numDisplays = 0;
  SDL_DisplayID *displays = SDL_GetDisplays(&numDisplays);
  if (!displays)
  {
    fprintf(stderr, "EnumDisplayDevicesW: SDL_GetDisplays failed: %s\n", SDL_GetError());
    return FALSE;
  }

  if (iDevNum >= (DWORD)numDisplays)
  {
    SDL_free(displays);
    return FALSE;
  }

  DISPLAY_DEVICEW *device = lpDisplayDevice;
  SDL_DisplayID display = displays[iDevNum];
  if (device->cb < sizeof(DISPLAY_DEVICEW))
  {
    SDL_free(displays);
    fprintf(stderr, "EnumDisplayDevicesW: DISPLAY_DEVICEW size is too small\n");
    return FALSE;
  }

  // Fill in the DISPLAY_DEVICEW structure
  ZeroMemory(device, sizeof(DISPLAY_DEVICEW));
  device->cb = sizeof(DISPLAY_DEVICEW);
  const char *name = SDL_GetDisplayName(display);
  mbsrtowcs(device->DeviceName, &name, 32, NULL);
  // For DeviceString, we can use the same as DeviceName for now
  name = SDL_GetDisplayName(display);
  mbsrtowcs(device->DeviceString, &name, 128, NULL);
  // DISPLAY_DEVICE_PRIMARY_DEVICE is considered by wined3d but not set for now
  device->StateFlags = DISPLAY_DEVICE_ATTACHED_TO_DESKTOP;

  SDL_free(displays);
  return TRUE;
}

BOOL EnumDisplaySettingsW(LPCWSTR lpszDeviceName, DWORD iModeNum, LPDEVMODEW lpDevMode)
{
  STUBBED();
  return 0;
}

BOOL EnumDisplaySettingsExW(LPCWSTR lpszDeviceName, DWORD iModeNum, LPDEVMODEW lpDevMode, DWORD dwFlags)
{
  if (lpDevMode == NULL)
    return FALSE;
  if (lpDevMode->dmSize < sizeof(DEVMODEW))
    return FALSE;

  lpDevMode->dmSize = sizeof(DEVMODEW);

  // TODO: Match lpszDeviceName to the correct display
  int numDisplays = 0;
  SDL_DisplayID *displays = SDL_GetDisplays(&numDisplays);
  if (!displays)
  {
    fprintf(stderr, "EnumDisplaySettingsExW: SDL_GetDisplays failed: %s\n", SDL_GetError());
    return FALSE;
  }

  if (displays[0] == 0)
  {
    fprintf(stderr, "EnumDisplaySettingsExW: No displays found\n");
    SDL_free(displays);
    return FALSE;
  }

  

  int numDisplayModes = 0;
  const SDL_DisplayMode **displayModes = NULL;
  const SDL_DisplayMode *currentMode = NULL;

  if (iModeNum != ENUM_CURRENT_SETTINGS)
  {
    displayModes = (const SDL_DisplayMode**)SDL_GetFullscreenDisplayModes(displays[0], &numDisplayModes);
    if (displayModes == NULL)
    {
      fprintf(stderr, "EnumDisplaySettingsExW: SDL_GetFullscreenDisplayModes failed: %s\n", SDL_GetError());
      SDL_free(displays);
      return FALSE;
    }
  }

  if (iModeNum == ENUM_CURRENT_SETTINGS || numDisplayModes == 0)
  {
    if (numDisplayModes == 0)
    {
      fprintf(stderr, "EnumDisplaySettingsExW: No display modes found, falling back to current display mode\n");
    }
    numDisplayModes = 1;
    currentMode = SDL_GetCurrentDisplayMode(displays[0]);
    if (currentMode == NULL)
    {
      fprintf(stderr, "EnumDisplaySettingsExW: SDL_GetCurrentDisplayMode failed: %s\n", SDL_GetError());
      SDL_free(displays);
      return FALSE;
    }
  }

  if (iModeNum >= (DWORD)numDisplayModes && iModeNum != ENUM_CURRENT_SETTINGS)
  {
    fprintf(stderr, "EnumDisplaySettingsExW: Requested mode %u out of range (%d modes available)\n", iModeNum, numDisplayModes);
    SDL_free(displayModes);
    SDL_free(displays);
    return FALSE;
  }

  const SDL_DisplayMode *mode = NULL;
  // Either requested through ENUM_CURRENT_SETTINGS or selected as fallback
  if (currentMode != NULL)
    mode = currentMode;
  else
    mode = displayModes[iModeNum];
  lpDevMode->dmPelsWidth = mode->w;
  lpDevMode->dmPelsHeight = mode->h;
  lpDevMode->dmBitsPerPel = SDL_BITSPERPIXEL(mode->format);
  lpDevMode->dmDisplayFrequency = mode->refresh_rate ? mode->refresh_rate : 60; // SDL may return 0 for some modes, in that case we can default to 60Hz
  lpDevMode->dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
  SDL_free(displayModes);
  SDL_free(displays);
  return TRUE;
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

NTSTATUS WINAPI D3DKMTEscape( const D3DKMT_ESCAPE *desc )
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
)
{
  STUBBED();
  return FALSE;
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
  return TRUE;
}

BOOL UnregisterClassA(LPCSTR lpClassName, HINSTANCE hInstance)
{
  STUBBED();
  return TRUE;
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

BOOL AdjustWindowRectEx(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle)
{
  STUBBED();
  return FALSE;
}

BOOL GetWindowRect(HWND hWnd, LPRECT lpRect)
{
  STUBBED();
  return FALSE;
}

INT MapWindowPoints(HWND hWndFrom, HWND hWndTo, LPPOINT lpPoints, UINT cPoints)
{
  STUBBED();
  return 0;
}