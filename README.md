# WineD3D-based Direct3D 8 Implementation
This project aims to provide an implementation of Direct3D 8
for Linux (and possibly other) systems via OpenGL,
in the hopes of assisting with porting games faster,
for a wider range of computer hardware.

It has been made specifically to assist porting the
"Command & Conquer Generals"-engine to a native Linux binary.
("Command & Conquer" is a trademark by Electronic Arts.
This project is neither affiliated with nor endorsed by Electronic Arts)

The original WineD3D code makes use of the Win32 API,
to enumerate video devices, modes, etc.
In order to work around this, a small compatibility layer
is implemented on top of SDL3.

This project comes with a stripped down version of WineD3D,
taken from the Wine project at version 11.0,
that has it's Vulkan capabilities stripped.

This project makes no promises on stability, correctness or performance.
It is provided as-is, to speed-up prototyping.

# Motivation
While porting games from Windows to Linux, a big chunk of code is already
written for the Direct3D 8 API.
While there exist other excellent projects, like DXVK, which also have
a native build available, those projects have a much more modern approach.
DXVK requires Vulkan, which is not available on all hardware.

This project aims to provide a more lightweight solution,
which can run on older hardware, that only supports OpenGL.

# Structure
This project consists of four main parts:
1. A small compatibility layer for Win32 API calls, implemented on top of SDL3.
    This layer is located in the `compat/` folder. (Original work)
2. A stripped down version of WineD3D, located in the `wined3d/` folder.
3. VKD3D code, located in the vkd3d/ folder, which is required by WineD3D.
4. The main Direct3D 8 implementation using WineD3D, located in the `d3d8/` folder.

# Using this library
To use this library, use the header files provided by dxvk (tested),
or this project (untested, likely not working).

Then load this library via `handle = dlopen("libd3d8-native.so", RTLD_LAZY)`.
Inside that library, the `D3D8CreateDevice` function can be found,
which can be used to obtain a Direct3D 8 device.
To do this use `dlsym(handle, "Direct3D8CreateDevice")`,
to obtain the function pointer.

After that you should be able to use the Direct3D 8 device as usual,
as long as it's used from the main thread exclusively.

This library is not expected to be linked against directly.

# Possible Questions and Answers
**Q: Why has Vulkan support been stripped?**
A:  This project has been made to run on hardware, where DXVK is not an option.
    For systems that support Vulkan, DXVK is likely a better choice.
    If you feel like WineD3D's Vulkan backend should be supported,
    contributions are welcome.

**Q: Why not use SDL2?**
A:  SDL3 is used, because it's used in the project, that motivated this project
    In order to ensure proper operation, SDL3 is expected to be used as a shared library.

**Q: How to create and pass the window?**
A:  The SDL_CreateWindow function is used to create the window,
    then this is passed to the `CreateDevice` virtual function of the Direct3D interface,
    that has been obtained prior by calling `Direct3D8CreateDevice`.