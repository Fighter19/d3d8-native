cmake_minimum_required(VERSION 3.20)
project(wined3d8-native)

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
    #ffp_hlsl.c
    gl_compat.c
    glsl_shader.c
    palette.c
    query.c
    resource.c
    sampler.c
    shader_sm1.c
    shader_sm4.c
    shader_spirv.c
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

set(VKD3D_SOURCES
    cache.c
    command.c
    device.c
    resource.c
    state.c
    utils.c
    vkd3d_main.c
)

set(VKD3D_COMMON_SOURCES
    blob.c
    debuc.c
    error.c
    memory.c
    utf8.c
)

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

set(VKD3D_UTILS_SOURCES
    reflection.c
    vkd3d_utils_main.c
)

add_library(vkd3d-shader ${VKD3D_SHADER_SOURCES})
target_include_directories(vkd3d-shader PUBLIC vkd3d/include)
target_include_directories(vkd3d-shader PRIVATE vkd3d/include/private)
target_include_directories(vkd3d-shader PUBLIC compat/include)
target_include_directories(vkd3d-shader PUBLIC compat/include/vkd3d)
target_include_directories(vkd3d-shader PUBLIC vkd3d)
target_include_directories(vkd3d-shader PUBLIC include)
