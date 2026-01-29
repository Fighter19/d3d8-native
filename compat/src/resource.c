#include <windows.h>

DWORD SizeofResource(HMODULE hModule, HRSRC hResInfo)
{
  STUBBED();
  return 0;
}

HRSRC FindResourceA(HMODULE hModule, LPCSTR lpName, LPCSTR lpType)
{
  STUBBED();
  return NULL;
}

HRSRC LoadResource(HMODULE hModule, HRSRC hResInfo)
{
  STUBBED();
  return NULL;
}

LPVOID LockResource(HRSRC hResData)
{
  STUBBED();
  return NULL;
}

BOOL FreeResource(HRSRC hResData)
{
  STUBBED();
  return FALSE;
}