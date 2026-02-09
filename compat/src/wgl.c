#include <windows.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_egl.h>
#include <stdlib.h>

#pragma clang optimize off

typedef enum {
  HANDLE_TYPE_DC,
  HANDLE_TYPE_GLRC,
} HHANDLE_TYPE;

typedef struct OBJECT__ {
  HHANDLE_TYPE type; 
} OBJECT, *HOBJECT;

typedef struct HDC__ {
  struct OBJECT__ obj;
  SDL_Window *window;
} DC, *HDC;

typedef struct HGLRC__ {
  struct OBJECT__ obj;
  SDL_GLContext sdlGlContext;
} GLRC, *HGLRC;


HGLRC wglCreateContext(HDC hdc)
{
  SDL_GLContext sdlGlContext = SDL_GL_CreateContext((SDL_Window *)hdc->window);
  if (sdlGlContext == NULL)
  {
    assert(false && "SDL_GL_CreateContext failed in wglCreateContext");
    fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
    return NULL;
  }

  HGLRC hglrc = (HGLRC)malloc(sizeof(GLRC));
  hglrc->obj.type = HANDLE_TYPE_GLRC;
  hglrc->sdlGlContext = sdlGlContext;
  return hglrc;
}

BOOL wglSwapBuffers(HDC hdc)
{
  if (!SDL_GL_SwapWindow((SDL_Window *)hdc->window))
  {
    printf("SDL_GL_SwapWindow failed in wglSwapBuffers: %s\n", SDL_GetError());
    assert(false && "SDL_GL_SwapWindow failed in wglSwapBuffers");
    return FALSE;
  }
  return TRUE;
}

BOOL wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
  SDL_Window *currentWindow = NULL;
  SDL_GLContext currentContext = NULL;
  if (hdc)
  {
    if (hdc->obj.type != HANDLE_TYPE_DC)
    {
      assert(false && "Invalid HDC type in wglMakeCurrent");
      return FALSE;
    }
    currentWindow = (SDL_Window *)hdc->window;
    if (!currentWindow)
    {
      assert(false && "Invalid window in HDC in wglMakeCurrent");
      return FALSE;
    }
  }

  if (hglrc)
  {
    if (hglrc->obj.type != HANDLE_TYPE_GLRC)
    {
      assert(false && "Invalid HGLRC type in wglMakeCurrent");
      return FALSE;
    }
    currentContext = hglrc->sdlGlContext;
    if (!currentContext)
    {
      assert(false && "Invalid SDL_GLContext in HGLRC in wglMakeCurrent");
      return FALSE;
    }
  }

  if (!currentWindow)
  {
    currentWindow = SDL_GL_GetCurrentWindow();
  }

  if (!currentContext)
  {
    currentContext = SDL_GL_GetCurrentContext();
  }

  if (!SDL_GL_MakeCurrent(currentWindow, currentContext))
  {
    printf("SDL_GL_MakeCurrent failed: %s\n", SDL_GetError());
    assert(false && "SDL_GL_MakeCurrent failed in wglMakeCurrent");
    return FALSE;
  }
  return TRUE;
}

HDC wglGetCurrentDC(void)
{
  // Retrieves a handle to the device context that is associated with the calling thread's current OpenGL rendering context
  assert(false && "wglGetCurrentDC is not implemented yet");

  return NULL;
}

HGLRC wglGetCurrentContext(void)
{
  // Retrieves a handle to the calling thread's current OpenGL rendering context
  assert(false && "wglGetCurrentContext is not implemented yet");

  return NULL;
}

BOOL wglDeleteContext(HGLRC hglrc)
{
  SDL_GLContext sdlGlContext = hglrc->sdlGlContext;
  SDL_GL_DestroyContext(sdlGlContext);
  free(hglrc);
  return TRUE;
}

BOOL wglShareLists(HGLRC hglrc1, HGLRC hglrc2)
{
  // Sharing of display lists between OpenGL rendering contexts is not implemented
  assert(false && "wglShareLists is not implemented yet");

  return FALSE;
}

HWND GetDesktopWindow(void)
{
  // TODO: IIRC, this usually returns a constant handle to the desktop window
  return (HWND)(-1);
}

HWND WindowFromDC(HDC dc)
{
  if (!dc)
  {
    assert(false && "Invalid HDC in WindowFromDC");
    return NULL;
  }
  return (HWND)(dc->window);
}

HDC GetDC(HWND hWnd)
{
  if (!hWnd)
  {
    assert(false && "Invalid HWND in GetDC");
    return NULL;
  }

  HDC hdc = (HDC)malloc(sizeof(DC));
  hdc->obj.type = HANDLE_TYPE_DC;
  hdc->window = (SDL_Window *)hWnd;

  return hdc;
}

HDC GetDCEx(HWND hWnd, HRGN hrgnClip, DWORD flags)
{
  // Theoretically, the hrgnClip and flags parameters can be used to control the clipping region of the device context
  // We don't do that here for simplicity
  return GetDC(hWnd);
}

HWND CreateWindowA(
  LPCSTR               lpClassName,
  LPCSTR               lpWindowName,
  DWORD                dwStyle,
  int                  X,
  int                  Y,
  int                  nWidth,
  int                  nHeight,
  HWND                 hWndParent,
  HANDLE               hMenu,
  HINSTANCE            hInstance,
  LPVOID               lpParam
)
{
  SDL_Window *window = SDL_CreateWindow(lpWindowName,
                                        nWidth,
                                        nHeight,
                                        SDL_WINDOW_OPENGL);
  if (!window)
  {
    assert(false && "SDL_CreateWindow failed in CreateWindowA");
    return NULL;
  }
  return (HWND)window;
}

BOOL GetClientRect(HWND hWnd, LPRECT lpRect)
{
  if (!hWnd || !lpRect)
  {
    assert(false && "Invalid parameters in GetClientRect");
    return FALSE;
  }

  SDL_Window *window = (SDL_Window *)hWnd;
  int w, h;
  SDL_GetWindowSize(window, &w, &h);

  lpRect->left = 0;
  lpRect->top = 0;
  lpRect->right = w;
  lpRect->bottom = h;

  return TRUE;
}

void DestroyWindow(HWND hWnd)
{
  if (!hWnd)
  {
    assert(false && "Invalid HWND in DestroyWindow");
    return;
  }
  SDL_Window *window = (SDL_Window *)hWnd;
  SDL_DestroyWindow(window);
}

INT ReleaseDC(HWND hWnd, HDC hDC)
{
  // Releases a device context (DC), freeing it for use by other applications
  if (!hDC)
  {
    assert(false && "Invalid HDC in ReleaseDC");
    return FALSE;
  }
  if (hDC->obj.type != HANDLE_TYPE_DC)
  {
    assert(false && "Invalid HDC type in ReleaseDC");
    return FALSE;
  }
  if (hWnd != (HWND)hDC->window)
  {
    assert(false && "HDC does not belong to the specified HWND in ReleaseDC");
    return FALSE;
  }
  free(hDC);
  return TRUE;
}

int ChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd)
{
  if (ppfd->iPixelType != PFD_TYPE_RGBA)
  {
    assert(false && "Only PFD_TYPE_RGBA is supported in ChoosePixelFormat");
    return 0;
  }

  if (ppfd->cColorBits != 32)
  {
    assert(false && "Only 32-bit color is supported in ChoosePixelFormat");
    return 0;
  }

  return 1;
}

int GetPixelFormat(HDC hdc)
{
  if (hdc == NULL)
  {
    assert(false && "Invalid HDC in GetPixelFormat");
    return 0;
  }
  return 1;
}

int SetPixelFormat(HDC hdc, int format, const PIXELFORMATDESCRIPTOR *ppfd)
{
  if (hdc == NULL)
  {
    assert(false && "Invalid HDC in SetPixelFormat");
    return 0;
  }

  if (format != 1)
  {
    assert(false && "Only pixel format 1 is supported in SetPixelFormat");
    return 0;
  }

  return 1;
}

int DescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, PIXELFORMATDESCRIPTOR *ppfd)
{
  if (iPixelFormat == 0)
  {
    // Returns the count of pixel formats
    return 1;
  }

  if (iPixelFormat != 1)
  {
    assert(false && "Only pixel format 1 is supported in DescribePixelFormat");
    return 0;
  }

  if (nBytes < sizeof(PIXELFORMATDESCRIPTOR))
  {
    assert(false && "nBytes is too small in DescribePixelFormat");
    return 0;
  }

  // TODO: Fill out ppfd based on actual pixel format
  ZeroMemory(ppfd, nBytes);
  ppfd->nSize = sizeof(PIXELFORMATDESCRIPTOR);
  ppfd->nVersion = 1;
  ppfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  ppfd->iPixelType = PFD_TYPE_RGBA;
  ppfd->cColorBits = 32;
  ppfd->cDepthBits = 24;
  ppfd->cStencilBits = 8;
  ppfd->cAuxBuffers = 0;
  ppfd->iLayerType = PFD_MAIN_PLANE;
  return 1;
}

extern HMODULE g_sdlOpenGLModule;

PROC wglGetProcAddress(const char *lpszProc)
{
  return GetProcAddress(g_sdlOpenGLModule, lpszProc);
}