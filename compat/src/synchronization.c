#define _GNU_SOURCE
#include <windows.h>
#include <pthread.h>
#include <stdlib.h>

BOOL InitializeCriticalSectionEx(CRITICAL_SECTION *cs, DWORD spincount, DWORD flags)
{
  pthread_mutexattr_t attr;
  int res;
  res = pthread_mutexattr_init(&attr);
  if (res != 0)
    return FALSE;

  res = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  if (res != 0)
    return FALSE;

  res = pthread_mutex_init(&cs->mutex, &attr);
  if (res != 0)
    return FALSE;

  pthread_mutexattr_destroy(&attr);

  cs->DebugInfo = (RTL_CRITICAL_SECTION_DEBUG *)-1;
  if (flags & RTL_CRITICAL_SECTION_FLAG_FORCE_DEBUG_INFO)
  {
    cs->DebugInfo = (RTL_CRITICAL_SECTION_DEBUG *)malloc(sizeof(RTL_CRITICAL_SECTION_DEBUG));
    if (!cs->DebugInfo)
      return FALSE;
    memset(cs->DebugInfo, 0, sizeof(RTL_CRITICAL_SECTION_DEBUG));
  }
  return TRUE;
}

void EnterCriticalSection(CRITICAL_SECTION *cs)
{
  if (!cs)
    return;
  DWORD_PTR name = 0;
  if (cs->DebugInfo != (RTL_CRITICAL_SECTION_DEBUG *)-1)
  {
    name = cs->DebugInfo->Spare[0];
  }
  if (name)
  {
    printf("Entering critical section: %s, TID: %lu\n", (const char *)name, (unsigned long)pthread_self());
  }
  pthread_mutex_lock(&cs->mutex);
}

void LeaveCriticalSection(CRITICAL_SECTION *cs)
{
  if (!cs)
    return;
  DWORD_PTR name = 0;
  if (cs->DebugInfo != (RTL_CRITICAL_SECTION_DEBUG *)-1)
  {
    name = cs->DebugInfo->Spare[0];
  }
  if (name)
  {
    printf("Leaving critical section: %s\n", (const char *)name);
  }
  pthread_mutex_unlock(&cs->mutex);
}

void DeleteCriticalSection(CRITICAL_SECTION *cs)
{
  pthread_mutex_destroy(&cs->mutex);
}

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