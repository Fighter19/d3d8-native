#include <windows.h>

BOOL CloseHandle(HANDLE hObject)
{
  // TODO: Implement closable handles:
  // Currently only thread and event are in use by WineD3D,
  // which shouldn't be a big deal if leaked.
  STUBBED();
  return FALSE;
}

BOOL SystemParametersInfoW(UINT uiAction, UINT uiParam, LPVOID lpvParam, UINT fWinIni)
{
  STUBBED();
  return FALSE;
}

BOOL QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency)
{
  STUBBED();
  return FALSE;
}

BOOL QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
{
  STUBBED();
  return FALSE;
}

DWORD GetTickCount(void)
{
  STUBBED();
  return 0;
}

BOOL GetVersionExW(LPOSVERSIONINFOW lpVersionInformation)
{
  STUBBED();
  return FALSE;
}

BOOL GlobalMemoryStatusEx(LPMEMORYSTATUSEX lpBuffer)
{
  STUBBED();
  return FALSE;
}

BOOL AllocateLocallyUniqueId(LUID *pluid)
{
  STUBBED();
  return FALSE;
}