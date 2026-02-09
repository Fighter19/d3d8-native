#pragma once

static inline VOID NtDelayExecution(BOOL Alertable, const LARGE_INTEGER *DelayInterval)
{
  int64_t nanoseconds = -DelayInterval->QuadPart * 100;
  struct timespec ts;
  ts.tv_sec = nanoseconds / 1000000000;
  ts.tv_nsec = nanoseconds % 1000000000;
  nanosleep(&ts, NULL);
}

typedef DWORD THREAD_ID;
static inline THREAD_ID GetCurrentThreadId()
{
	pthread_t thread_id = pthread_self();
	return (THREAD_ID)thread_id;
}

#define TLS_OUT_OF_INDEXES (~0u)

static inline DWORD TlsAlloc(void)
{
  pthread_key_t key;
  int result = pthread_key_create(&key, NULL);
  if (result != 0)
    return TLS_OUT_OF_INDEXES;
  return (DWORD)key;
}

static inline BOOL TlsFree(DWORD dwTlsIndex)
{
  pthread_key_t key = (pthread_key_t)dwTlsIndex;
  int result = pthread_key_delete(key);
  return (result == 0) ? TRUE : FALSE;
}

static inline LPVOID TlsGetValue(DWORD dwTlsIndex)
{
  void *value;
  pthread_key_t key = (pthread_key_t)dwTlsIndex;
  value = pthread_getspecific(key);
  return (LPVOID)value;
}

static inline BOOL TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue)
{
  pthread_key_t key = (pthread_key_t)dwTlsIndex;
  int result = pthread_setspecific(key, lpTlsValue);
  return (result == 0) ? TRUE : FALSE;
}

static inline void SetThreadDescription(HANDLE hThread, LPCWSTR lpThreadDescription)
{
  // TODO: Find a way to use pthread_setname_np
}

static inline HANDLE GetCurrentThread()
{
  return (HANDLE)pthread_self();
}

static inline void YieldProcessor()
{
  sched_yield();
}

// typedef struct NT_TIB {
//   struct {
//     RTL_CRITICAL_SECTION *LoaderLock;
//   } Peb;
// };

// static inline struct NT_TIB* NtCurrentTeb(void)
// {
//   static struct NT_TIB tib;
//   return &tib;
// }

void DECLSPEC_NORETURN FreeLibraryAndExitThread(HMODULE hLibModule, DWORD dwExitCode);
void DisableThreadLibraryCalls(HINSTANCE hInstance);

typedef DWORD (WINAPI *LPTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);
HANDLE CreateThread(
  SECURITY_ATTRIBUTES *lpThreadAttributes,
  SIZE_T dwStackSize,
  LPTHREAD_START_ROUTINE lpStartAddress,
  LPVOID lpParameter,
  DWORD dwCreationFlags,
  LPDWORD lpThreadId
);