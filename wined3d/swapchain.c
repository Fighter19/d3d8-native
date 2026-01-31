/*
 * Copyright 2002-2003 Jason Edmeades
 * Copyright 2002-2003 Raphael Junqueira
 * Copyright 2005 Oliver Stieber
 * Copyright 2007-2008 Stefan Dösinger for CodeWeavers
 * Copyright 2011 Henri Verbeet for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "wined3d_private.h"
#include "wined3d_gl.h"
#include "wined3d_vk.h"

WINE_DEFAULT_DEBUG_CHANNEL(d3d);
WINE_DECLARE_DEBUG_CHANNEL(d3d_perf);

static BOOL set_window_present_rect(HWND hwnd, UINT x, UINT y, UINT width, UINT height)
{
    RECT rect = {x, y, x + width, y + height};
    D3DKMT_ESCAPE escape = {0};

    escape.Type = D3DKMT_ESCAPE_SET_PRESENT_RECT_WINE;
    escape.hContext = HandleToULong(hwnd);
    escape.pPrivateDriverData = &rect;
    escape.PrivateDriverDataSize = sizeof(rect);

    return !D3DKMTEscape(&escape);
}

void wined3d_swapchain_cleanup(struct wined3d_swapchain *swapchain)
{
    HRESULT hr;
    UINT i;

    TRACE("Destroying swapchain %p.\n", swapchain);

    wined3d_swapchain_state_cleanup(&swapchain->state);
    wined3d_swapchain_set_gamma_ramp(swapchain, 0, &swapchain->orig_gamma);

    /* Release the swapchain's draw buffers. Make sure swapchain->back_buffers[0]
     * is the last buffer to be destroyed, FindContext() depends on that. */
    if (swapchain->front_buffer)
    {
        wined3d_texture_set_swapchain(swapchain->front_buffer, NULL);
        if (wined3d_texture_decref(swapchain->front_buffer))
            WARN("Something's still holding the front buffer (%p).\n", swapchain->front_buffer);
        swapchain->front_buffer = NULL;
    }

    if (swapchain->back_buffers)
    {
        i = swapchain->state.desc.backbuffer_count;

        while (i--)
        {
            wined3d_texture_set_swapchain(swapchain->back_buffers[i], NULL);
            if (wined3d_texture_decref(swapchain->back_buffers[i]))
                WARN("Something's still holding back buffer %u (%p).\n", i, swapchain->back_buffers[i]);
        }
        free(swapchain->back_buffers);
        swapchain->back_buffers = NULL;
    }

    /* Restore the screen resolution if we rendered in fullscreen.
     * This will restore the screen resolution to what it was before creating
     * the swapchain. In case of d3d8 and d3d9 this will be the original
     * desktop resolution. In case of d3d7 this will be a NOP because ddraw
     * sets the resolution before starting up Direct3D, thus orig_width and
     * orig_height will be equal to the modes in the presentation params. */
    if (!swapchain->state.desc.windowed)
    {
        if (swapchain->state.desc.auto_restore_display_mode)
        {
            if (FAILED(hr = wined3d_restore_display_modes(swapchain->device->wined3d)))
                ERR("Failed to restore display mode, hr %#lx.\n", hr);

            if (swapchain->state.desc.flags & WINED3D_SWAPCHAIN_RESTORE_WINDOW_RECT)
            {
                wined3d_swapchain_state_restore_from_fullscreen(&swapchain->state,
                        swapchain->state.device_window, &swapchain->state.original_window_rect);
                wined3d_device_release_focus_window(swapchain->device);
            }
        }
        else
        {
            wined3d_swapchain_state_restore_from_fullscreen(&swapchain->state, swapchain->state.device_window, NULL);
        }
    }
}

void wined3d_swapchain_gl_cleanup(struct wined3d_swapchain_gl *swapchain_gl)
{
    wined3d_swapchain_cleanup(&swapchain_gl->s);
}

ULONG CDECL wined3d_swapchain_incref(struct wined3d_swapchain *swapchain)
{
    unsigned int refcount = InterlockedIncrement(&swapchain->ref);

    TRACE("%p increasing refcount to %u.\n", swapchain, refcount);

    return refcount;
}

ULONG CDECL wined3d_swapchain_decref(struct wined3d_swapchain *swapchain)
{
    unsigned int refcount = InterlockedDecrement(&swapchain->ref);

    TRACE("%p decreasing refcount to %u.\n", swapchain, refcount);

    if (!refcount)
    {
        struct wined3d_device *device;

        wined3d_mutex_lock();

        device = swapchain->device;
        if (device->swapchain_count && device->swapchains[0] == swapchain)
            wined3d_device_uninit_3d(device);
        wined3d_cs_finish(device->cs, WINED3D_CS_QUEUE_DEFAULT);

        if (swapchain->dc)
            wined3d_release_dc(swapchain->win_handle, swapchain->dc);

        swapchain->parent_ops->wined3d_object_destroyed(swapchain->parent);
        swapchain->device->adapter->adapter_ops->adapter_destroy_swapchain(swapchain);

        wined3d_mutex_unlock();
    }

    return refcount;
}

void * CDECL wined3d_swapchain_get_parent(const struct wined3d_swapchain *swapchain)
{
    TRACE("swapchain %p.\n", swapchain);

    return swapchain->parent;
}

void CDECL wined3d_swapchain_set_window(struct wined3d_swapchain *swapchain, HWND window)
{
    if (!window)
        window = swapchain->state.device_window;
    if (window == swapchain->win_handle)
        return;

    TRACE("Setting swapchain %p window from %p to %p.\n",
            swapchain, swapchain->win_handle, window);

    wined3d_cs_finish(swapchain->device->cs, WINED3D_CS_QUEUE_DEFAULT);

    if (swapchain->dc)
        wined3d_release_dc(swapchain->win_handle, swapchain->dc);

    swapchain->win_handle = window;

    if (!(swapchain->dc = GetDCEx(swapchain->win_handle, 0, DCX_USESTYLE | DCX_CACHE)))
        WARN("Failed to retrieve device context, trying swapchain backup.\n");
}

HRESULT CDECL wined3d_swapchain_present(struct wined3d_swapchain *swapchain,
        const RECT *src_rect, const RECT *dst_rect, HWND dst_window_override,
        unsigned int swap_interval, uint32_t flags)
{
    const struct wined3d_swapchain_desc *desc = &swapchain->state.desc;
    RECT s, d;

    TRACE("swapchain %p, src_rect %s, dst_rect %s, dst_window_override %p, swap_interval %u, flags %#x.\n",
            swapchain, wine_dbgstr_rect(src_rect), wine_dbgstr_rect(dst_rect),
            dst_window_override, swap_interval, flags);

    if (flags)
        FIXME("Ignoring flags %#x.\n", flags);

    wined3d_mutex_lock();

    if (!swapchain->back_buffers)
    {
        WARN("Swapchain doesn't have a backbuffer, returning WINED3DERR_INVALIDCALL.\n");
        wined3d_mutex_unlock();
        return WINED3DERR_INVALIDCALL;
    }

    if (!src_rect)
    {
        SetRect(&s, 0, 0, desc->backbuffer_width, desc->backbuffer_height);
        src_rect = &s;
    }

    if (!dst_rect)
    {
        if (!desc->windowed)
            SetRect(&d, 0, 0, desc->backbuffer_width, desc->backbuffer_height);
        else
            GetClientRect(swapchain->win_handle, &d);
        dst_rect = &d;
    }

    wined3d_cs_emit_present(swapchain->device->cs, swapchain, src_rect,
            dst_rect, dst_window_override, swap_interval, flags);

    wined3d_mutex_unlock();

    return WINED3D_OK;
}

HRESULT CDECL wined3d_swapchain_get_front_buffer_data(const struct wined3d_swapchain *swapchain,
        struct wined3d_texture *dst_texture, unsigned int sub_resource_idx)
{
    RECT src_rect, dst_rect;

    TRACE("swapchain %p, dst_texture %p, sub_resource_idx %u.\n", swapchain, dst_texture, sub_resource_idx);

    SetRect(&src_rect, 0, 0, swapchain->front_buffer->resource.width, swapchain->front_buffer->resource.height);
    dst_rect = src_rect;

    if (swapchain->state.desc.windowed)
    {
        MapWindowPoints(swapchain->win_handle, NULL, (POINT *)&dst_rect, 2);
        FIXME("Using destination rect %s in windowed mode, this is likely wrong.\n",
                wine_dbgstr_rect(&dst_rect));
    }

    return wined3d_device_context_blt(&swapchain->device->cs->c, dst_texture, sub_resource_idx, &dst_rect,
            swapchain->front_buffer, 0, &src_rect, 0, NULL, WINED3D_TEXF_POINT);
}

struct wined3d_texture * CDECL wined3d_swapchain_get_back_buffer(const struct wined3d_swapchain *swapchain,
        UINT back_buffer_idx)
{
    TRACE("swapchain %p, back_buffer_idx %u.\n",
            swapchain, back_buffer_idx);

    /* Return invalid if there is no backbuffer array, otherwise it will
     * crash when ddraw is used (there swapchain->back_buffers is always
     * NULL). We need this because this function is called from
     * stateblock_init_default_state() to get the default scissorrect
     * dimensions. */
    if (!swapchain->back_buffers || back_buffer_idx >= swapchain->state.desc.backbuffer_count)
    {
        WARN("Invalid back buffer index.\n");
        /* Native d3d9 doesn't set NULL here, just as wine's d3d9. But set it
         * here in wined3d to avoid problems in other libs. */
        return NULL;
    }

    TRACE("Returning back buffer %p.\n", swapchain->back_buffers[back_buffer_idx]);

    return swapchain->back_buffers[back_buffer_idx];
}

struct wined3d_texture * CDECL wined3d_swapchain_get_front_buffer(const struct wined3d_swapchain *swapchain)
{
    TRACE("swapchain %p.\n", swapchain);

    return swapchain->front_buffer;
}

struct wined3d_output * wined3d_swapchain_get_output(const struct wined3d_swapchain *swapchain)
{
    TRACE("swapchain %p.\n", swapchain);

    return swapchain->state.desc.output;
}

HRESULT CDECL wined3d_swapchain_get_raster_status(const struct wined3d_swapchain *swapchain,
        struct wined3d_raster_status *raster_status)
{
    struct wined3d_output *output;

    TRACE("swapchain %p, raster_status %p.\n", swapchain, raster_status);

    output = wined3d_swapchain_get_output(swapchain);
    if (!output)
    {
        ERR("Failed to get output from swapchain %p.\n", swapchain);
        return E_FAIL;
    }

    return wined3d_output_get_raster_status(output, raster_status);
}

struct wined3d_swapchain_state * CDECL wined3d_swapchain_get_state(struct wined3d_swapchain *swapchain)
{
    return &swapchain->state;
}

HRESULT CDECL wined3d_swapchain_get_display_mode(const struct wined3d_swapchain *swapchain,
        struct wined3d_display_mode *mode, enum wined3d_display_rotation *rotation)
{
    struct wined3d_output *output;
    HRESULT hr;

    TRACE("swapchain %p, mode %p, rotation %p.\n", swapchain, mode, rotation);

    if (!(output = wined3d_swapchain_get_output(swapchain)))
    {
        ERR("Failed to get output from swapchain %p.\n", swapchain);
        return E_FAIL;
    }

    hr = wined3d_output_get_display_mode(output, mode, rotation);

    TRACE("Returning w %u, h %u, refresh rate %u, format %s.\n",
            mode->width, mode->height, mode->refresh_rate, debug_d3dformat(mode->format_id));

    return hr;
}

struct wined3d_device * CDECL wined3d_swapchain_get_device(const struct wined3d_swapchain *swapchain)
{
    TRACE("swapchain %p.\n", swapchain);

    return swapchain->device;
}

void CDECL wined3d_swapchain_get_desc(const struct wined3d_swapchain *swapchain,
        struct wined3d_swapchain_desc *desc)
{
    TRACE("swapchain %p, desc %p.\n", swapchain, desc);

    *desc = swapchain->state.desc;
}

HRESULT CDECL wined3d_swapchain_set_gamma_ramp(const struct wined3d_swapchain *swapchain,
        uint32_t flags, const struct wined3d_gamma_ramp *ramp)
{
    struct wined3d_output *output;

    TRACE("swapchain %p, flags %#x, ramp %p.\n", swapchain, flags, ramp);

    if (flags)
        FIXME("Ignoring flags %#x.\n", flags);

    if (!(output = wined3d_swapchain_get_output(swapchain)))
    {
        ERR("Failed to get output from swapchain %p.\n", swapchain);
        return E_FAIL;
    }

    return wined3d_output_set_gamma_ramp(output, ramp);
}

void CDECL wined3d_swapchain_set_palette(struct wined3d_swapchain *swapchain, struct wined3d_palette *palette)
{
    TRACE("swapchain %p, palette %p.\n", swapchain, palette);

    wined3d_cs_finish(swapchain->device->cs, WINED3D_CS_QUEUE_DEFAULT);

    swapchain->palette = palette;
}

HRESULT CDECL wined3d_swapchain_get_gamma_ramp(const struct wined3d_swapchain *swapchain,
        struct wined3d_gamma_ramp *ramp)
{
    struct wined3d_output *output;

    TRACE("swapchain %p, ramp %p.\n", swapchain, ramp);

    if (!(output = wined3d_swapchain_get_output(swapchain)))
    {
        ERR("Failed to get output from swapchain %p.\n", swapchain);
        return E_FAIL;
    }

    return wined3d_output_get_gamma_ramp(output, ramp);
}

/* The is a fallback for cases where we e.g. can't create a GL context or
 * Vulkan swapchain for the swapchain window. */
static void swapchain_blit_gdi(struct wined3d_swapchain *swapchain,
        struct wined3d_context *context, const RECT *src_rect, const RECT *dst_rect)
{
    struct wined3d_texture *back_buffer = swapchain->back_buffers[0];
    D3DKMT_DESTROYDCFROMMEMORY destroy_desc;
    D3DKMT_CREATEDCFROMMEMORY create_desc;
    const struct wined3d_format *format;
    unsigned int row_pitch, slice_pitch;
    NTSTATUS status;
    HBITMAP bitmap;
    HDC src_dc;

    static unsigned int once;

    TRACE("swapchain %p, context %p, src_rect %s, dst_rect %s.\n",
            swapchain, context, wine_dbgstr_rect(src_rect), wine_dbgstr_rect(dst_rect));

    if (!once++)
        FIXME("Using GDI present.\n");

    format = back_buffer->resource.format;
    if (!format->ddi_format)
    {
        WARN("Cannot create a DC for format %s.\n", debug_d3dformat(format->id));
        return;
    }

    wined3d_texture_load_location(back_buffer, 0, context, WINED3D_LOCATION_SYSMEM);
    wined3d_texture_get_pitch(back_buffer, 0, &row_pitch, &slice_pitch);

    create_desc.pMemory = back_buffer->resource.heap_memory;
    create_desc.Format = format->ddi_format;
    create_desc.Width = wined3d_texture_get_level_width(back_buffer, 0);
    create_desc.Height = wined3d_texture_get_level_height(back_buffer, 0);
    create_desc.Pitch = row_pitch;
    create_desc.hDeviceDc = CreateCompatibleDC(NULL);
    create_desc.pColorTable = NULL;

    status = D3DKMTCreateDCFromMemory(&create_desc);
    DeleteDC(create_desc.hDeviceDc);
    if (status)
    {
        WARN("Failed to create DC, status %#lx.\n", status);
        return;
    }

    src_dc = create_desc.hDc;
    bitmap = create_desc.hBitmap;

    TRACE("Created source DC %p, bitmap %p for backbuffer %p.\n", src_dc, bitmap, back_buffer);

    if (!StretchBlt(swapchain->dc, dst_rect->left, dst_rect->top, dst_rect->right - dst_rect->left,
            dst_rect->bottom - dst_rect->top, src_dc, src_rect->left, src_rect->top,
            src_rect->right - src_rect->left, src_rect->bottom - src_rect->top, SRCCOPY))
        ERR("Failed to blit.\n");

    destroy_desc.hDc = src_dc;
    destroy_desc.hBitmap = bitmap;
    if ((status = D3DKMTDestroyDCFromMemory(&destroy_desc)))
        ERR("Failed to destroy src dc, status %#lx.\n", status);
}

/* A GL context is provided by the caller */
static void swapchain_blit(const struct wined3d_swapchain *swapchain,
        struct wined3d_context *context, const RECT *src_rect, const RECT *dst_rect)
{
    struct wined3d_texture *texture = swapchain->back_buffers[0];
    struct wined3d_device *device = swapchain->device;
    enum wined3d_texture_filter_type filter;
    DWORD location;

    TRACE("swapchain %p, context %p, src_rect %s, dst_rect %s.\n",
            swapchain, context, wine_dbgstr_rect(src_rect), wine_dbgstr_rect(dst_rect));

    if ((src_rect->right - src_rect->left == dst_rect->right - dst_rect->left
            && src_rect->bottom - src_rect->top == dst_rect->bottom - dst_rect->top)
            || is_complex_fixup(texture->resource.format->color_fixup))
        filter = WINED3D_TEXF_NONE;
    else
        filter = WINED3D_TEXF_LINEAR;

    location = WINED3D_LOCATION_TEXTURE_RGB;
    if (texture->resource.multisample_type)
        location = WINED3D_LOCATION_RB_RESOLVED;

    wined3d_texture_validate_location(texture, 0, WINED3D_LOCATION_DRAWABLE);
    device->blitter->ops->blitter_blit(device->blitter, WINED3D_BLIT_OP_COLOR_BLIT, context, texture, 0,
            location, src_rect, texture, 0, WINED3D_LOCATION_DRAWABLE, dst_rect, NULL, filter, NULL);
    wined3d_texture_invalidate_location(texture, 0, WINED3D_LOCATION_DRAWABLE);
}

static void swapchain_gl_set_swap_interval(struct wined3d_swapchain *swapchain,
        struct wined3d_context_gl *context_gl, unsigned int swap_interval)
{
    const struct wined3d_gl_info *gl_info = context_gl->gl_info;

    swap_interval = swap_interval <= 4 ? swap_interval : 1;
    if (swapchain->swap_interval == swap_interval)
        return;

    swapchain->swap_interval = swap_interval;

    if (!gl_info->supported[WGL_EXT_SWAP_CONTROL])
        return;

    if (!GL_EXTCALL(wglSwapIntervalEXT(swap_interval)))
    {
        ERR("Failed to set swap interval %u for context %p, last error %#lx.\n",
                swap_interval, context_gl, GetLastError());
    }
}

/* Context activation is done by the caller. */
static void wined3d_swapchain_gl_rotate(struct wined3d_swapchain *swapchain, struct wined3d_context *context)
{
    struct wined3d_texture_sub_resource *sub_resource;
    struct wined3d_texture_gl *texture, *texture_prev;
    struct gl_texture tex0;
    GLuint rb0;
    DWORD locations0;
    unsigned int i;
    static const DWORD supported_locations = WINED3D_LOCATION_TEXTURE_RGB | WINED3D_LOCATION_RB_MULTISAMPLE;

    if (swapchain->state.desc.backbuffer_count < 2)
        return;

    texture_prev = wined3d_texture_gl(swapchain->back_buffers[0]);

    /* Back buffer 0 is already in the draw binding. */
    tex0 = texture_prev->texture_rgb;
    rb0 = texture_prev->rb_multisample;
    locations0 = texture_prev->t.sub_resources[0].locations;

    for (i = 1; i < swapchain->state.desc.backbuffer_count; ++i)
    {
        texture = wined3d_texture_gl(swapchain->back_buffers[i]);
        sub_resource = &texture->t.sub_resources[0];

        if (!(sub_resource->locations & supported_locations))
            wined3d_texture_load_location(&texture->t, 0, context, texture->t.resource.draw_binding);

        texture_prev->texture_rgb = texture->texture_rgb;
        texture_prev->rb_multisample = texture->rb_multisample;

        wined3d_texture_validate_location(&texture_prev->t, 0, sub_resource->locations & supported_locations);
        wined3d_texture_invalidate_location(&texture_prev->t, 0, ~(sub_resource->locations & supported_locations));

        texture_prev = texture;
    }

    texture_prev->texture_rgb = tex0;
    texture_prev->rb_multisample = rb0;

    wined3d_texture_validate_location(&texture_prev->t, 0, locations0 & supported_locations);
    wined3d_texture_invalidate_location(&texture_prev->t, 0, ~(locations0 & supported_locations));

    device_invalidate_state(swapchain->device, STATE_FRAMEBUFFER);
}

static bool swapchain_present_is_partial_copy(struct wined3d_swapchain *swapchain, const RECT *dst_rect)
{
    enum wined3d_swap_effect swap_effect = swapchain->state.desc.swap_effect;
    const struct wined3d_swapchain_desc *desc = &swapchain->state.desc;
    unsigned int width, height;

    if (swap_effect != WINED3D_SWAP_EFFECT_COPY && swap_effect != WINED3D_SWAP_EFFECT_COPY_VSYNC)
        return false;

    if (!desc->windowed)
    {
        width = desc->backbuffer_width;
        height = desc->backbuffer_height;
    }
    else
    {
        RECT client_rect;
        GetClientRect(swapchain->win_handle, &client_rect);
        width = client_rect.right - client_rect.left;
        height = client_rect.bottom - client_rect.top;
    }

    if ((dst_rect->left && dst_rect->right) || abs(dst_rect->right - dst_rect->left) != width)
        return true;
    if ((dst_rect->top && dst_rect->bottom) || abs(dst_rect->bottom - dst_rect->top) != height)
        return true;

    return false;
}

static void swapchain_gl_present(struct wined3d_swapchain *swapchain,
        const RECT *src_rect, const RECT *dst_rect, unsigned int swap_interval, uint32_t flags)
{
    struct wined3d_texture *back_buffer = swapchain->back_buffers[0];
    const struct wined3d_pixel_format *pixel_format;
    const struct wined3d_gl_info *gl_info;
    struct wined3d_context_gl *context_gl;
    struct wined3d_context *context;

    context = context_acquire(swapchain->device, swapchain->front_buffer, 0);
    context_gl = wined3d_context_gl(context);
    if (!context_gl->valid)
    {
        context_release(context);
        WARN("Invalid context, skipping present.\n");
        return;
    }

    TRACE("Presenting DC %p.\n", context_gl->dc);

    pixel_format = &wined3d_adapter_gl(swapchain->device->adapter)->pixel_formats[context_gl->pixel_format];
    if (context_gl->dc == wined3d_device_gl(swapchain->device)->backup_dc
            || (pixel_format->swap_method != WGL_SWAP_COPY_ARB
            && swapchain_present_is_partial_copy(swapchain, dst_rect)))
    {
        swapchain_blit_gdi(swapchain, context, src_rect, dst_rect);
    }
    else
    {
        gl_info = context_gl->gl_info;

        swapchain_gl_set_swap_interval(swapchain, context_gl, swap_interval);

        wined3d_texture_load_location(back_buffer, 0, context, back_buffer->resource.draw_binding);

        swapchain_blit(swapchain, context, src_rect, dst_rect);

        if (swapchain->device->context_count > 1)
        {
            WARN_(d3d_perf)("Multiple contexts, calling glFinish() to enforce ordering.\n");
            gl_info->gl_ops.gl.p_glFinish();
        }

        /* call wglSwapBuffers through the gl table to avoid confusing the Steam overlay */
        gl_info->gl_ops.wgl.p_wglSwapBuffers(context_gl->dc);
    }

    if (context->d3d_info->fences)
        wined3d_context_gl_submit_command_fence(context_gl);

    wined3d_swapchain_gl_rotate(swapchain, context);

    TRACE("SwapBuffers called, Starting new frame\n");

    wined3d_texture_validate_location(swapchain->front_buffer, 0, WINED3D_LOCATION_DRAWABLE);
    wined3d_texture_invalidate_location(swapchain->front_buffer, 0, ~WINED3D_LOCATION_DRAWABLE);

    context_release(context);
}

static void swapchain_frontbuffer_updated(struct wined3d_swapchain *swapchain)
{
    struct wined3d_texture *front_buffer = swapchain->front_buffer;
    struct wined3d_context *context;

    context = context_acquire(swapchain->device, front_buffer, 0);
    wined3d_texture_load_location(front_buffer, 0, context, front_buffer->resource.draw_binding);
    context_release(context);
    SetRectEmpty(&swapchain->front_buffer_update);
}

static const struct wined3d_swapchain_ops swapchain_gl_ops =
{
    swapchain_gl_present,
    swapchain_frontbuffer_updated,
};

static void swapchain_gdi_frontbuffer_updated(struct wined3d_swapchain *swapchain)
{
    struct wined3d_dc_info *front;
    POINT offset = {0, 0};
    RECT draw_rect;
    HWND window;
    HDC src_dc;

    TRACE("swapchain %p.\n", swapchain);

    front = &swapchain->front_buffer->dc_info[0];
    if (swapchain->palette)
        wined3d_palette_apply_to_dc(swapchain->palette, front->dc);

    if (swapchain->front_buffer->resource.map_count)
        ERR("Trying to blit a mapped surface.\n");

    TRACE("Copying surface %p to screen.\n", front);

    src_dc = front->dc;
    window = swapchain->win_handle;

    /* Front buffer coordinates are screen coordinates. Map them to the
     * destination window if not fullscreened. */
    if (swapchain->state.desc.windowed)
        ClientToScreen(window, &offset);

    TRACE("offset %s.\n", wine_dbgstr_point(&offset));

    SetRect(&draw_rect, 0, 0, swapchain->front_buffer->resource.width,
            swapchain->front_buffer->resource.height);
    IntersectRect(&draw_rect, &draw_rect, &swapchain->front_buffer_update);

    BitBlt(swapchain->dc, draw_rect.left - offset.x, draw_rect.top - offset.y,
            draw_rect.right - draw_rect.left, draw_rect.bottom - draw_rect.top,
            src_dc, draw_rect.left, draw_rect.top, SRCCOPY);

    SetRectEmpty(&swapchain->front_buffer_update);
}

static void swapchain_gdi_present(struct wined3d_swapchain *swapchain,
        const RECT *src_rect, const RECT *dst_rect, unsigned int swap_interval, uint32_t flags)
{
    struct wined3d_dc_info *front, *back;
    HBITMAP bitmap;
    void *heap_pointer;
    void *heap_memory;
    HDC dc;

    front = &swapchain->front_buffer->dc_info[0];
    back = &swapchain->back_buffers[0]->dc_info[0];

    /* Flip the surface data. */
    dc = front->dc;
    bitmap = front->bitmap;
    heap_pointer = swapchain->front_buffer->resource.heap_pointer;
    heap_memory = swapchain->front_buffer->resource.heap_memory;

    front->dc = back->dc;
    front->bitmap = back->bitmap;
    swapchain->front_buffer->resource.heap_pointer = swapchain->back_buffers[0]->resource.heap_pointer;
    swapchain->front_buffer->resource.heap_memory = swapchain->back_buffers[0]->resource.heap_memory;

    back->dc = dc;
    back->bitmap = bitmap;
    swapchain->back_buffers[0]->resource.heap_pointer = heap_pointer;
    swapchain->back_buffers[0]->resource.heap_memory = heap_memory;

    SetRect(&swapchain->front_buffer_update, 0, 0,
            swapchain->front_buffer->resource.width,
            swapchain->front_buffer->resource.height);
    swapchain_gdi_frontbuffer_updated(swapchain);
}

static const struct wined3d_swapchain_ops swapchain_no3d_ops =
{
    swapchain_gdi_present,
    swapchain_gdi_frontbuffer_updated,
};

static void wined3d_swapchain_apply_sample_count_override(const struct wined3d_swapchain *swapchain,
        enum wined3d_format_id format_id, enum wined3d_multisample_type *type, unsigned int *quality)
{
    const struct wined3d_adapter *adapter;
    const struct wined3d_format *format;
    enum wined3d_multisample_type t;

    if (wined3d_settings.sample_count == ~0u)
        return;

    adapter = swapchain->device->adapter;
    if (!(format = wined3d_get_format(adapter, format_id, WINED3D_BIND_RENDER_TARGET)))
        return;

    if ((t = min(wined3d_settings.sample_count, adapter->d3d_info.limits.sample_count)))
        while (!(format->multisample_types & 1u << (t - 1)))
            ++t;
    TRACE("Using sample count %u.\n", t);
    *type = t;
    *quality = 0;
}

void swapchain_set_max_frame_latency(struct wined3d_swapchain *swapchain, const struct wined3d_device *device)
{
    /* Subtract 1 for the implicit OpenGL latency. */
    swapchain->max_frame_latency = device->max_frame_latency >= 2 ? device->max_frame_latency - 1 : 1;
}

static enum wined3d_format_id adapter_format_from_backbuffer_format(const struct wined3d_adapter *adapter,
        enum wined3d_format_id format_id)
{
    const struct wined3d_format *backbuffer_format;

    backbuffer_format = wined3d_get_format(adapter, format_id, WINED3D_BIND_RENDER_TARGET);
    return pixelformat_for_depth(backbuffer_format->byte_count * CHAR_BIT);
}

static HRESULT wined3d_swapchain_state_init(struct wined3d_swapchain_state *state,
        const struct wined3d_swapchain_desc *desc, HWND window, struct wined3d *wined3d,
        struct wined3d_swapchain_state_parent *parent)
{
    HRESULT hr;

    state->desc = *desc;

    if (FAILED(hr = wined3d_output_get_display_mode(desc->output, &state->original_mode, NULL)))
    {
        ERR("Failed to get current display mode, hr %#lx.\n", hr);
        return hr;
    }

    if (state->desc.windowed)
    {
        RECT client_rect;

        GetClientRect(window, &client_rect);
        TRACE("Client rect %s.\n", wine_dbgstr_rect(&client_rect));

        if (!state->desc.backbuffer_width)
        {
            state->desc.backbuffer_width = client_rect.right ? client_rect.right : 8;
            TRACE("Updating width to %u.\n", state->desc.backbuffer_width);
        }
        if (!state->desc.backbuffer_height)
        {
            state->desc.backbuffer_height = client_rect.bottom ? client_rect.bottom : 8;
            TRACE("Updating height to %u.\n", state->desc.backbuffer_height);
        }

        if (state->desc.backbuffer_format == WINED3DFMT_UNKNOWN)
        {
            state->desc.backbuffer_format = state->original_mode.format_id;
            TRACE("Updating format to %s.\n", debug_d3dformat(state->original_mode.format_id));
        }
    }
    else
    {
        if (desc->flags & WINED3D_SWAPCHAIN_ALLOW_MODE_SWITCH)
        {
            state->d3d_mode.width = desc->backbuffer_width;
            state->d3d_mode.height = desc->backbuffer_height;
            state->d3d_mode.format_id = adapter_format_from_backbuffer_format(desc->output->adapter,
                    desc->backbuffer_format);
            state->d3d_mode.refresh_rate = desc->refresh_rate;
            state->d3d_mode.scanline_ordering = WINED3D_SCANLINE_ORDERING_UNKNOWN;
        }
        else
        {
            state->d3d_mode = state->original_mode;
        }
    }

    GetWindowRect(window, &state->original_window_rect);
    state->wined3d = wined3d;
    state->device_window = window;
    state->desc.device_window = window;
    state->parent = parent;

    if (desc->flags & WINED3D_SWAPCHAIN_REGISTER_STATE)
        wined3d_swapchain_state_register(state);

    return hr;
}

static HRESULT swapchain_create_texture(struct wined3d_swapchain *swapchain,
        bool front, bool depth, struct wined3d_texture **texture)
{
    const struct wined3d_swapchain_desc *swapchain_desc = &swapchain->state.desc;
    struct wined3d_device *device = swapchain->device;
    struct wined3d_resource_desc texture_desc;
    uint32_t texture_flags = 0;
    HRESULT hr;

    texture_desc.resource_type = WINED3D_RTYPE_TEXTURE_2D;
    texture_desc.format = depth ? swapchain_desc->auto_depth_stencil_format : swapchain_desc->backbuffer_format;
    texture_desc.multisample_type = swapchain_desc->multisample_type;
    texture_desc.multisample_quality = swapchain_desc->multisample_quality;
    texture_desc.usage = 0;
    if (!depth && (device->wined3d->flags & WINED3D_NO3D))
        texture_desc.usage |= WINED3DUSAGE_OWNDC;
    if (device->wined3d->flags & WINED3D_NO3D)
        texture_desc.access = WINED3D_RESOURCE_ACCESS_CPU;
    else
        texture_desc.access = WINED3D_RESOURCE_ACCESS_GPU;
    if (!depth && (swapchain_desc->flags & WINED3D_SWAPCHAIN_LOCKABLE_BACKBUFFER))
        texture_desc.access |= WINED3D_RESOURCE_ACCESS_MAP_R | WINED3D_RESOURCE_ACCESS_MAP_W;
    texture_desc.width = swapchain_desc->backbuffer_width;
    texture_desc.height = swapchain_desc->backbuffer_height;
    texture_desc.depth = 1;
    texture_desc.size = 0;

    if (front)
        texture_desc.bind_flags = 0;
    else if (depth)
        texture_desc.bind_flags = WINED3D_BIND_DEPTH_STENCIL;
    else
        texture_desc.bind_flags = swapchain_desc->backbuffer_bind_flags;

    if (swapchain_desc->flags & WINED3D_SWAPCHAIN_GDI_COMPATIBLE)
        texture_flags |= WINED3D_TEXTURE_CREATE_GET_DC;

    if (FAILED(hr = wined3d_texture_create(device, &texture_desc, 1, 1,
            texture_flags, NULL, NULL, &wined3d_null_parent_ops, texture)))
    {
        WARN("Failed to create texture, hr %#lx.\n", hr);
        return hr;
    }

    if (!depth)
        wined3d_texture_set_swapchain(*texture, swapchain);

    return S_OK;
}

static HRESULT wined3d_swapchain_init(struct wined3d_swapchain *swapchain, struct wined3d_device *device,
        const struct wined3d_swapchain_desc *desc, struct wined3d_swapchain_state_parent *state_parent,
        void *parent, const struct wined3d_parent_ops *parent_ops,
        const struct wined3d_swapchain_ops *swapchain_ops)
{
    struct wined3d_output_desc output_desc;
    BOOL displaymode_set = FALSE;
    HRESULT hr = E_FAIL;
    unsigned int i;
    HWND window;

    wined3d_mutex_lock();

    if (desc->backbuffer_count > 1)
    {
        FIXME("The application requested more than one back buffer, this is not properly supported.\n"
                "Please configure the application to use double buffering (1 back buffer) if possible.\n");
    }

    if (desc->swap_effect != WINED3D_SWAP_EFFECT_DISCARD
            && desc->swap_effect != WINED3D_SWAP_EFFECT_SEQUENTIAL
            && desc->swap_effect != WINED3D_SWAP_EFFECT_COPY)
        FIXME("Unimplemented swap effect %#x.\n", desc->swap_effect);

    window = desc->device_window ? desc->device_window : device->create_parms.focus_window;
    TRACE("Using target window %p.\n", window);

    if (FAILED(hr = wined3d_swapchain_state_init(&swapchain->state, desc, window, device->wined3d, state_parent)))
    {
        ERR("Failed to initialise swapchain state, hr %#lx.\n", hr);
        wined3d_mutex_unlock();
        return hr;
    }

    swapchain->swapchain_ops = swapchain_ops;
    swapchain->device = device;
    swapchain->parent = parent;
    swapchain->parent_ops = parent_ops;
    swapchain->ref = 1;
    swapchain->win_handle = window;
    swapchain->swap_interval = WINED3D_SWAP_INTERVAL_DEFAULT;
    swapchain_set_max_frame_latency(swapchain, device);

    if (!(swapchain->dc = GetDCEx(swapchain->win_handle, 0, DCX_USESTYLE | DCX_CACHE)))
        WARN("Failed to retrieve device context, trying swapchain backup.\n");

    if (!swapchain->state.desc.windowed)
    {
        if (FAILED(hr = wined3d_output_get_desc(desc->output, &output_desc)))
        {
            ERR("Failed to get output description, hr %#lx.\n", hr);
            goto err;
        }

        wined3d_swapchain_state_setup_fullscreen(&swapchain->state, window,
                output_desc.desktop_rect.left, output_desc.desktop_rect.top, desc->backbuffer_width,
                desc->backbuffer_height);
    }
    wined3d_swapchain_apply_sample_count_override(swapchain, swapchain->state.desc.backbuffer_format,
            &swapchain->state.desc.multisample_type, &swapchain->state.desc.multisample_quality);

    TRACE("Creating front buffer.\n");

    if (FAILED(hr = swapchain_create_texture(swapchain, true, false, &swapchain->front_buffer)))
    {
        WARN("Failed to create front buffer, hr %#lx.\n", hr);
        goto err;
    }

    if (!(device->wined3d->flags & WINED3D_NO3D))
    {
        wined3d_texture_validate_location(swapchain->front_buffer, 0, WINED3D_LOCATION_DRAWABLE);
        wined3d_texture_invalidate_location(swapchain->front_buffer, 0, ~WINED3D_LOCATION_DRAWABLE);
    }

    /* MSDN says we're only allowed a single fullscreen swapchain per device,
     * so we should really check to see if there is a fullscreen swapchain
     * already. Does a single head count as full screen? */
    if (!desc->windowed && desc->flags & WINED3D_SWAPCHAIN_ALLOW_MODE_SWITCH)
    {
        /* Change the display settings */
        if (FAILED(hr = wined3d_output_set_display_mode(desc->output,
                &swapchain->state.d3d_mode)))
        {
            WARN("Failed to set display mode, hr %#lx.\n", hr);
            goto err;
        }
        displaymode_set = TRUE;
    }

    if (swapchain->state.desc.backbuffer_count > 0)
    {
        if (!(swapchain->back_buffers = calloc(swapchain->state.desc.backbuffer_count,
                sizeof(*swapchain->back_buffers))))
        {
            ERR("Failed to allocate backbuffer array memory.\n");
            hr = E_OUTOFMEMORY;
            goto err;
        }

        for (i = 0; i < swapchain->state.desc.backbuffer_count; ++i)
        {
            TRACE("Creating back buffer %u.\n", i);
            if (FAILED(hr = swapchain_create_texture(swapchain, false, false, &swapchain->back_buffers[i])))
            {
                WARN("Failed to create back buffer %u, hr %#lx.\n", i, hr);
                swapchain->state.desc.backbuffer_count = i;
                goto err;
            }
        }
    }

    /* Swapchains share the depth/stencil buffer, so only create a single depthstencil surface. */
    if (desc->enable_auto_depth_stencil)
    {
        TRACE("Creating depth/stencil buffer.\n");
        if (!device->auto_depth_stencil_view)
        {
            struct wined3d_view_desc desc;
            struct wined3d_texture *ds;

            if (FAILED(hr = swapchain_create_texture(swapchain, false, true, &ds)))
            {
                WARN("Failed to create the auto depth/stencil surface, hr %#lx.\n", hr);
                goto err;
            }

            desc.format_id = ds->resource.format->id;
            desc.flags = 0;
            desc.u.texture.level_idx = 0;
            desc.u.texture.level_count = 1;
            desc.u.texture.layer_idx = 0;
            desc.u.texture.layer_count = 1;
            hr = wined3d_rendertarget_view_create(&desc, &ds->resource, NULL, &wined3d_null_parent_ops,
                    &device->auto_depth_stencil_view);
            wined3d_texture_decref(ds);
            if (FAILED(hr))
            {
                ERR("Failed to create rendertarget view, hr %#lx.\n", hr);
                goto err;
            }
        }
    }

    wined3d_swapchain_get_gamma_ramp(swapchain, &swapchain->orig_gamma);

    wined3d_mutex_unlock();

    return WINED3D_OK;

err:
    if (displaymode_set)
    {
        if (FAILED(wined3d_restore_display_modes(device->wined3d)))
            ERR("Failed to restore display mode.\n");
    }

    if (swapchain->back_buffers)
    {
        for (i = 0; i < swapchain->state.desc.backbuffer_count; ++i)
        {
            if (swapchain->back_buffers[i])
            {
                wined3d_texture_set_swapchain(swapchain->back_buffers[i], NULL);
                wined3d_texture_decref(swapchain->back_buffers[i]);
            }
        }
        free(swapchain->back_buffers);
    }

    if (swapchain->front_buffer)
    {
        wined3d_texture_set_swapchain(swapchain->front_buffer, NULL);
        wined3d_texture_decref(swapchain->front_buffer);
    }

    if (swapchain->dc)
        wined3d_release_dc(swapchain->win_handle, swapchain->dc);

    wined3d_swapchain_state_cleanup(&swapchain->state);
    wined3d_mutex_unlock();

    return hr;
}

HRESULT wined3d_swapchain_no3d_init(struct wined3d_swapchain *swapchain_no3d, struct wined3d_device *device,
        const struct wined3d_swapchain_desc *desc, struct wined3d_swapchain_state_parent *state_parent,
        void *parent, const struct wined3d_parent_ops *parent_ops)
{
    TRACE("swapchain_no3d %p, device %p, desc %p, state_parent %p, parent %p, parent_ops %p.\n",
            swapchain_no3d, device, desc, state_parent, parent, parent_ops);

    return wined3d_swapchain_init(swapchain_no3d, device, desc, state_parent, parent, parent_ops,
            &swapchain_no3d_ops);
}

HRESULT wined3d_swapchain_gl_init(struct wined3d_swapchain_gl *swapchain_gl, struct wined3d_device *device,
        const struct wined3d_swapchain_desc *desc, struct wined3d_swapchain_state_parent *state_parent,
        void *parent, const struct wined3d_parent_ops *parent_ops)
{
    TRACE("swapchain_gl %p, device %p, desc %p, state_parent %p, parent %p, parent_ops %p.\n",
            swapchain_gl, device, desc, state_parent, parent, parent_ops);

    return wined3d_swapchain_init(&swapchain_gl->s, device, desc, state_parent, parent,
            parent_ops, &swapchain_gl_ops);
}

HRESULT CDECL wined3d_swapchain_create(struct wined3d_device *device,
        const struct wined3d_swapchain_desc *desc, struct wined3d_swapchain_state_parent *state_parent,
        void *parent, const struct wined3d_parent_ops *parent_ops,
        struct wined3d_swapchain **swapchain)
{
    struct wined3d_swapchain *object;
    HRESULT hr;

    if (FAILED(hr = device->adapter->adapter_ops->adapter_create_swapchain(device,
            desc, state_parent, parent, parent_ops, &object)))
        return hr;

    if (desc->flags & WINED3D_SWAPCHAIN_IMPLICIT)
    {
        wined3d_mutex_lock();
        if (FAILED(hr = wined3d_device_set_implicit_swapchain(device, object)))
        {
            wined3d_cs_finish(device->cs, WINED3D_CS_QUEUE_DEFAULT);
            device->adapter->adapter_ops->adapter_destroy_swapchain(object);
            wined3d_mutex_unlock();
            return hr;
        }
        wined3d_mutex_unlock();
    }

    *swapchain = object;

    return hr;
}

static struct wined3d_context_gl *wined3d_swapchain_gl_create_context(struct wined3d_swapchain_gl *swapchain_gl)
{
    struct wined3d_device *device = swapchain_gl->s.device;
    struct wined3d_context_gl *context_gl;

    TRACE("Creating a new context for swapchain %p, thread %lu.\n", swapchain_gl, GetCurrentThreadId());

    wined3d_from_cs(device->cs);

    if (!(context_gl = calloc(1, sizeof(*context_gl))))
    {
        ERR("Failed to allocate context memory.\n");
        return NULL;
    }

    if (FAILED(wined3d_context_gl_init(context_gl, swapchain_gl)))
    {
        WARN("Failed to initialise context.\n");
        free(context_gl);
        return NULL;
    }

    if (!device_context_add(device, &context_gl->c))
    {
        ERR("Failed to add the newly created context to the context list.\n");
        wined3d_context_gl_destroy(context_gl);
        return NULL;
    }

    TRACE("Created context %p.\n", context_gl);

    context_release(&context_gl->c);

    return context_gl;
}

struct wined3d_context_gl *wined3d_swapchain_gl_get_context(struct wined3d_swapchain_gl *swapchain_gl)
{
    struct wined3d_device *device = swapchain_gl->s.device;
    DWORD tid = GetCurrentThreadId();
    unsigned int i;

    for (i = 0; i < device->context_count; ++i)
    {
        if (wined3d_context_gl(device->contexts[i])->tid == tid)
            return wined3d_context_gl(device->contexts[i]);
    }

    /* Create a new context for the thread. */
    return wined3d_swapchain_gl_create_context(swapchain_gl);
}

void swapchain_update_draw_bindings(struct wined3d_swapchain *swapchain)
{
    UINT i;

    wined3d_resource_update_draw_binding(&swapchain->front_buffer->resource);

    for (i = 0; i < swapchain->state.desc.backbuffer_count; ++i)
    {
        wined3d_resource_update_draw_binding(&swapchain->back_buffers[i]->resource);
    }
}

void wined3d_swapchain_activate(struct wined3d_swapchain *swapchain, BOOL activate)
{
    struct wined3d_device *device = swapchain->device;
    HWND window = swapchain->state.device_window;
    struct wined3d_output_desc output_desc;
    unsigned int screensaver_active;
    struct wined3d_output *output;
    BOOL focus_messages, filter;
    HRESULT hr;

    /* This code is not protected by the wined3d mutex, so it may run while
     * wined3d_device_reset is active. Testing on Windows shows that changing
     * focus during resets and resetting during focus change events causes
     * the application to crash with an invalid memory access. */

    if (!(focus_messages = device->wined3d->flags & WINED3D_FOCUS_MESSAGES))
        filter = wined3d_filter_messages(window, TRUE);

    if (activate)
    {
        SystemParametersInfoW(SPI_GETSCREENSAVEACTIVE, 0, &screensaver_active, 0);
        if ((device->restore_screensaver = !!screensaver_active))
            SystemParametersInfoW(SPI_SETSCREENSAVEACTIVE, FALSE, NULL, 0);

        if (!(device->create_parms.flags & WINED3DCREATE_NOWINDOWCHANGES))
        {
            /* The d3d versions do not agree on the exact messages here. D3d8 restores
             * the window but leaves the size untouched, d3d9 sets the size on an
             * invisible window, generates messages but doesn't change the window
             * properties. The implementation follows d3d9.
             *
             * Guild Wars 1 wants a WINDOWPOSCHANGED message on the device window to
             * resume drawing after a focus loss. */
            output = wined3d_swapchain_get_output(swapchain);
            if (!output)
            {
                ERR("Failed to get output from swapchain %p.\n", swapchain);
                return;
            }

            if (SUCCEEDED(hr = wined3d_output_get_desc(output, &output_desc)))
                SetWindowPos(window, NULL, output_desc.desktop_rect.left,
                        output_desc.desktop_rect.top, swapchain->state.desc.backbuffer_width,
                        swapchain->state.desc.backbuffer_height, SWP_NOACTIVATE | SWP_NOZORDER);
            else
                ERR("Failed to get output description, hr %#lx.\n", hr);
        }

        if (device->wined3d->flags & WINED3D_RESTORE_MODE_ON_ACTIVATE)
        {
            output = wined3d_swapchain_get_output(swapchain);
            if (!output)
            {
                ERR("Failed to get output from swapchain %p.\n", swapchain);
                return;
            }

            if (FAILED(hr = wined3d_output_set_display_mode(output,
                    &swapchain->state.d3d_mode)))
                ERR("Failed to set display mode, hr %#lx.\n", hr);
        }

        if (swapchain == device->swapchains[0])
            device->device_parent->ops->activate(device->device_parent, TRUE);
    }
    else
    {
        if (device->restore_screensaver)
        {
            SystemParametersInfoW(SPI_SETSCREENSAVEACTIVE, TRUE, NULL, 0);
            device->restore_screensaver = FALSE;
        }

        if (FAILED(hr = wined3d_restore_display_modes(device->wined3d)))
            ERR("Failed to restore display modes, hr %#lx.\n", hr);

        swapchain->reapply_mode = TRUE;

        /* Some DDraw apps (Deus Ex: GOTY, and presumably all UT 1 based games) destroy the device
         * during window minimization. Do our housekeeping now, as the device may not exist after
         * the ShowWindow call.
         *
         * In d3d9, the device is marked lost after the window is minimized. If we find an app
         * that needs this behavior (e.g. because it calls TestCooperativeLevel in the window proc)
         * we'll have to control this via a create flag. Note that the device and swapchain are not
         * safe to access after the ShowWindow call. */
        if (swapchain == device->swapchains[0])
            device->device_parent->ops->activate(device->device_parent, FALSE);

        if (!(device->create_parms.flags & WINED3DCREATE_NOWINDOWCHANGES) && IsWindowVisible(window))
            ShowWindow(window, SW_MINIMIZE);
    }

    if (!focus_messages)
        wined3d_filter_messages(window, filter);
}

HRESULT CDECL wined3d_swapchain_resize_buffers(struct wined3d_swapchain *swapchain, unsigned int buffer_count,
        unsigned int width, unsigned int height, enum wined3d_format_id format_id,
        enum wined3d_multisample_type multisample_type, unsigned int multisample_quality)
{
    struct wined3d_swapchain_desc *desc = &swapchain->state.desc;
    bool recreate = false;

    TRACE("swapchain %p, buffer_count %u, width %u, height %u, format %s, "
            "multisample_type %#x, multisample_quality %#x.\n",
            swapchain, buffer_count, width, height, debug_d3dformat(format_id),
            multisample_type, multisample_quality);

    wined3d_swapchain_apply_sample_count_override(swapchain, format_id, &multisample_type, &multisample_quality);

    if (buffer_count && buffer_count != desc->backbuffer_count)
        FIXME("Cannot change the back buffer count yet.\n");

    wined3d_cs_finish(swapchain->device->cs, WINED3D_CS_QUEUE_DEFAULT);

    if (!width || !height)
    {
        RECT client_rect;

        /* The application is requesting that either the swapchain width or
         * height be set to the corresponding dimension in the window's
         * client rect. */

        if (!GetClientRect(swapchain->state.device_window, &client_rect))
        {
            ERR("Failed to get client rect, last error %#lx.\n", GetLastError());
            return WINED3DERR_INVALIDCALL;
        }

        if (!width)
            width = client_rect.right;

        if (!height)
            height = client_rect.bottom;
    }

    if (width != desc->backbuffer_width || height != desc->backbuffer_height)
    {
        desc->backbuffer_width = width;
        desc->backbuffer_height = height;
        recreate = true;
    }

    if (format_id == WINED3DFMT_UNKNOWN)
    {
        if (!desc->windowed)
            return WINED3DERR_INVALIDCALL;
        format_id = swapchain->state.original_mode.format_id;
    }

    if (format_id != desc->backbuffer_format)
    {
        desc->backbuffer_format = format_id;
        recreate = true;
    }

    if (multisample_type != desc->multisample_type
            || multisample_quality != desc->multisample_quality)
    {
        desc->multisample_type = multisample_type;
        desc->multisample_quality = multisample_quality;
        recreate = true;
    }

    if (recreate)
    {
        struct wined3d_texture *new_texture;
        HRESULT hr;
        UINT i;

        TRACE("Recreating swapchain textures.\n");

        if (FAILED(hr = swapchain_create_texture(swapchain, true, false, &new_texture)))
            return hr;
        wined3d_texture_set_swapchain(swapchain->front_buffer, NULL);
        if (wined3d_texture_decref(swapchain->front_buffer))
            ERR("Something's still holding the front buffer (%p).\n", swapchain->front_buffer);
        swapchain->front_buffer = new_texture;

        if (!(swapchain->device->wined3d->flags & WINED3D_NO3D))
        {
            wined3d_texture_validate_location(swapchain->front_buffer, 0, WINED3D_LOCATION_DRAWABLE);
            wined3d_texture_invalidate_location(swapchain->front_buffer, 0, ~WINED3D_LOCATION_DRAWABLE);
        }

        for (i = 0; i < desc->backbuffer_count; ++i)
        {
            if (FAILED(hr = swapchain_create_texture(swapchain, false, false, &new_texture)))
                return hr;
            wined3d_texture_set_swapchain(swapchain->back_buffers[i], NULL);
            if (wined3d_texture_decref(swapchain->back_buffers[i]))
                ERR("Something's still holding back buffer %u (%p).\n", i, swapchain->back_buffers[i]);
            swapchain->back_buffers[i] = new_texture;
        }
    }

    swapchain_update_draw_bindings(swapchain);

    return WINED3D_OK;
}

static HRESULT wined3d_swapchain_state_set_display_mode(struct wined3d_swapchain_state *state,
        struct wined3d_output *output, struct wined3d_display_mode *mode)
{
    HRESULT hr;

    if (state->desc.flags & WINED3D_SWAPCHAIN_USE_CLOSEST_MATCHING_MODE)
    {
        if (FAILED(hr = wined3d_output_find_closest_matching_mode(output, mode)))
        {
            WARN("Failed to find closest matching mode, hr %#lx.\n", hr);
        }
    }

    if (output != state->desc.output)
    {
        if (FAILED(hr = wined3d_restore_display_modes(state->wined3d)))
        {
            WARN("Failed to restore display modes, hr %#lx.\n", hr);
            return hr;
        }

        if (FAILED(hr = wined3d_output_get_display_mode(output, &state->original_mode, NULL)))
        {
            WARN("Failed to get current display mode, hr %#lx.\n", hr);
            return hr;
        }
    }

    if (FAILED(hr = wined3d_output_set_display_mode(output, mode)))
    {
        WARN("Failed to set display mode, hr %#lx.\n", hr);
        return WINED3DERR_INVALIDCALL;
    }

    return WINED3D_OK;
}

HRESULT CDECL wined3d_swapchain_state_resize_target(struct wined3d_swapchain_state *state,
        const struct wined3d_display_mode *mode)
{
    struct wined3d_display_mode actual_mode;
    struct wined3d_output_desc output_desc;
    RECT original_window_rect, window_rect;
    int x, y, width, height;
    HWND window;
    HRESULT hr;

    TRACE("state %p, mode %p.\n", state, mode);

    wined3d_mutex_lock();

    window = state->device_window;

    if (state->desc.windowed)
    {
        SetRect(&window_rect, 0, 0, mode->width, mode->height);
        AdjustWindowRectEx(&window_rect,
                GetWindowLongW(window, GWL_STYLE), FALSE,
                GetWindowLongW(window, GWL_EXSTYLE));
        GetWindowRect(window, &original_window_rect);

        x = original_window_rect.left;
        y = original_window_rect.top;
        width = window_rect.right - window_rect.left;
        height = window_rect.bottom - window_rect.top;
    }
    else
    {
        if (FAILED(hr = wined3d_output_get_desc(state->desc.output, &output_desc)))
        {
            ERR("Failed to get output description, hr %#lx.\n", hr);
            wined3d_mutex_unlock();
            return hr;
        }
        width = output_desc.desktop_rect.right - output_desc.desktop_rect.left;
        height = output_desc.desktop_rect.bottom - output_desc.desktop_rect.top;

        GetWindowRect(window, &window_rect);
        if (width != window_rect.right - window_rect.left || height != window_rect.bottom - window_rect.top)
        {
            TRACE("Update saved window state.\n");
            state->original_window_rect = window_rect;
        }

        if (state->desc.flags & WINED3D_SWAPCHAIN_ALLOW_MODE_SWITCH)
        {
            actual_mode = *mode;
            if (FAILED(hr = wined3d_swapchain_state_set_display_mode(state, state->desc.output,
                    &actual_mode)))
            {
                ERR("Failed to set display mode, hr %#lx.\n", hr);
                wined3d_mutex_unlock();
                return hr;
            }
            if (FAILED(hr = wined3d_output_get_desc(state->desc.output, &output_desc)))
            {
                ERR("Failed to get output description, hr %#lx.\n", hr);
                wined3d_mutex_unlock();
                return hr;
            }

            width = output_desc.desktop_rect.right - output_desc.desktop_rect.left;
            height = output_desc.desktop_rect.bottom - output_desc.desktop_rect.top;
        }
        x = output_desc.desktop_rect.left;
        y = output_desc.desktop_rect.top;
    }

    wined3d_mutex_unlock();

    MoveWindow(window, x, y, width, height, TRUE);

    return WINED3D_OK;
}

static LONG fullscreen_style(LONG style)
{
    /* Make sure the window is managed, otherwise we won't get keyboard input. */
    style |= WS_POPUP | WS_SYSMENU;
    style &= ~(WS_CAPTION | WS_THICKFRAME);

    return style;
}

static LONG fullscreen_exstyle(LONG exstyle)
{
    /* Filter out window decorations. */
    exstyle &= ~(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE);

    return exstyle;
}

struct wined3d_window_state
{
    HWND window;
    HWND window_pos_after;
    LONG style, exstyle;
    int x, y, width, height;
    uint32_t flags;
    bool set_style;
    bool register_topmost_timer;
    bool set_topmost_timer;
};

#define WINED3D_WINDOW_TOPMOST_TIMER_ID 0x4242

static void CALLBACK topmost_timer_proc(HWND hwnd, UINT msg, UINT_PTR id, DWORD time)
{
    if (!(GetWindowLongW(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST))
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    KillTimer(hwnd, WINED3D_WINDOW_TOPMOST_TIMER_ID);
}

static DWORD WINAPI set_window_state_thread(void *ctx)
{
    struct wined3d_window_state *s = ctx;
    bool filter;

    filter = wined3d_filter_messages(s->window, TRUE);

    if (s->set_style)
    {
        SetWindowLongW(s->window, GWL_STYLE, s->style);
        SetWindowLongW(s->window, GWL_EXSTYLE, s->exstyle);
    }
    SetWindowPos(s->window, s->window_pos_after, s->x, s->y, s->width, s->height, s->flags);

    wined3d_filter_messages(s->window, filter);

    free(s);

    return 0;
}

static void set_window_state(struct wined3d_window_state *s)
{
    static const UINT timeout = 1500;
    DWORD window_tid = GetWindowThreadProcessId(s->window, NULL);
    DWORD tid = GetCurrentThreadId();
    HANDLE thread;

    TRACE("Window %p belongs to thread %#lx.\n", s->window, window_tid);
    /* If the window belongs to a different thread, modifying the style and/or
     * position can potentially deadlock if that thread isn't processing
     * messages. */
    if (window_tid == tid)
    {
        /* Deus Ex: Game of the Year Edition removes WS_EX_TOPMOST after changing resolutions in
         * exclusive fullscreen mode. Tests show that WS_EX_TOPMOST will be restored when a ~1.5s
         * timer times out */
        if (s->register_topmost_timer)
        {
            if (s->set_topmost_timer)
                SetTimer(s->window, WINED3D_WINDOW_TOPMOST_TIMER_ID, timeout, topmost_timer_proc);
            else
                KillTimer(s->window, WINED3D_WINDOW_TOPMOST_TIMER_ID);
        }

        set_window_state_thread(s);
    }
    else if ((thread = CreateThread(NULL, 0, set_window_state_thread, s, 0, NULL)))
    {
        SetThreadDescription(thread, L"wined3d_set_window_state");
        CloseHandle(thread);
    }
    else
    {
        ERR("Failed to create thread.\n");
    }
}

HRESULT wined3d_swapchain_state_setup_fullscreen(struct wined3d_swapchain_state *state,
        HWND window, int x, int y, int width, int height)
{
    struct wined3d_window_state *s;

    TRACE("Setting up window %p for fullscreen mode.\n", window);

    if (!IsWindow(window))
    {
        WARN("%p is not a valid window.\n", window);
        return WINED3DERR_NOTAVAILABLE;
    }

    set_window_present_rect(window, x, y, width, height);

    if (!(s = malloc(sizeof(*s))))
        return E_OUTOFMEMORY;
    s->window = window;
    s->window_pos_after = HWND_TOPMOST;
    s->x = x;
    s->y = y;
    s->width = width;
    s->height = height;

    if (state->style || state->exstyle)
    {
        ERR("Changing the window style for window %p, but another style (%08lx, %08lx) is already stored.\n",
                window, state->style, state->exstyle);
    }

    s->flags = SWP_FRAMECHANGED | SWP_NOACTIVATE;
    if (state->desc.flags & WINED3D_SWAPCHAIN_NO_WINDOW_CHANGES)
        s->flags |= SWP_NOZORDER;
    else
        s->flags |= SWP_SHOWWINDOW;

    state->style = GetWindowLongW(window, GWL_STYLE);
    state->exstyle = GetWindowLongW(window, GWL_EXSTYLE);

    s->style = fullscreen_style(state->style);
    s->exstyle = fullscreen_exstyle(state->exstyle);
    s->set_style = true;
    s->register_topmost_timer = !!(state->desc.flags & WINED3D_SWAPCHAIN_REGISTER_TOPMOST_TIMER);
    s->set_topmost_timer = true;

    TRACE("Old style was %08lx, %08lx, setting to %08lx, %08lx.\n",
            state->style, state->exstyle, s->style, s->exstyle);

    set_window_state(s);
    return WINED3D_OK;
}

void wined3d_swapchain_state_restore_from_fullscreen(struct wined3d_swapchain_state *state,
        HWND window, const RECT *window_rect)
{
    struct wined3d_window_state *s;
    LONG style, exstyle;

    set_window_present_rect(window, 0, 0, 0, 0);

    if (!state->style && !state->exstyle)
        return;

    if (!(s = malloc(sizeof(*s))))
        return;

    s->window = window;
    s->window_pos_after = NULL;
    s->flags = SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE;

    if ((state->desc.flags & WINED3D_SWAPCHAIN_RESTORE_WINDOW_STATE)
            && !(state->desc.flags & WINED3D_SWAPCHAIN_NO_WINDOW_CHANGES))
    {
        s->window_pos_after = (state->exstyle & WS_EX_TOPMOST) ? HWND_TOPMOST : HWND_NOTOPMOST;
        s->flags |= (state->style & WS_VISIBLE) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
        s->flags &= ~SWP_NOZORDER;
    }

    style = GetWindowLongW(window, GWL_STYLE);
    exstyle = GetWindowLongW(window, GWL_EXSTYLE);

    /* These flags are set by wined3d_device_setup_fullscreen_window, not the
     * application, and we want to ignore them in the test below, since it's
     * not the application's fault that they changed. Additionally, we want to
     * preserve the current status of these flags (i.e. don't restore them) to
     * more closely emulate the behavior of Direct3D, which leaves these flags
     * alone when returning to windowed mode. */
    state->style ^= (state->style ^ style) & WS_VISIBLE;
    state->exstyle ^= (state->exstyle ^ exstyle) & WS_EX_TOPMOST;

    TRACE("Restoring window style of window %p to %08lx, %08lx.\n",
            window, state->style, state->exstyle);

    s->style = state->style;
    s->exstyle = state->exstyle;
    /* Only restore the style if the application didn't modify it during the
     * fullscreen phase. Some applications change it before calling Reset()
     * when switching between windowed and fullscreen modes (HL2), some
     * depend on the original style (Eve Online). */
    s->set_style = style == fullscreen_style(state->style) && exstyle == fullscreen_exstyle(state->exstyle);
    s->register_topmost_timer = !!(state->desc.flags & WINED3D_SWAPCHAIN_REGISTER_TOPMOST_TIMER);
    s->set_topmost_timer = false;

    if (window_rect)
    {
        s->x = window_rect->left;
        s->y = window_rect->top;
        s->width = window_rect->right - window_rect->left;
        s->height = window_rect->bottom - window_rect->top;
    }
    else
    {
        s->x = s->y = s->width = s->height = 0;
        s->flags |= (SWP_NOMOVE | SWP_NOSIZE);
    }

    set_window_state(s);

    /* Delete the old values. */
    state->style = 0;
    state->exstyle = 0;
}

HRESULT CDECL wined3d_swapchain_state_set_fullscreen(struct wined3d_swapchain_state *state,
        const struct wined3d_swapchain_desc *swapchain_desc,
        const struct wined3d_display_mode *mode)
{
    struct wined3d_display_mode actual_mode;
    struct wined3d_output_desc output_desc;
    BOOL windowed = state->desc.windowed;
    HRESULT hr;

    TRACE("state %p, swapchain_desc %p, mode %p.\n", state, swapchain_desc, mode);

    if (state->desc.flags & WINED3D_SWAPCHAIN_ALLOW_MODE_SWITCH)
    {
        if (mode)
        {
            actual_mode = *mode;
            if (FAILED(hr = wined3d_swapchain_state_set_display_mode(state, swapchain_desc->output,
                    &actual_mode)))
                return hr;
        }
        else
        {
            if (!swapchain_desc->windowed)
            {
                actual_mode.width = swapchain_desc->backbuffer_width;
                actual_mode.height = swapchain_desc->backbuffer_height;
                actual_mode.refresh_rate = swapchain_desc->refresh_rate;
                actual_mode.format_id = adapter_format_from_backbuffer_format(swapchain_desc->output->adapter,
                        swapchain_desc->backbuffer_format);
                actual_mode.scanline_ordering = WINED3D_SCANLINE_ORDERING_UNKNOWN;
                if (FAILED(hr = wined3d_swapchain_state_set_display_mode(state, swapchain_desc->output,
                        &actual_mode)))
                    return hr;
            }
            else
            {
                if (FAILED(hr = wined3d_restore_display_modes(state->wined3d)))
                {
                    WARN("Failed to restore display modes for all outputs, hr %#lx.\n", hr);
                    return hr;
                }
            }
        }
    }
    else
    {
        if (mode)
            WARN("WINED3D_SWAPCHAIN_ALLOW_MODE_SWITCH is not set, ignoring mode.\n");

        if (FAILED(hr = wined3d_output_get_display_mode(swapchain_desc->output, &actual_mode,
                NULL)))
        {
            ERR("Failed to get display mode, hr %#lx.\n", hr);
            return WINED3DERR_INVALIDCALL;
        }
    }

    if (!swapchain_desc->windowed)
    {
        unsigned int width = actual_mode.width;
        unsigned int height = actual_mode.height;

        if (FAILED(hr = wined3d_output_get_desc(swapchain_desc->output, &output_desc)))
        {
            ERR("Failed to get output description, hr %#lx.\n", hr);
            return hr;
        }

        if (state->desc.windowed)
        {
            /* Switch from windowed to fullscreen */
            if (FAILED(hr = wined3d_swapchain_state_setup_fullscreen(state, state->device_window,
                    output_desc.desktop_rect.left, output_desc.desktop_rect.top, width, height)))
                return hr;
        }
        else
        {
            HWND window = state->device_window;
            BOOL filter;

            set_window_present_rect(state->device_window, output_desc.desktop_rect.left,
                    output_desc.desktop_rect.top, width, height);

            /* Fullscreen -> fullscreen mode change */
            filter = wined3d_filter_messages(window, TRUE);
            MoveWindow(window, output_desc.desktop_rect.left, output_desc.desktop_rect.top, width,
                    height, TRUE);
            ShowWindow(window, SW_SHOW);
            wined3d_filter_messages(window, filter);
        }
        state->d3d_mode = actual_mode;
    }
    else if (!state->desc.windowed)
    {
        /* Fullscreen -> windowed switch */
        RECT *window_rect = NULL;
        if (state->desc.flags & WINED3D_SWAPCHAIN_RESTORE_WINDOW_RECT)
            window_rect = &state->original_window_rect;
        wined3d_swapchain_state_restore_from_fullscreen(state, state->device_window, window_rect);
    }

    state->desc.output = swapchain_desc->output;
    state->desc.windowed = swapchain_desc->windowed;

    if (windowed != state->desc.windowed)
        state->parent->ops->windowed_state_changed(state->parent, state->desc.windowed);

    return WINED3D_OK;
}

BOOL CDECL wined3d_swapchain_state_is_windowed(const struct wined3d_swapchain_state *state)
{
    TRACE("state %p.\n", state);

    return state->desc.windowed;
}

void CDECL wined3d_swapchain_state_get_size(const struct wined3d_swapchain_state *state,
        unsigned int *width, unsigned int *height)
{
    TRACE("state %p.\n", state);

    *width = state->desc.backbuffer_width;
    *height = state->desc.backbuffer_height;
}

void CDECL wined3d_swapchain_state_destroy(struct wined3d_swapchain_state *state)
{
    wined3d_swapchain_state_cleanup(state);
    free(state);
}

HRESULT CDECL wined3d_swapchain_state_create(const struct wined3d_swapchain_desc *desc,
        HWND window, struct wined3d *wined3d, struct wined3d_swapchain_state_parent *state_parent,
        struct wined3d_swapchain_state **state)
{
    struct wined3d_swapchain_state *s;
    HRESULT hr;

    TRACE("desc %p, window %p, wined3d %p, state %p.\n", desc, window, wined3d, state);

    if (!(s = calloc(1, sizeof(*s))))
        return E_OUTOFMEMORY;

    if (FAILED(hr = wined3d_swapchain_state_init(s, desc, window, wined3d, state_parent)))
    {
        free(s);
        return hr;
    }

    *state = s;

    return hr;
}
