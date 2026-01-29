#include <windows.h>
#include <pthread.h>
#include <stdlib.h>

HANDLE CreateEventW(SECURITY_ATTRIBUTES *lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCWSTR lpName)
{
  STUBBED();
  return NULL;
}

BOOL SetEvent(HANDLE hEvent)
{
  STUBBED();
  return FALSE;
}

DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds)
{
  return 0;
}

DWORD NtWaitForSingleObject(HANDLE Handle, BOOL Alertable, const LARGE_INTEGER *Timeout)
{
  return 0;
}

void DECLSPEC_NORETURN FreeLibraryAndExitThread(HMODULE hLibModule, DWORD dwExitCode)
{
  STUBBED();
  exit(dwExitCode);
}

void DisableThreadLibraryCalls(HINSTANCE hInstance)
{
  STUBBED();
}

HANDLE CreateThread(
  SECURITY_ATTRIBUTES *lpThreadAttributes,
  SIZE_T dwStackSize,
  LPTHREAD_START_ROUTINE lpStartAddress,
  LPVOID lpParameter,
  DWORD dwCreationFlags,
  LPDWORD lpThreadId
)
{
  STUBBED();
  pthread_t new_thread;
  int result = pthread_create(&new_thread, NULL, (void*(*)(void*))lpStartAddress, lpParameter);
  if (result != 0)
      return NULL;
  // TODO: Create closable HANDLE type
  return (HANDLE)new_thread;
}