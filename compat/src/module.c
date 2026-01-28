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