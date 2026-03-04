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

typedef enum {
    RM690B0_BMP_PIXEL_BGR888 = 0,
    RM690B0_BMP_PIXEL_RGB565 = 1,
    RM690B0_BMP_PIXEL_XRGB1555 = 2,
} rm690b0_bmp_pixel_format_t;

static inline uint16_t rm690b0_read_u16_le(const uint8_t *src) {
    return (uint16_t)src[0] | ((uint16_t)src[1] << 8);
}

static inline uint32_t rm690b0_read_u32_le(const uint8_t *src) {
    return (uint32_t)src[0]
        | ((uint32_t)src[1] << 8)
        | ((uint32_t)src[2] << 16)
        | ((uint32_t)src[3] << 24);
}

static inline uint16_t rm690b0_bmp_1555_to_rgb565(uint16_t pixel) {
    uint16_t r5 = (pixel >> 10) & 0x1F;
    uint16_t g5 = (pixel >> 5) & 0x1F;
    uint16_t b5 = pixel & 0x1F;
    uint16_t g6 = (uint16_t)((g5 << 1) | (g5 >> 4));
    return (uint16_t)((r5 << 11) | (g6 << 5) | b5);
}

static bool rm690b0_parse_bmp_pixel_format(const bmp_header_t *header, const uint8_t *bmp_data, size_t buf_len,
    rm690b0_bmp_pixel_format_t *out_pixel_format, size_t *out_bytes_per_pixel) {

    if (header->bpp == 24) {
        if (header->compression == 3) {
            if (header->bpp != 16 && header->bpp != 32) {
                mp_raise_ValueError(MP_ERROR_TEXT("Compression=3 only supported for 16/32 bpp"));
                return false;
            }
        } else if (header->compression != 0) {
            mp_raise_ValueError(MP_ERROR_TEXT("Compressed BMP not supported"));
            return false;
        }
        *out_pixel_format = RM690B0_BMP_PIXEL_BGR888;
        *out_bytes_per_pixel = 3;
        return true;
    }

    if (header->bpp != 16) {
        mp_raise_ValueError(MP_ERROR_TEXT("Only 16-bit and 24-bit BMP supported"));
        return false;
    }

    *out_bytes_per_pixel = 2;

    if (header->compression == 0) {
        // BI_RGB 16-bit BMP is conventionally X1R5G5B5.
        *out_pixel_format = RM690B0_BMP_PIXEL_XRGB1555;
        return true;
    }

    if (header->compression != 3) {
        mp_raise_ValueError(MP_ERROR_TEXT("Unsupported 16-bit BMP compression"));
        return false;
    }

    // BI_BITFIELDS: expect RGB masks at DIB + 40 (BITMAPINFOHEADER extension).
    size_t masks_offset = 14u + 40u;
    if (masks_offset + 12u > buf_len) {
        mp_raise_ValueError(MP_ERROR_TEXT("BMP bit masks truncated"));
        return false;
    }
    if ((size_t)header->offset < masks_offset + 12u) {
        mp_raise_ValueError(MP_ERROR_TEXT("Invalid BMP bit mask offset"));
        return false;
    }

    uint32_t mask_r = rm690b0_read_u32_le(bmp_data + masks_offset);
    uint32_t mask_g = rm690b0_read_u32_le(bmp_data + masks_offset + 4u);
    uint32_t mask_b = rm690b0_read_u32_le(bmp_data + masks_offset + 8u);

    if (mask_r == 0xF800 && mask_g == 0x07E0 && mask_b == 0x001F) {
        *out_pixel_format = RM690B0_BMP_PIXEL_RGB565;
        return true;
    }
    if (mask_r == 0x7C00 && mask_g == 0x03E0 && mask_b == 0x001F) {
        *out_pixel_format = RM690B0_BMP_PIXEL_XRGB1555;
        return true;
    }

    mp_raise_ValueError(MP_ERROR_TEXT("Unsupported 16-bit BMP bit masks"));
    return false;
}

static bool rm690b0_parse_bmp_layout(
    const uint8_t *bmp_data,
    const bmp_header_t *header,
    size_t buf_len,
    mp_int_t *out_width,
    mp_int_t *out_height,
    bool *out_top_down,
    size_t *out_data_offset,
    size_t *out_src_stride,
    size_t *out_bytes_per_pixel,
    rm690b0_bmp_pixel_format_t *out_pixel_format) {

    if (header->width <= 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("Invalid BMP width"));
        return false;
    }

    if (header->planes != 1) {
        mp_raise_ValueError(MP_ERROR_TEXT("Unsupported BMP color planes"));
        return false;
    }

    if (header->header_size < 40) {
        mp_raise_ValueError(MP_ERROR_TEXT("Unsupported BMP header"));
        return false;
    }

    if (header->height == 0 || header->height == INT32_MIN) {
        mp_raise_ValueError(MP_ERROR_TEXT("Invalid BMP height"));
        return false;
    }

    size_t bytes_per_pixel = 0;
    rm690b0_bmp_pixel_format_t pixel_format = RM690B0_BMP_PIXEL_BGR888;
    if (!rm690b0_parse_bmp_pixel_format(header, bmp_data, buf_len, &pixel_format, &bytes_per_pixel)) {
        return false;
    }
    uint64_t width_u64 = (uint64_t)(uint32_t)header->width;
    int64_t signed_height = header->height;
    uint64_t height_u64 = signed_height < 0 ? (uint64_t)(-signed_height) : (uint64_t)signed_height;
    uint64_t row_bytes_u64 = width_u64 * bytes_per_pixel;
    uint64_t row_padding_u64 = (4 - (row_bytes_u64 % 4)) % 4;
    uint64_t src_stride_u64 = row_bytes_u64 + row_padding_u64;

    if (src_stride_u64 > SIZE_MAX || height_u64 > SIZE_MAX) {
        mp_raise_ValueError(MP_ERROR_TEXT("BMP dimensions too large"));
        return false;
    }

    size_t data_offset = (size_t)header->offset;
    uint64_t min_data_offset_u64 = 14u + (uint64_t)header->header_size;
    if ((uint64_t)header->offset < min_data_offset_u64) {
        mp_raise_ValueError(MP_ERROR_TEXT("Invalid BMP data offset"));
        return false;
    }
    if (data_offset > buf_len) {
        mp_raise_ValueError(MP_ERROR_TEXT("Invalid BMP data offset"));
        return false;
    }

    uint64_t payload_u64 = height_u64 * src_stride_u64;
    if (payload_u64 > (uint64_t)(buf_len - data_offset)) {
        mp_raise_ValueError(MP_ERROR_TEXT("BMP data truncated"));
        return false;
    }

    *out_width = (mp_int_t)width_u64;
    *out_height = (mp_int_t)height_u64;
    *out_top_down = header->height < 0;
    *out_data_offset = data_offset;
    *out_src_stride = (size_t)src_stride_u64;
    if (out_bytes_per_pixel != NULL) {
        *out_bytes_per_pixel = bytes_per_pixel;
    }
    if (out_pixel_format != NULL) {
        *out_pixel_format = pixel_format;
    }
    return true;
}

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

    mp_int_t width = 0;
    mp_int_t height = 0;
    bool top_down = false;
    size_t data_offset = 0;
    size_t src_stride = 0;
    size_t bytes_per_pixel = 0;
    rm690b0_bmp_pixel_format_t pixel_format = RM690B0_BMP_PIXEL_BGR888;
    if (!rm690b0_parse_bmp_layout((const uint8_t *)bufinfo.buf, header, bufinfo.len, &width, &height, &top_down,
            &data_offset, &src_stride, &bytes_per_pixel, &pixel_format)) {
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

        for (mp_int_t row = 0; row < clip_h; row++) {
            mp_int_t src_y = y_offset + row;
            mp_int_t src_row_idx = top_down ? src_y : (height - 1 - src_y);
            const uint8_t *row_ptr = src_data + (size_t)src_row_idx * src_stride + (size_t)x_offset * bytes_per_pixel;

            uint16_t *dst_ptr = fb + (size_t)(clip_y + row) * fb_stride + clip_x;

            if (pixel_format == RM690B0_BMP_PIXEL_BGR888) {
                for (mp_int_t col = 0; col < clip_w; col++) {
                    size_t col3 = (size_t)col * 3;
                    uint8_t b = row_ptr[col3];
                    uint8_t g = row_ptr[col3 + 1];
                    uint8_t r = row_ptr[col3 + 2];
                    uint16_t rgb = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                    dst_ptr[col] = RGB565_SWAP_GB(rgb);
                }
            } else if (pixel_format == RM690B0_BMP_PIXEL_RGB565) {
                for (mp_int_t col = 0; col < clip_w; col++) {
                    size_t col2 = (size_t)col * 2;
                    uint16_t val = rm690b0_read_u16_le(row_ptr + col2);
                    dst_ptr[col] = RGB565_SWAP_GB(val);
                }
            } else {
                for (mp_int_t col = 0; col < clip_w; col++) {
                    size_t col2 = (size_t)col * 2;
                    uint16_t val_1555 = rm690b0_read_u16_le(row_ptr + col2);
                    dst_ptr[col] = RGB565_SWAP_GB(rm690b0_bmp_1555_to_rgb565(val_1555));
                }
            }
        }
    } else {
        for (mp_int_t row = 0; row < clip_h; row++) {
            mp_int_t src_y = y_offset + row;
            mp_int_t src_row_idx = top_down ? src_y : (height - 1 - src_y);
            const uint8_t *row_ptr = src_data + (size_t)src_row_idx * src_stride + (size_t)x_offset * bytes_per_pixel;

            for (mp_int_t col = 0; col < clip_w; col++) {
                uint16_t color565;
                if (pixel_format == RM690B0_BMP_PIXEL_BGR888) {
                    size_t col3 = (size_t)col * 3;
                    uint8_t b = row_ptr[col3];
                    uint8_t g = row_ptr[col3 + 1];
                    uint8_t r = row_ptr[col3 + 2];
                    uint16_t rgb = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                    color565 = RGB565_SWAP_GB(rgb);
                } else if (pixel_format == RM690B0_BMP_PIXEL_RGB565) {
                    size_t col2 = (size_t)col * 2;
                    uint16_t val = rm690b0_read_u16_le(row_ptr + col2);
                    color565 = RGB565_SWAP_GB(val);
                } else {
                    size_t col2 = (size_t)col * 2;
                    uint16_t val_1555 = rm690b0_read_u16_le(row_ptr + col2);
                    color565 = RGB565_SWAP_GB(rm690b0_bmp_1555_to_rgb565(val_1555));
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
    (void)self;

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

    mp_int_t width = 0;
    mp_int_t height = 0;
    bool top_down = false;
    size_t data_offset = 0;
    size_t src_stride = 0;
    rm690b0_bmp_pixel_format_t pixel_format = RM690B0_BMP_PIXEL_BGR888;
    if (!rm690b0_parse_bmp_layout((const uint8_t *)src_info.buf, header, src_info.len, &width, &height, &top_down,
            &data_offset, &src_stride, NULL, &pixel_format)) {
        return;
    }

    size_t needed_bytes = 0;
    if (!check_bitmap_size((size_t)width, (size_t)height, &needed_bytes)) {
        mp_raise_ValueError(MP_ERROR_TEXT("BMP dimensions too large"));
        return;
    }
    if (needed_bytes > dest_info.len) {
        mp_raise_ValueError(MP_ERROR_TEXT("Destination bitmap too small"));
        return;
    }

    uint16_t *dest_buf = (uint16_t *)dest_info.buf;
    const uint8_t *src_pixels = (const uint8_t *)src_info.buf + data_offset;

    for (mp_int_t row = 0; row < height; row++) {
         mp_int_t src_row_idx = top_down ? row : (height - 1 - row);
         const uint8_t *row_ptr = src_pixels + (size_t)src_row_idx * src_stride;
         uint16_t *dst_row_ptr = dest_buf + (size_t)row * width;

         if (pixel_format == RM690B0_BMP_PIXEL_BGR888) {
             for (mp_int_t col = 0; col < width; col++) {
                 size_t col3 = (size_t)col * 3;
                 uint8_t b = row_ptr[col3];
                 uint8_t g = row_ptr[col3 + 1];
                 uint8_t r = row_ptr[col3 + 2];
                 uint16_t rgb = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                 dst_row_ptr[col] = RGB565_SWAP_GB(rgb);
             }
         } else if (pixel_format == RM690B0_BMP_PIXEL_RGB565) {
             for (mp_int_t col = 0; col < width; col++) {
                 size_t col2 = (size_t)col * 2;
                 uint16_t val = rm690b0_read_u16_le(row_ptr + col2);
                 dst_row_ptr[col] = RGB565_SWAP_GB(val);
             }
         } else {
             for (mp_int_t col = 0; col < width; col++) {
                 size_t col2 = (size_t)col * 2;
                 uint16_t val_1555 = rm690b0_read_u16_le(row_ptr + col2);
                 dst_row_ptr[col] = RGB565_SWAP_GB(rm690b0_bmp_1555_to_rgb565(val_1555));
             }
         }
    }
}
