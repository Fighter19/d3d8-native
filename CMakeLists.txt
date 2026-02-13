cmake_minimum_required(VERSION 3.20)
project(wined3d8-native)

option(D3D8_USE_GL4ES "Use GL4ES for OpenGL ES emulation" ON)

if(D3D8_USE_GL4ES)
    if (EMSCRIPTEN)
        # Don't cache these options, in Emscripten's case
        cmake_policy(SET CMP0077 NEW)
        message(STATUS "Setting Emscripten defaults for GL4ES")
        set(NOX11 ON)
        set(NOEGL ON)
        set(STATICLIB ON)
    endif()
    FetchContent_Declare(
        gl4es
        GIT_REPOSITORY https://github.com/ptitSeb/gl4es.git
        GIT_TAG        1d69dd6a9867f8c2a9592e035ba00090f74071d7)
    FetchContent_MakeAvailable(gl4es)
endif()

set(WINED3D8_SOURCES
    buffer.c
    d3d8_main.c
    device.c
    directx.c
    shader.c
    surface.c
    swapchain.c
    texture.c
    vertexdeclaration.c
    volume.c
)

set(WINED3D_SOURCES
    adapter_gl.c
    #adapter_vk.c
    buffer.c
    context_gl.c
    context.c
    cs.c
    decoder.c
    device.c
    directx.c
    ffp_gl.c
    ffp_hlsl.c
    gl_compat.c
    glsl_shader.c
    palette.c
    query.c
    resource.c
    sampler.c
    shader_sm1.c
    shader_sm4.c
    #shader_spirv.c
    shader.c
    stateblock.c
    surface.c
    swapchain.c
    texture_gl.c
    #texture_vk.c
    texture.c
    utils.c
    vertexdeclaration.c
    view.c
    wined3d_main.c
)

list(TRANSFORM WINED3D_SOURCES PREPEND wined3d/)

add_library(wined3d SHARED ${WINED3D_SOURCES})

set(COMPAT_SOURCES
    memory.c
    messages.c
    module.c
    registry.c
    resource.c
    synchronization.c
    system.c
    wgl.c
    windowing.c
)

list(TRANSFORM COMPAT_SOURCES PREPEND compat/src/)

add_library(compat SHARED ${COMPAT_SOURCES})
find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(compat PRIVATE SDL3::SDL3 EGL)
if(D3D8_USE_GL4ES)
    # GL is defined by GL4ES, if not using GL4ES, pick system GLES instead
    target_link_libraries(compat PRIVATE GL)
    target_compile_definitions(compat PRIVATE -DGL4ES)
else()
    target_link_libraries(compat PRIVATE GLESv2)
endif()

target_include_directories(compat PUBLIC include compat/include)

set(VKD3D_SHADER_SOURCES
    checksum.c
    d3d_asm.c
    d3dbc.c
    dxbc.c
    dxil.c
    fx.c
    glsl.c
    hlsl_codegen.c
    hlsl_constant_ops.c
    hlsl.c
    ir.c
    msl.c
    spirv.c
    tpf.c
    vkd3d_shader_main.c
)

list(TRANSFORM VKD3D_SHADER_SOURCES PREPEND vkd3d/libs/vkd3d-shader/)

# Append vkd3d_compat/vkd3d.c to WINED3D_SOURCES
list(APPEND VKD3D_SHADER_SOURCES vkd3d_compat/vkd3d.c vkd3d/libs/vkd3d-common/memory.c)

set(VKD3D_UTILS_SOURCES
    reflection.c
    vkd3d_utils_main.c
)

# Add Bison and Flex rules for hlsl.y and hlsl.l
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(HLSLParser vkd3d/libs/vkd3d-shader/hlsl.y ${CMAKE_CURRENT_BINARY_DIR}/hlsl.tab.c)
FLEX_TARGET(HLSLLexer vkd3d/libs/vkd3d-shader/hlsl.l ${CMAKE_CURRENT_BINARY_DIR}/hlsl.yy.c)
ADD_FLEX_BISON_DEPENDENCY(HLSLLexer HLSLParser)

# Add Bison and Flex rules for preproc.y and preproc.l
BISON_TARGET(PreprocParser vkd3d/libs/vkd3d-shader/preproc.y ${CMAKE_CURRENT_BINARY_DIR}/preproc.tab.c)
FLEX_TARGET(PreprocLexer vkd3d/libs/vkd3d-shader/preproc.l ${CMAKE_CURRENT_BINARY_DIR}/preproc.yy.c)
ADD_FLEX_BISON_DEPENDENCY(PreprocLexer PreprocParser)

# Include the generated files in the VKD3D_SHADER_SOURCES
list(APPEND VKD3D_SHADER_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/hlsl.tab.c
    ${CMAKE_CURRENT_BINARY_DIR}/hlsl.yy.c
    ${CMAKE_CURRENT_BINARY_DIR}/preproc.tab.c
    ${CMAKE_CURRENT_BINARY_DIR}/preproc.yy.c
)

add_library(vkd3d-shader SHARED ${VKD3D_SHADER_SOURCES})
# Ensure the binary directory is included for generated headers
target_include_directories(vkd3d-shader PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(vkd3d-shader PRIVATE vkd3d/libs/vkd3d-shader)
target_include_directories(vkd3d-shader PUBLIC vkd3d/include)
target_include_directories(vkd3d-shader PRIVATE vkd3d/include/private)
target_include_directories(vkd3d-shader PUBLIC compat/include/vkd3d)
target_include_directories(vkd3d-shader PUBLIC vkd3d)
target_link_libraries(vkd3d-shader PRIVATE compat)

target_link_libraries(wined3d PRIVATE vkd3d-shader compat)

set(D3D8_NATIVE_SOURCES
    buffer.c
    d3d8_main.c
    device.c
    directx.c
    shader.c
    surface.c
    swapchain.c
    texture.c
    vertexdeclaration.c
    volume.c
)

list(TRANSFORM D3D8_NATIVE_SOURCES PREPEND d3d8/)
add_library(d3d8-native SHARED ${D3D8_NATIVE_SOURCES})
target_link_libraries(d3d8-native PRIVATE wined3d compat)