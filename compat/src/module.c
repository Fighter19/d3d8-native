#include <windows.h>
#include <dlfcn.h>

PROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
  return (PROC)dlsym(hModule, lpProcName);
}

HMODULE GetModuleHandleA(LPCSTR lpModuleName)
{
  return (HMODULE)dlopen(lpModuleName, RTLD_LAZY);
}

BOOL GetModuleHandleExA(DWORD dwFlags, LPCSTR lpModuleName, HMODULE *phModule)
{
  STUBBED();
  return FALSE;
}

DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
  STUBBED();
  return -1;
}
