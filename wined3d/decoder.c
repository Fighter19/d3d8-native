/*
 * Copyright 2024 Elizabeth Figura for CodeWeavers
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
#include "wined3d_vk.h"

WINE_DEFAULT_DEBUG_CHANNEL(d3d);

struct wined3d_decoder
{
    LONG ref;
    struct wined3d_device *device;
    struct wined3d_decoder_desc desc;
    struct wined3d_buffer *bitstream, *parameters, *matrix, *slice_control;
    struct wined3d_decoder_output_view *output_view;

    /* Accessed from both the CS and application threads. */
    CRITICAL_SECTION feedback_cs;
    unsigned int feedback_number;
    uint8_t feedback_pic_entry;
    bool feedback_field;
};

static void wined3d_decoder_cleanup(struct wined3d_decoder *decoder)
{
    wined3d_buffer_decref(decoder->bitstream);
    wined3d_buffer_decref(decoder->parameters);
    wined3d_buffer_decref(decoder->matrix);
    wined3d_buffer_decref(decoder->slice_control);
    wined3d_lock_cleanup(&decoder->feedback_cs);
}

ULONG CDECL wined3d_decoder_decref(struct wined3d_decoder *decoder)
{
    unsigned int refcount = InterlockedDecrement(&decoder->ref);

    TRACE("%p decreasing refcount to %u.\n", decoder, refcount);

    if (!refcount)
    {
        wined3d_mutex_lock();
        decoder->device->adapter->decoder_ops->destroy(decoder);
        wined3d_mutex_unlock();
    }

    return refcount;
}

static bool is_supported_codec(struct wined3d_adapter *adapter, const GUID *codec)
{
    GUID profiles[WINED3D_DECODER_MAX_PROFILE_COUNT];
    unsigned int count;

    adapter->decoder_ops->get_profiles(adapter, &count, profiles);

    for (unsigned int i = 0; i < count; ++i)
    {
        if (IsEqualGUID(&profiles[i], codec))
            return true;
    }
    return false;
}

static HRESULT wined3d_decoder_init(struct wined3d_decoder *decoder,
        struct wined3d_device *device, const struct wined3d_decoder_desc *desc)
{
    HRESULT hr;

    struct wined3d_buffer_desc buffer_desc =
    {
        .access = WINED3D_RESOURCE_ACCESS_CPU | WINED3D_RESOURCE_ACCESS_MAP_R | WINED3D_RESOURCE_ACCESS_MAP_W,
    };

    decoder->ref = 1;
    decoder->device = device;
    decoder->desc = *desc;
    wined3d_lock_init(&decoder->feedback_cs, "wined3d_decoder.feedback_cs");

    buffer_desc.byte_width = sizeof(DXVA_PicParams_H264);
    if (FAILED(hr = wined3d_buffer_create(device, &buffer_desc,
            NULL, NULL, &wined3d_null_parent_ops, &decoder->parameters)))
        return hr;

    buffer_desc.byte_width = sizeof(DXVA_Qmatrix_H264);
    if (FAILED(hr = wined3d_buffer_create(device, &buffer_desc,
            NULL, NULL, &wined3d_null_parent_ops, &decoder->matrix)))
    {
        wined3d_buffer_decref(decoder->parameters);
        return hr;
    }

    /* NVidia gives 64 * sizeof(DXVA_Slice_H264_Long).
     * AMD gives 4096 bytes. Pick the smaller one. */
    buffer_desc.byte_width = 4096;
    if (FAILED(hr = wined3d_buffer_create(device, &buffer_desc,
            NULL, NULL, &wined3d_null_parent_ops, &decoder->slice_control)))
    {
        wined3d_buffer_decref(decoder->matrix);
        wined3d_buffer_decref(decoder->parameters);
        return hr;
    }

    /* NVidia makes this buffer as large as width * height (as if each pixel
     * is at most 1 byte). AMD makes it larger than that.
     * Go with the smaller of the two. */
    buffer_desc.byte_width = desc->width * desc->height;
    buffer_desc.bind_flags = WINED3D_BIND_DECODER_SRC;
    buffer_desc.access = WINED3D_RESOURCE_ACCESS_GPU | WINED3D_RESOURCE_ACCESS_MAP_W;
    buffer_desc.usage = WINED3DUSAGE_DYNAMIC;

    if (FAILED(hr = wined3d_buffer_create(device, &buffer_desc,
            NULL, NULL, &wined3d_null_parent_ops, &decoder->bitstream)))
    {
        wined3d_buffer_decref(decoder->matrix);
        wined3d_buffer_decref(decoder->parameters);
        wined3d_buffer_decref(decoder->slice_control);
        return hr;
    }

    return S_OK;
}

HRESULT CDECL wined3d_decoder_create(struct wined3d_device *device,
        const struct wined3d_decoder_desc *desc, struct wined3d_decoder **decoder)
{
    TRACE("device %p, codec %s, size %ux%u, output_format %s, decoder %p.\n", device,
            debugstr_guid(&desc->codec), desc->width, desc->height, debug_d3dformat(desc->output_format), decoder);

    if (!is_supported_codec(device->adapter, &desc->codec))
    {
        WARN("Codec %s is not supported; returning E_INVALIDARG.\n", debugstr_guid(&desc->codec));
        return E_INVALIDARG;
    }

    return device->adapter->decoder_ops->create(device, desc, decoder);
}

static void wined3d_null_decoder_get_profiles(struct wined3d_adapter *adapter, unsigned int *count, GUID *profiles)
{
    *count = 0;
}

const struct wined3d_decoder_ops wined3d_null_decoder_ops =
{
    .get_profiles = wined3d_null_decoder_get_profiles,
};

struct wined3d_resource * CDECL wined3d_decoder_get_buffer(
        struct wined3d_decoder *decoder, enum wined3d_decoder_buffer_type type)
{
    switch (type)
    {
        case WINED3D_DECODER_BUFFER_BITSTREAM:
            return &decoder->bitstream->resource;

        case WINED3D_DECODER_BUFFER_INVERSE_QUANTIZATION_MATRIX:
            return &decoder->matrix->resource;

        case WINED3D_DECODER_BUFFER_PICTURE_PARAMETERS:
            return &decoder->parameters->resource;

        case WINED3D_DECODER_BUFFER_SLICE_CONTROL:
            return &decoder->slice_control->resource;
    }

    FIXME("Unhandled buffer type %#x.\n", type);
    return NULL;
}

HRESULT CDECL wined3d_decoder_begin_frame(struct wined3d_decoder *decoder,
        struct wined3d_decoder_output_view *view)
{
    TRACE("decoder %p, view %p.\n", decoder, view);

    if (decoder->output_view)
    {
        ERR("Already in frame.\n");
        return E_INVALIDARG;
    }

    wined3d_decoder_output_view_incref(view);
    decoder->output_view = view;

    return S_OK;
}

HRESULT CDECL wined3d_decoder_end_frame(struct wined3d_decoder *decoder)
{
    TRACE("decoder %p.\n", decoder);

    if (!decoder->output_view)
    {
        ERR("Not in frame.\n");
        return E_INVALIDARG;
    }

    wined3d_decoder_output_view_decref(decoder->output_view);
    decoder->output_view = NULL;

    return S_OK;
}

HRESULT CDECL wined3d_decoder_decode(struct wined3d_decoder *decoder,
        unsigned int bitstream_size, unsigned int slice_control_size)
{
    TRACE("decoder %p, bitstream_size %u, slice_control_size %u.\n", decoder, bitstream_size, slice_control_size);

    if (!decoder->output_view)
    {
        ERR("Not in frame.\n");
        return E_INVALIDARG;
    }

    wined3d_cs_emit_decode(decoder, decoder->output_view, bitstream_size, slice_control_size);
    return S_OK;
}

HRESULT CDECL wined3d_decoder_extension(struct wined3d_decoder *decoder, unsigned int function,
        const void *input, unsigned int input_size, void *output, unsigned int output_size)
{
    TRACE("decoder %p, function %#x, input %p, input_size %u, output %p, output_size %u.\n",
            decoder, function, input, input_size, output, output_size);

    if (function == DXVA_STATUS_REPORTING_FUNCTION)
    {
        DXVA_Status_H264 *status = output;
        unsigned int feedback_number;
        uint8_t feedback_pic_entry;
        bool feedback_field;

        if (output_size < sizeof(*status))
        {
            WARN("Invalid size %u.\n", output_size);
            return E_FAIL;
        }

        /* FIXME: We should use Vulkan result status queries here. */

        /* In lieu of real status information, we report the frame done as soon
         * as the command has been submitted. That's earlier than it should be,
         * but the application can't tell the difference. */

        EnterCriticalSection(&decoder->feedback_cs);
        feedback_number = decoder->feedback_number;
        feedback_pic_entry = decoder->feedback_pic_entry;
        feedback_field = decoder->feedback_field;
        LeaveCriticalSection(&decoder->feedback_cs);

        /* AMD returns E_FAIL here; NVidia returns S_OK with zeroed output. */
        if (!feedback_number)
            return E_FAIL;

        status->StatusReportFeedbackNumber = feedback_number;
        status->CurrPic.bPicEntry = feedback_pic_entry;
        status->field_pic_flag = feedback_field;
        status->bDXVA_Func = DXVA_PICTURE_DECODING_FUNCTION;
        status->bBufType = (UCHAR)~0;
        status->bStatus = 0;
        status->bReserved8Bits = 0;
        /* 0xffff means no estimate provided. NVidia returns this.
         * AMD returns 1, which is obviously wrong. */
        status->wNumMbsAffected = 0xffff;
        return S_OK;
    }
    else
    {
        FIXME("Unhandled function %#x.\n", function);
        return E_NOTIMPL;
    }
}
