#include <windows.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_egl.h>


HGLRC wglCreateContext(HDC hdc)
{
  EGLDisplay disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  EGLContext ctx = eglCreateContext(disp, NULL, EGL_NO_CONTEXT, NULL);
  if (ctx == EGL_NO_CONTEXT)
  {
    assert(false && "eglCreateContext failed in wglCreateContext");
    return NULL;
  }
  return (HGLRC)ctx;
}

BOOL wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
  // Sets the associated OpenGL rendering context to the device context
  assert(false && "wglMakeCurrent is not implemented yet");

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
  // Deletes the specified OpenGL rendering context
  assert(false && "wglDeleteContext is not implemented yet");

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
  // This function will return the SDL_Window* later
  // Retrieves a handle to the window associated with the specified device context (DC)
  assert(false && "WindowFromDC is not implemented yet");

  return NULL;
}

HDC GetDC(HWND hWnd)
{
  // Retrieves a handle to a device context (DC) for the client area of a specified window or for the entire screen
  assert(false && "GetDC is not implemented yet");

  return NULL;
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
                                        X,
                                        Y,
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
  // Destroys the specified window
  assert(false && "DestroyWindow is not implemented yet");
}

INT ReleaseDC(HWND hWnd, HDC hDC)
{
  // Releases a device context (DC), freeing it for use by other applications
  assert(false && "ReleaseDC is not implemented yet");
  return 0;
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

PROC wglGetProcAddress(const char *lpszProc)
{
  return (PROC)eglGetProcAddress(lpszProc);
}
