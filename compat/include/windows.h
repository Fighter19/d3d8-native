#pragma once
#include <stddef.h>
#include <string.h>
#include <stdint.h>
typedef wchar_t WCHAR;
typedef int HRESULT;
typedef int INT;

typedef int32_t INT;
typedef uint32_t UINT;

typedef size_t SIZE_T;

typedef int32_t LONG;
typedef uint32_t ULONG;
typedef uint32_t DWORD;
typedef unsigned char BYTE;
typedef char CHAR;
typedef char* PCHAR;
typedef unsigned char UCHAR;
typedef BYTE byte;
typedef uint16_t WORD;
typedef uint16_t USHORT;

typedef float FLOAT;

typedef wchar_t* LPWSTR;
typedef const wchar_t* LPCWSTR;
typedef int64_t LONGLONG;
typedef uint64_t ULONGLONG;

typedef DWORD LCID;
typedef int BOOL;

typedef DWORD *LPDWORD;

typedef void* LPVOID;
typedef void* HANDLE;
typedef HANDLE* LPHANDLE;

#define DECLARE_HANDLE(name) typedef struct name##__ { int unused; } *name;
DECLARE_HANDLE(HGLOBAL)
DECLARE_HANDLE(HTASK)
DECLARE_HANDLE(HBITMAP)
DECLARE_HANDLE(HICON)
DECLARE_HANDLE(HDC)
DECLARE_HANDLE(HENHMETAFILE)
DECLARE_HANDLE(HWND)
DECLARE_HANDLE(HINSTANCE)
DECLARE_HANDLE(HMONITOR)
DECLARE_HANDLE(HMODULE)

typedef struct LUID {
  DWORD LowPart;
  LONG  HighPart;
} LUID;

typedef struct POINT {
  LONG x;
  LONG y;
} POINT;

typedef POINT* LPPOINT;

typedef struct RECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT,*PRECT,*NPRECT,*LPRECT;

typedef union {
  struct {
    DWORD LowPart;
    LONG HighPart;
  };

  struct {
    DWORD LowPart;
    LONG HighPart;
  } u;

  LONGLONG QuadPart;
} LARGE_INTEGER;

typedef union {
  struct {
    DWORD LowPart;
    DWORD HighPart;
  };

  struct {
    DWORD LowPart;
    DWORD HighPart;
  } u;

  ULONGLONG QuadPart;
} ULARGE_INTEGER;

typedef struct RGNDATAHEADER {
  DWORD dwSize;
  DWORD iType;
  DWORD nCount;
  DWORD nRgnSize;
  RECT  rcBound;
} RGNDATAHEADER;

typedef struct RGNDATA {
  RGNDATAHEADER rdh;
  char          Buffer[1];
} RGNDATA,*PRGNDATA,*NPRGNDATA,*LPRGNDATA;

typedef struct _GLYPHMETRICSFLOAT GLYPHMETRICSFLOAT;

// TODO: I have no idea what this is
typedef void* hyper;

typedef struct SECURITY_DESCRIPTOR *PSECURITY_DESCRIPTOR;
typedef struct _RPC_AUTHZ_HANDLE RPC_AUTHZ_HANDLE;

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif


// Defined in guiddef.h
// typedef struct _GUID
// {
//     unsigned long  Data1;
//     unsigned short Data2;
//     unsigned short Data3;
//     unsigned char  Data4[8];
// } GUID;
#include "guiddef.h"

#define S_OK ((HRESULT)0L)
#define S_FALSE ((HRESULT)1L)
#define E_NOTIMPL ((HRESULT)0x80004001L)
#define E_NOINTERFACE ((HRESULT)0x80004002L)
#define E_POINTER ((HRESULT)0x80004003L)
#define E_ABORT ((HRESULT)0x80004004L)
#define E_FAIL ((HRESULT)0x80004005L)
#define E_OUTOFMEMORY ((HRESULT)0x8007000EL)
#define E_INVALIDARG ((HRESULT)0x80070057L)
#define DXGI_ERROR_ALREADY_EXISTS ((HRESULT)0x887A0036L)
#define DXGI_ERROR_NOT_FOUND ((HRESULT)0x887A0002L)
#define DXGI_ERROR_MORE_DATA ((HRESULT)0x887A0003L)
#define DXGI_ERROR_UNSUPPORTED ((HRESULT)0x887A0004L)

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define DECL_WINELIB_TYPE_AW(x)
#define DECLSPEC_IMPORT
#define DECLSPEC_EXPORT
#define DECLSPEC_NORETURN [[noreturn]]

#define CONST const
#define CONST_VTBL const

#define TRUE 1
#define FALSE 0

#define interface struct

// There's no stdcall on non-Windows platforms
#define __stdcall
#define STDMETHODCALLTYPE
#define CALLBACK
#define WINAPI
#define STDAPI HRESULT STDMETHODCALLTYPE

#define MAKE_HRESULT(sev,fac,code) \
    ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )

typedef struct IAgileReference IAgileReference;

// My guess is that these are for annotating memory allocation functions for sanitizers
#define __WINE_ALLOC_SIZE(x)
#define __WINE_DEALLOC(x)
#define __WINE_MALLOC

#ifdef NONAMELESSUNION
#define DUMMYSTRUCTNAME s
#define DUMMYUNIONNAME u
#endif

#define BEGIN_INTERFACE
#define END_INTERFACE

#define LF_FACESIZE 32
#define CONTAINING_RECORD(address, type, field) \
  ((type *)((PCHAR)(address) - offsetof(type, field)))