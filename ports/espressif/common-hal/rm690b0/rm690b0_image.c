// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

// Image support: BMP blitting, JPEG decoding, BMP→RGB565 conversion.

#include "rm690b0_internal.h"
#include "esp_jpeg.h"

// ============================================================================
// BMP support
// ============================================================================

#pragma pack(push, 1)
typedef struct {
    uint16_t type;             // Magic identifier: 0x4d42
    uint32_t size;             // File size in bytes
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;           // Offset to image data in bytes from beginning of file
    uint32_t header_size;      // Header size in bytes
    int32_t width;             // Width of the image
    int32_t height;            // Height of the image
    uint16_t planes;           // Number of color planes
    uint16_t bpp;              // Bits per pixel
    uint32_t compression;      // Compression type
    uint32_t image_size;       // Image size in bytes
    int32_t x_res;             // Pixels per meter
    int32_t y_res;             // Pixels per meter
    uint32_t n_colors;         // Number of colors
    uint32_t important_colors; // Important colors
} bmp_header_t;
#pragma pack(pop)

void common_hal_rm690b0_rm690b0_blit_bmp(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_obj_t bmp_data) {
    CHECK_INITIALIZED();

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(bmp_data, &bufinfo, MP_BUFFER_READ);

    if (bufinfo.len < sizeof(bmp_header_t)) {
        mp_raise_ValueError(MP_ERROR_TEXT("BMP data too small"));
        return;
    }

    const bmp_header_t *header = (const bmp_header_t *)bufinfo.buf;

    if (header->type != 0x4D42) { // 'BM'
        mp_raise_ValueError(MP_ERROR_TEXT("Invalid BMP header"));
        return;
    }

    if (header->bpp != 24 && header->bpp != 16) {
        mp_raise_ValueError(MP_ERROR_TEXT("Only 16-bit and 24-bit BMP supported"));
        return;
    }

    if (header->compression != 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("Compressed BMP not supported"));
        return;
    }

    if (header->width <= 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("Invalid BMP width"));
        return;
    }

    mp_int_t width = header->width;
    mp_int_t height = abs(header->height);
    bool top_down = (header->height < 0);
    size_t data_offset = header->offset;

    if (data_offset >= bufinfo.len) {
        mp_raise_ValueError(MP_ERROR_TEXT("Invalid BMP data offset"));
        return;
    }

    int row_padding = (4 - ((width * (header->bpp / 8)) % 4)) % 4;
    int src_stride = width * (header->bpp / 8) + row_padding;

    if (data_offset + (size_t)height * src_stride > bufinfo.len) {
        mp_raise_ValueError(MP_ERROR_TEXT("BMP data truncated"));
        return;
    }

    mp_int_t clip_x = x;
    mp_int_t clip_y = y;
    mp_int_t clip_w = width;
    mp_int_t clip_h = height;

    if (!clip_logical_rect(self, &clip_x, &clip_y, &clip_w, &clip_h)) {
        return;
    }

    mp_int_t x_offset = clip_x - x;
    mp_int_t y_offset = clip_y - y;

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    const uint8_t *src_data = (const uint8_t *)bufinfo.buf + data_offset;

    if (self->rotation == 0) {
        size_t fb_stride = RM690B0_PANEL_WIDTH;
        uint16_t *fb = impl->framebuffer;

        for (int row = 0; row < clip_h; row++) {
            int src_y = y_offset + row;
            int src_row_idx = top_down ? src_y : (height - 1 - src_y);
            const uint8_t *row_ptr = src_data + (size_t)src_row_idx * src_stride + (size_t)x_offset * (header->bpp / 8);

            uint16_t *dst_ptr = fb + (size_t)(clip_y + row) * fb_stride + clip_x;

            if (header->bpp == 24) {
                for (int col = 0; col < clip_w; col++) {
                    uint8_t b = row_ptr[col * 3];
                    uint8_t g = row_ptr[col * 3 + 1];
                    uint8_t r = row_ptr[col * 3 + 2];
                    uint16_t rgb = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                    dst_ptr[col] = RGB565_SWAP_GB(rgb);
                }
            } else { // 16-bit
                for (int col = 0; col < clip_w; col++) {
                    uint16_t val = row_ptr[col * 2] | (row_ptr[col * 2 + 1] << 8);
                    dst_ptr[col] = RGB565_SWAP_GB(val);
                }
            }
        }
    } else {
        for (int row = 0; row < clip_h; row++) {
            int src_y = y_offset + row;
            int src_row_idx = top_down ? src_y : (height - 1 - src_y);
            const uint8_t *row_ptr = src_data + (size_t)src_row_idx * src_stride + (size_t)x_offset * (header->bpp / 8);

            for (int col = 0; col < clip_w; col++) {
                uint16_t color565;
                if (header->bpp == 24) {
                    uint8_t b = row_ptr[col * 3];
                    uint8_t g = row_ptr[col * 3 + 1];
                    uint8_t r = row_ptr[col * 3 + 2];
                    uint16_t rgb = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                    color565 = RGB565_SWAP_GB(rgb);
                } else {
                    uint16_t val = row_ptr[col * 2] | (row_ptr[col * 2 + 1] << 8);
                    color565 = RGB565_SWAP_GB(val);
                }
                rm690b0_write_pixel_rotated(self, impl, clip_x + col, clip_y + row, color565);
            }
        }
    }

    mp_int_t bx = clip_x, by = clip_y, bw = clip_w, bh = clip_h;
    if (map_rect_for_rotation(self, &bx, &by, &bw, &bh)) {
        esp_err_t ret = rm690b0_finalize_draw(self, impl, bx, by, bw, bh);
        if (ret != ESP_OK) {
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to draw BMP: %s"), esp_err_to_name(ret));
        }
    }
}

// ============================================================================
// JPEG support
// ============================================================================

static esp_err_t rm690b0_jpeg_on_block(intptr_t ctx_ptr,
    uint32_t block_top, uint32_t block_left,
    uint32_t block_bottom, uint32_t block_right,
    const uint16_t *pixels) {

    rm690b0_jpeg_draw_ctx_t *ctx = (rm690b0_jpeg_draw_ctx_t *)ctx_ptr;
    rm690b0_impl_t *impl = ctx->impl;
    if (impl == NULL || impl->framebuffer == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    mp_int_t dest_left = ctx->origin_x + (mp_int_t)block_left;
    mp_int_t dest_top = ctx->origin_y + (mp_int_t)block_top;
    mp_int_t dest_right = ctx->origin_x + (mp_int_t)block_right;
    mp_int_t dest_bottom = ctx->origin_y + (mp_int_t)block_bottom;

    mp_int_t clip_right = ctx->clip_x + ctx->clip_w - 1;
    mp_int_t clip_bottom = ctx->clip_y + ctx->clip_h - 1;

    mp_int_t draw_left = dest_left > ctx->clip_x ? dest_left : ctx->clip_x;
    mp_int_t draw_top = dest_top > ctx->clip_y ? dest_top : ctx->clip_y;
    mp_int_t draw_right = dest_right < clip_right ? dest_right : clip_right;
    mp_int_t draw_bottom = dest_bottom < clip_bottom ? dest_bottom : clip_bottom;

    if (draw_left > draw_right || draw_top > draw_bottom) {
        return ESP_OK;
    }

    mp_int_t block_w = (mp_int_t)(block_right - block_left + 1);
    if (block_w <= 0) {
        return ESP_OK;
    }

    size_t fb_stride = RM690B0_PANEL_WIDTH;

    for (mp_int_t row = draw_top; row <= draw_bottom; row++) {
        mp_int_t src_row = row - dest_top;
        const uint16_t *row_src = pixels + (size_t)src_row * block_w + (draw_left - dest_left);

        if (ctx->rotation_zero) {
            uint16_t *dst = impl->framebuffer + (size_t)row * fb_stride + draw_left;
            for (mp_int_t col = draw_left; col <= draw_right; col++) {
                dst[col - draw_left] = RGB565_SWAP_GB(*row_src++);
            }
        } else {
            for (mp_int_t col = draw_left; col <= draw_right; col++) {
                uint16_t color = RGB565_SWAP_GB(*row_src++);
                rm690b0_write_pixel_rotated(ctx->self, impl, col, row, color);
            }
        }
    }

    return ESP_OK;
}


void common_hal_rm690b0_rm690b0_blit_jpeg(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_obj_t jpeg_data) {
    CHECK_INITIALIZED();

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(jpeg_data, &bufinfo, MP_BUFFER_READ);

    esp_jpeg_image_cfg_t jpeg_cfg = {
        .indata = (uint8_t *)bufinfo.buf,
        .indata_size = bufinfo.len,
        .out_format = JPEG_IMAGE_FORMAT_RGB565,
        .out_scale = JPEG_IMAGE_SCALE_0,
        .flags = { .swap_color_bytes = 0 },
        .outbuf = NULL,
        .outbuf_size = 0,
        .user_data = 0,
        .on_block = NULL,
    };

    esp_jpeg_image_output_t jpeg_out;
    esp_err_t ret = esp_jpeg_get_image_info(&jpeg_cfg, &jpeg_out);
    if (ret != ESP_OK) {
        mp_raise_ValueError(MP_ERROR_TEXT("Invalid JPEG data"));
        return;
    }

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || impl->framebuffer == NULL) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Invalid display handle"));
        return;
    }

    int width = jpeg_out.width;
    int height = jpeg_out.height;

    mp_int_t clip_x = x;
    mp_int_t clip_y = y;
    mp_int_t clip_w = width;
    mp_int_t clip_h = height;

    if (clip_logical_rect(self, &clip_x, &clip_y, &clip_w, &clip_h)) {
        rm690b0_jpeg_draw_ctx_t draw_ctx = {
            .self = self,
            .impl = impl,
            .origin_x = x,
            .origin_y = y,
            .clip_x = clip_x,
            .clip_y = clip_y,
            .clip_w = clip_w,
            .clip_h = clip_h,
            .rotation_zero = (self->rotation == 0),
        };

        jpeg_cfg.user_data = (intptr_t)&draw_ctx;
        jpeg_cfg.on_block = rm690b0_jpeg_on_block;

        ret = esp_jpeg_decode(&jpeg_cfg, NULL);
        if (ret != ESP_OK) {
            mp_raise_ValueError(MP_ERROR_TEXT("JPEG decode failed"));
            return;
        }

        mp_int_t bx = clip_x, by = clip_y, bw = clip_w, bh = clip_h;
        if (map_rect_for_rotation(self, &bx, &by, &bw, &bh)) {
            esp_err_t flush_ret = rm690b0_finalize_draw(self, impl, bx, by, bw, bh);
            if (flush_ret != ESP_OK) {
                mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to draw JPEG: %s"), esp_err_to_name(flush_ret));
            }
        }
    }
}

// ============================================================================
// BMP → RGB565 conversion (no display write)
// ============================================================================

void common_hal_rm690b0_rm690b0_convert_bmp(rm690b0_rm690b0_obj_t *self, mp_obj_t src_data, mp_obj_t dest_bitmap) {
    mp_buffer_info_t src_info;
    mp_get_buffer_raise(src_data, &src_info, MP_BUFFER_READ);

    mp_buffer_info_t dest_info;
    mp_get_buffer_raise(dest_bitmap, &dest_info, MP_BUFFER_WRITE);

    if (src_info.len < sizeof(bmp_header_t)) {
        mp_raise_ValueError(MP_ERROR_TEXT("BMP data too small"));
        return;
    }

    const bmp_header_t *header = (const bmp_header_t *)src_info.buf;

    if (header->type != 0x4D42) { // 'BM'
        mp_raise_ValueError(MP_ERROR_TEXT("Invalid BMP header"));
        return;
    }

    if (header->bpp != 24 && header->bpp != 16) {
        mp_raise_ValueError(MP_ERROR_TEXT("Only 16-bit and 24-bit BMP supported"));
        return;
    }

    if (header->compression != 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("Compressed BMP not supported"));
        return;
    }

    if (header->width <= 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("Invalid BMP width"));
        return;
    }

    mp_int_t width = header->width;
    mp_int_t height = abs(header->height);
    bool top_down = (header->height < 0);
    size_t data_offset = header->offset;

    if (data_offset >= src_info.len) {
        mp_raise_ValueError(MP_ERROR_TEXT("Invalid BMP data offset"));
        return;
    }

    int row_padding = (4 - ((width * (header->bpp / 8)) % 4)) % 4;
    int src_stride = width * (header->bpp / 8) + row_padding;

    if (data_offset + (size_t)height * src_stride > src_info.len) {
        mp_raise_ValueError(MP_ERROR_TEXT("BMP data truncated"));
        return;
    }

    size_t max_dest_pixels = dest_info.len / sizeof(uint16_t);
    if ((size_t)width * (size_t)height > max_dest_pixels) {
         mp_raise_ValueError(MP_ERROR_TEXT("Destination bitmap too small"));
         return;
    }

    uint16_t *dest_buf = (uint16_t *)dest_info.buf;
    const uint8_t *src_pixels = (const uint8_t *)src_info.buf + data_offset;

    for (int row = 0; row < height; row++) {
         int src_row_idx = top_down ? row : (height - 1 - row);
         const uint8_t *row_ptr = src_pixels + (size_t)src_row_idx * src_stride;
         uint16_t *dst_row_ptr = dest_buf + (size_t)row * width;

         if (header->bpp == 24) {
             for (int col = 0; col < width; col++) {
                 uint8_t b = row_ptr[col * 3];
                 uint8_t g = row_ptr[col * 3 + 1];
                 uint8_t r = row_ptr[col * 3 + 2];
                 uint16_t rgb = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                 dst_row_ptr[col] = RGB565_SWAP_GB(rgb);
             }
         } else {
             for (int col = 0; col < width; col++) {
                 uint16_t val = row_ptr[col * 2] | (row_ptr[col * 2 + 1] << 8);
                 dst_row_ptr[col] = RGB565_SWAP_GB(val);
             }
         }
    }
}
