#include <windows.h>
#include <dlfcn.h>
#include <strings.h>
#include <SDL3/SDL.h>

typedef struct HMODULE__ {
  bool bIsSDLOpenGL;
  void *handle;
} MODULE, *HMODULE;

static MODULE s_sdlOpenGLModule = { .bIsSDLOpenGL = true };
HMODULE g_sdlOpenGLModule = &s_sdlOpenGLModule;

PROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
  if (hModule->bIsSDLOpenGL)
  {
    if (strcmp(lpProcName, "wglGetPixelFormat") == 0)
    {
      // Special case for wglGetPixelFormat
      printf("GetProcAddress: %s -> special case handler\n", lpProcName);
      // Same protoype as GetPixelFormat
      return (PROC)GetPixelFormat;
    }

    // If the string starts with "wgl", return NULL to indicate unsupported wgl extensions
    if (strncmp(lpProcName, "wgl", 3) == 0)
    {
      printf("wglGetProcAddress: %s -> NULL (unsupported wgl extension)\n", lpProcName);
      return NULL;
    }

    void *ptr = SDL_GL_GetProcAddress(lpProcName);
    // printf("GetProcAddress: %s -> %p\n", lpProcName, ptr);
    if (!ptr)
    {
      printf("GetProcAddress: %s not found in SDL OpenGL library\n", lpProcName);
      printf("SDL Error: %s\n", SDL_GetError());
    }
    return (PROC)ptr;
  }

  return (PROC)dlsym(hModule, lpProcName);
}

HMODULE GetModuleHandleA(LPCSTR lpModuleName)
{
  // Do a case compare with opengl32.dll and replace with libGLESv2.so (or libGL.so)
  if (lpModuleName && strcasecmp(lpModuleName, "opengl32.dll") == 0)
  {
    // Initialize SDL OpenGL library if not already done
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
    {
      if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
      {
        printf("GetModuleHandleA: Failed to initialize SDL video subsystem: %s\n", SDL_GetError());
        return NULL;
      }
    }

    if (!SDL_GL_LoadLibrary(NULL))
    {
      printf("GetProcAddress: Failed to load SDL OpenGL library\n");
      printf("SDL Error: %s\n", SDL_GetError());
      return NULL;
    }
    s_sdlOpenGLModule.handle = (void*)0xCAFECAFE; // Dummy non-NULL handle
    return (HMODULE)&s_sdlOpenGLModule;
  }

  STUBBED();
  // Strip DLL extension if present and replace with .so
  const char *dot = strrchr(lpModuleName, '.');
  char so_name[MAX_PATH];
  if (dot && strcasecmp(dot, ".dll") == 0)
  {
    size_t len = dot - lpModuleName;
    if (len >= sizeof(so_name) - 4)
      len = sizeof(so_name) - 5;
    memcpy(so_name, lpModuleName, len);
    strcpy(so_name + len, ".so");
    lpModuleName = so_name;
  }
  return (HMODULE)dlopen(lpModuleName, RTLD_LAZY);
}

BOOL GetModuleHandleExA(DWORD dwFlags, LPCSTR lpModuleName, HMODULE *phModule)
{
  STUBBED();
  return FALSE;
}

DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
  STUBBED();
  return -1;
}
