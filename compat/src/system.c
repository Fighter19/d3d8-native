#include <windows.h>
#include <time.h>

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
  lpFrequency->QuadPart = 1000; // 1ms resolution
  return TRUE;
}

BOOL QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
  {
    return FALSE;
  }
  lpPerformanceCount->QuadPart = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
  return TRUE;
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
  // TODO: Determine if this is actually required
  pluid->LowPart = 0xDEADBEEF;
  pluid->HighPart = 0xC0FEBABE;
  return TRUE;
}