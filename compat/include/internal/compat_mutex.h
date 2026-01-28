#pragma once

typedef struct _LIST_ENTRY {
  struct _LIST_ENTRY *Flink;
  struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY;

typedef struct _RTL_CRITICAL_SECTION_DEBUG
{
  WORD   Type;
  WORD   CreatorBackTraceIndex;
  struct _RTL_CRITICAL_SECTION *CriticalSection;
  LIST_ENTRY ProcessLocksList;
  DWORD EntryCount;
  DWORD ContentionCount;
  DWORD_PTR Spare[8/sizeof(DWORD_PTR)];
} RTL_CRITICAL_SECTION_DEBUG, *PRTL_CRITICAL_SECTION_DEBUG, RTL_RESOURCE_DEBUG, *PRTL_RESOURCE_DEBUG;

typedef struct _RTL_CRITICAL_SECTION {
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;
    HANDLE LockSemaphore;
    ULONG_PTR SpinCount;
    // End of original CS
    pthread_mutex_t mutex;
}  RTL_CRITICAL_SECTION, *PRTL_CRITICAL_SECTION;

typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;
typedef RTL_CRITICAL_SECTION_DEBUG CRITICAL_SECTION_DEBUG;

#define RTL_CRITICAL_SECTION_FLAG_FORCE_DEBUG_INFO 0x01000000

static BOOL InitializeCriticalSectionEx(CRITICAL_SECTION *cs, DWORD spincount, DWORD flags)
{
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutex_init(&cs->mutex, &attr);
  pthread_mutexattr_destroy(&attr);

  cs->DebugInfo = (RTL_CRITICAL_SECTION_DEBUG *)-1;
  return TRUE;
}

static inline void EnterCriticalSection(CRITICAL_SECTION *cs)
{
  pthread_mutex_lock(&cs->mutex);
}

static inline void LeaveCriticalSection(CRITICAL_SECTION *cs)
{
  pthread_mutex_unlock(&cs->mutex);
}

static inline void DeleteCriticalSection(CRITICAL_SECTION *cs)
{
  pthread_mutex_destroy(&cs->mutex);
}

BOOL SetEvent(HANDLE hEvent);
DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
DWORD NtWaitForSingleObject(HANDLE Handle, BOOL Alertable, const LARGE_INTEGER *Timeout);
//BOOL RtlIsCriticalSectionLockedByThread(RTL_CRITICAL_SECTION *pCriticalSection);

// This define is used to check if the current thread allows loading of modules
#define RtlIsCriticalSectionLockedByThread(pCriticalSection) FALSE

HANDLE CreateEventW(SECURITY_ATTRIBUTES *lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCWSTR lpName);
#define WAIT_OBJECT_0 0