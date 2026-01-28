#pragma once

#include <sys/types.h>

// Using builtin atomics, because C11 would require annotating every type with _Atomic
static inline LONG InterlockedIncrement(volatile LONG *lpAddend)
{
  return __atomic_add_fetch(lpAddend, 1, __ATOMIC_SEQ_CST);
}

static inline LONG InterlockedDecrement(volatile LONG *lpAddend)
{
  return __atomic_sub_fetch(lpAddend, 1, __ATOMIC_SEQ_CST);
}

static inline LONG InterlockedExchange(volatile LONG *target, LONG value)
{
  return __atomic_exchange_n(target, value, __ATOMIC_SEQ_CST);
}

static inline LONG InterlockedCompareExchange(volatile LONG *destination, LONG exchange, LONG comperand)
{
  return __atomic_compare_exchange_n(destination, &comperand, exchange, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) ? comperand : *destination;
}

static inline LPVOID InterlockedCompareExchangePointer(volatile LPVOID *destination, LPVOID exchange, LPVOID comperand)
{
  return __atomic_compare_exchange_n(destination, &comperand, exchange, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) ? comperand : *destination;
}

static inline LPVOID InterlockedExchangePointer(volatile LPVOID *target, LPVOID value)
{
  return __atomic_exchange_n(target, value, __ATOMIC_SEQ_CST);
}

static inline ssize_t InterlockedExchangeAddSizeT(volatile ssize_t *addend, ssize_t value)
{
  return __atomic_fetch_add(addend, value, __ATOMIC_SEQ_CST);
}