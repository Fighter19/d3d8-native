#include <windows.h>

LSTATUS RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType,
                      LPBYTE lpData, LPDWORD lpcbData)
{
  STUBBED();
  return ERROR_CALL_NOT_IMPLEMENTED;
}

LSTATUS RegCloseKey(HKEY hKey)
{
  STUBBED();
  return ERROR_CALL_NOT_IMPLEMENTED;
}

LSTATUS RegOpenKeyA(HKEY hKey, LPCSTR lpSubKey, HKEY *phkResult)
{
  STUBBED();
  return ERROR_CALL_NOT_IMPLEMENTED;
}