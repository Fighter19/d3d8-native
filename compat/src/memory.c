#include <windows.h>
#include <stdlib.h>

HANDLE GetProcessHeap(void)
{
  // Retrieves a handle to the default heap of the calling process
  STUBBED();
  return NULL;
}

LPVOID HeapCreate(DWORD flOptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize)
{
  STUBBED();
  return NULL;
}

BOOL HeapDestroy(LPVOID hHeap)
{
  STUBBED();
  return FALSE;
}

LPVOID HeapAlloc(LPVOID hHeap, DWORD dwFlags, SIZE_T dwBytes)
{
  STUBBED();
  return NULL;
}

BOOL HeapFree(LPVOID hHeap, DWORD dwFlags, LPVOID lpMem)
{
  STUBBED();
  return FALSE;
}