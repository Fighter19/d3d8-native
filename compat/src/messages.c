#include <windows.h>

HHOOK SetWindowsHookExW(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId)
{
  STUBBED();
  return NULL;
}

BOOL UnhookWindowsHookEx(HHOOK hhk)
{
  STUBBED();
  return FALSE;
}

LRESULT CallNextHookEx(HHOOK hhk, int nCode, WPARAM wParam, LPARAM lParam)
{
  STUBBED();
  return 0;
}

UINT_PTR SetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc)
{
  STUBBED();
  return 0;
}

BOOL KillTimer(HWND hWnd, UINT_PTR uIDEvent)
{
  STUBBED();
  return FALSE;
}