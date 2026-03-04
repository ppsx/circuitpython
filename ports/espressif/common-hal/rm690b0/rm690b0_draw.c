// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

// Drawing primitives: pixel, fill_color, fill_rect, hline, vline, rect,
// line, circle, fill_circle, blit_buffer.

#include "rm690b0_internal.h"

// ============================================================================
// fill_color
// ============================================================================

void common_hal_rm690b0_rm690b0_fill_color(rm690b0_rm690b0_obj_t *self, uint16_t color) {
    CHECK_INITIALIZED();

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl != NULL && !impl->double_buffered) {
        rm690b0_fill_color_direct(self, color);
        return;
    }

    common_hal_rm690b0_rm690b0_fill_rect(self, 0, 0, self->width, self->height, color);
}

// ============================================================================
// pixel
// ============================================================================

void common_hal_rm690b0_rm690b0_pixel(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_int_t color) {
    CHECK_INITIALIZED();

    mp_int_t bx = x;
    mp_int_t by = y;
    mp_int_t bw = 1;
    mp_int_t bh = 1;

    if (!rm690b0_prepare_draw(self, &bx, &by, &bw, &bh)) {
        return;
    }

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || impl->framebuffer == NULL) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Invalid display handle"));
        return;
    }

    uint16_t swapped_color = RGB565_SWAP_GB(color);
    impl->framebuffer[(size_t)by * RM690B0_PANEL_WIDTH + bx] = swapped_color;

    esp_err_t ret = rm690b0_finalize_draw(self, impl, bx, by, bw, bh);
    if (ret != ESP_OK) {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to draw pixel: %s"), esp_err_to_name(ret));
    }
}

// ============================================================================
// fill_rect
// ============================================================================

void common_hal_rm690b0_rm690b0_fill_rect(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_int_t width, mp_int_t height, mp_int_t color) {
    CHECK_INITIALIZED();

    mp_int_t bx = x;
    mp_int_t by = y;
    mp_int_t bw = width;
    mp_int_t bh = height;

    if (!rm690b0_prepare_draw(self, &bx, &by, &bw, &bh)) {
        return;
    }

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || impl->framebuffer == NULL) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Invalid display handle"));
        return;
    }

    uint16_t swapped_color = RGB565_SWAP_GB(color);

    if (!impl->double_buffered && bx == 0 && bw == RM690B0_PANEL_WIDTH) {
        esp_err_t direct_ret = rm690b0_fill_rect_direct_fullwidth(self, by, bh, swapped_color);
        if (direct_ret == ESP_OK) {
            mark_dirty_region(impl, bx, by, bw, bh);
            return;
        }
        ESP_LOGW(TAG, "Full-width fast fill_rect path failed (%s) – falling back",
            esp_err_to_name(direct_ret));
    }

    rm690b0_fill_rect_framebuffer(impl, bx, by, bw, bh, swapped_color);

    esp_err_t ret = rm690b0_finalize_draw(self, impl, bx, by, bw, bh);
    if (ret != ESP_OK) {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to draw fill_rect: %s"), esp_err_to_name(ret));
    }
}

// ============================================================================
// hline, vline, rect
// ============================================================================

void common_hal_rm690b0_rm690b0_hline(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_int_t width, mp_int_t color) {
    common_hal_rm690b0_rm690b0_fill_rect(self, x, y, width, 1, color);
}

void common_hal_rm690b0_rm690b0_vline(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_int_t height, mp_int_t color) {
    common_hal_rm690b0_rm690b0_fill_rect(self, x, y, 1, height, color);
}

void common_hal_rm690b0_rm690b0_rect(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_int_t width, mp_int_t height, mp_int_t color) {
    if (width <= 0 || height <= 0) {
        return;
    }
    common_hal_rm690b0_rm690b0_hline(self, x, y, width, color);
    common_hal_rm690b0_rm690b0_hline(self, x, y + height - 1, width, color);
    common_hal_rm690b0_rm690b0_vline(self, x, y, height, color);
    common_hal_rm690b0_rm690b0_vline(self, x + width - 1, y, height, color);
}

// ============================================================================
// line
// ============================================================================

#define CS_INSIDE 0
#define CS_LEFT 1
#define CS_RIGHT 2
#define CS_BOTTOM 4
#define CS_TOP 8

static inline int compute_outcode(mp_int_t x, mp_int_t y, mp_int_t w, mp_int_t h) {
    int code = CS_INSIDE;
    if (x < 0) code |= CS_LEFT;
    else if (x >= w) code |= CS_RIGHT;
    if (y < 0) code |= CS_TOP;
    else if (y >= h) code |= CS_BOTTOM;
    return code;
}

static void rm690b0_draw_line_segment(rm690b0_rm690b0_obj_t *self, mp_int_t x0, mp_int_t y0, mp_int_t x1, mp_int_t y1, mp_int_t color) {
    int outcode0 = compute_outcode(x0, y0, self->width, self->height);
    int outcode1 = compute_outcode(x1, y1, self->width, self->height);
    bool accept = false;

    while (true) {
        if (!(outcode0 | outcode1)) {
            accept = true;
            break;
        } else if (outcode0 & outcode1) {
            break;
        } else {
            int outcodeOut = outcode0 ? outcode0 : outcode1;
            mp_int_t x = 0, y = 0;
            if (outcodeOut & CS_BOTTOM) {
                x = x0 + (x1 - x0) * (self->height - 1 - y0) / (y1 - y0);
                y = self->height - 1;
            } else if (outcodeOut & CS_TOP) {
                x = x0 + (x1 - x0) * (0 - y0) / (y1 - y0);
                y = 0;
            } else if (outcodeOut & CS_RIGHT) {
                y = y0 + (y1 - y0) * (self->width - 1 - x0) / (x1 - x0);
                x = self->width - 1;
            } else if (outcodeOut & CS_LEFT) {
                y = y0 + (y1 - y0) * (0 - x0) / (x1 - x0);
                x = 0;
            }
            if (outcodeOut == outcode0) {
                x0 = x; y0 = y; outcode0 = compute_outcode(x0, y0, self->width, self->height);
            } else {
                x1 = x; y1 = y; outcode1 = compute_outcode(x1, y1, self->width, self->height);
            }
        }
    }
    if (!accept) return;

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || impl->framebuffer == NULL) {
        return;
    }

    uint16_t swapped_color = RGB565_SWAP_GB(color);
    size_t fb_stride = RM690B0_PANEL_WIDTH;

    mp_int_t px0, py0, px1, py1;
    rm690b0_map_point(self, x0, y0, &px0, &py0);
    rm690b0_map_point(self, x1, y1, &px1, &py1);

    mp_int_t pdx = labs(px1 - px0);
    mp_int_t pdy = labs(py1 - py0);
    mp_int_t sx = (px0 < px1) ? 1 : -1;
    mp_int_t sy = (py0 < py1) ? 1 : -1;
    mp_int_t err = pdx - pdy;

    mp_int_t x = px0;
    mp_int_t y = py0;
    mp_int_t dirty_min_x = x, dirty_min_y = y;
    mp_int_t dirty_max_x = x, dirty_max_y = y;

    while (1) {
        if (x < dirty_min_x) {
            dirty_min_x = x;
        }
        if (x > dirty_max_x) {
            dirty_max_x = x;
        }
        if (y < dirty_min_y) {
            dirty_min_y = y;
        }
        if (y > dirty_max_y) {
            dirty_max_y = y;
        }

        if (x >= 0 && x < RM690B0_PANEL_WIDTH && y >= 0 && y < RM690B0_PANEL_HEIGHT) {
            impl->framebuffer[(size_t)y * fb_stride + x] = swapped_color;
        }

        if (x == px1 && y == py1) {
            break;
        }
        mp_int_t e2 = 2 * err;
        if (e2 > -pdy) {
            err -= pdy;
            x += sx;
        }
        if (e2 < pdx) {
            err += pdx;
            y += sy;
        }
    }

    mp_int_t bx = dirty_min_x;
    mp_int_t by = dirty_min_y;
    mp_int_t bw = dirty_max_x - dirty_min_x + 1;
    mp_int_t bh = dirty_max_y - dirty_min_y + 1;

    if (bx < 0) {
        bw += bx;
        bx = 0;
    }
    if (by < 0) {
        bh += by;
        by = 0;
    }
    if (bx + bw > RM690B0_PANEL_WIDTH) {
        bw = RM690B0_PANEL_WIDTH - bx;
    }
    if (by + bh > RM690B0_PANEL_HEIGHT) {
        bh = RM690B0_PANEL_HEIGHT - by;
    }

    if (bw > 0 && bh > 0) {
        rm690b0_finalize_draw(self, impl, bx, by, bw, bh);
    }
}

void common_hal_rm690b0_rm690b0_line(rm690b0_rm690b0_obj_t *self, mp_int_t x0, mp_int_t y0, mp_int_t x1, mp_int_t y1, mp_int_t color) {
    CHECK_INITIALIZED();

    if (x0 == x1) {
        if (y1 < y0) {
            mp_int_t tmp = y0;
            y0 = y1;
            y1 = tmp;
        }
        common_hal_rm690b0_rm690b0_vline(self, x0, y0, y1 - y0 + 1, color);
        return;
    }
    if (y0 == y1) {
        if (x1 < x0) {
            mp_int_t tmp = x0;
            x0 = x1;
            x1 = tmp;
        }
        common_hal_rm690b0_rm690b0_hline(self, x0, y0, x1 - x0 + 1, color);
        return;
    }

    rm690b0_draw_line_segment(self, x0, y0, x1, y1, color);
}

// ============================================================================
// circle
// ============================================================================

void common_hal_rm690b0_rm690b0_circle(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_int_t radius, mp_int_t color) {
    CHECK_INITIALIZED();
    if (radius < 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("radius must be non-negative"));
        return;
    }
    if (radius == 0) {
        common_hal_rm690b0_rm690b0_pixel(self, x, y, color);
        return;
    }

    if (x + radius < 0 || x - radius >= self->width || y + radius < 0 || y - radius >= self->height) {
        return;
    }

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || impl->framebuffer == NULL) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Invalid display handle"));
        return;
    }

    uint16_t swapped_color = RGB565_SWAP_GB(color);

    mp_int_t x0 = 0;
    mp_int_t y0 = radius;
    mp_int_t d = 1 - radius;
    while (x0 <= y0) {
        rm690b0_write_pixel_rotated(self, impl, x + x0, y + y0, swapped_color);
        rm690b0_write_pixel_rotated(self, impl, x - x0, y + y0, swapped_color);
        rm690b0_write_pixel_rotated(self, impl, x + x0, y - y0, swapped_color);
        rm690b0_write_pixel_rotated(self, impl, x - x0, y - y0, swapped_color);
        rm690b0_write_pixel_rotated(self, impl, x + y0, y + x0, swapped_color);
        rm690b0_write_pixel_rotated(self, impl, x - y0, y + x0, swapped_color);
        rm690b0_write_pixel_rotated(self, impl, x + y0, y - x0, swapped_color);
        rm690b0_write_pixel_rotated(self, impl, x - y0, y - x0, swapped_color);
        x0 += 1;
        if (d < 0) {
            d += (x0 << 1) + 1;
        } else {
            y0 -= 1;
            d += ((x0 - y0) << 1) + 1;
        }
    }

    mp_int_t bx = x - radius;
    mp_int_t by = y - radius;
    mp_int_t bw = radius * 2 + 1;
    mp_int_t bh = radius * 2 + 1;
    if (rm690b0_prepare_draw(self, &bx, &by, &bw, &bh)) {
        esp_err_t ret = rm690b0_finalize_draw(self, impl, bx, by, bw, bh);
        if (ret != ESP_OK) {
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to draw circle: %s"), esp_err_to_name(ret));
        }
    }
}

// ============================================================================
// fill_circle
// ============================================================================

void common_hal_rm690b0_rm690b0_fill_circle(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_int_t radius, mp_int_t color) {
    CHECK_INITIALIZED();
    if (radius < 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("radius must be non-negative"));
        return;
    }
    if (radius == 0) {
        common_hal_rm690b0_rm690b0_pixel(self, x, y, color);
        return;
    }

    mp_int_t max_radius = (RM690B0_MAX_DIAMETER - 1) / 2;
    if (radius > max_radius) {
        radius = max_radius;
    }

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || impl->framebuffer == NULL) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Invalid display handle"));
        return;
    }

    mp_int_t top = y - radius;
    mp_int_t row_count = radius * 2 + 1;
    if (row_count <= 0) {
        return;
    }

    mp_int_t bx = x - radius;
    mp_int_t by = y - radius;
    mp_int_t bw = row_count;
    mp_int_t bh = row_count;

    if (bx >= self->width || by >= self->height || bx + bw <= 0 || by + bh <= 0) {
        return;
    }

    bool circle_fully_inside = (bx >= 0 && by >= 0 &&
        bx + bw <= self->width && by + bh <= self->height);

    #define STACK_ALLOC_THRESHOLD 128
    int16_t left_stack[STACK_ALLOC_THRESHOLD];
    int16_t right_stack[STACK_ALLOC_THRESHOLD];
    int16_t *left = left_stack;
    int16_t *right = right_stack;
    int16_t *heap_span = NULL;

    if (row_count > STACK_ALLOC_THRESHOLD) {
        int16_t *cache = rm690b0_acquire_span_cache(impl, (size_t)row_count);
        if (cache != NULL) {
            left = cache;
            right = cache + (mp_int_t)impl->circle_span_capacity;
        } else {
            size_t span_entries = (size_t)row_count * 2;
            heap_span = (int16_t *)heap_caps_malloc(span_entries * sizeof(int16_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
            if (heap_span == NULL) {
                mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to allocate memory for circle"));
                return;
            }
            left = heap_span;
            right = heap_span + row_count;
        }
    }

    for (mp_int_t i = 0; i < row_count; i++) {
        left[i] = INT16_MAX;
        right[i] = INT16_MIN;
    }

    rm690b0_span_accumulator_t acc = {
        .top = top,
        .row_count = row_count,
        .left = left,
        .right = right,
    };

    mp_int_t xi = 0;
    mp_int_t yi = radius;
    mp_int_t d = 1 - radius;

    while (xi <= yi) {
        rm690b0_span_update(&acc, y + yi, x + xi);
        rm690b0_span_update(&acc, y + yi, x - xi);
        rm690b0_span_update(&acc, y - yi, x + xi);
        rm690b0_span_update(&acc, y - yi, x - xi);

        if (yi != xi) {
            rm690b0_span_update(&acc, y + xi, x + yi);
            rm690b0_span_update(&acc, y + xi, x - yi);
            rm690b0_span_update(&acc, y - xi, x + yi);
            rm690b0_span_update(&acc, y - xi, x - yi);
        }

        xi += 1;
        if (d < 0) {
            d += (xi << 1) + 1;
        } else {
            yi -= 1;
            d += ((xi - yi) << 1) + 1;
        }
    }

    uint16_t swapped_color = RGB565_SWAP_GB(color);

    if (self->rotation == 0) {
        size_t fb_stride = RM690B0_PANEL_WIDTH;
        if (circle_fully_inside) {
            for (mp_int_t row = 0; row < row_count; row++) {
                int16_t span_left = left[row];
                int16_t span_right = right[row];
                if (span_left > span_right) {
                    continue;
                }
                mp_int_t yy = top + row;
                size_t span_width = (size_t)(span_right - span_left + 1);
                uint16_t *dest = impl->framebuffer + (size_t)yy * fb_stride + span_left;
                rm690b0_fill_span_fast(dest, span_width, swapped_color);
            }
        } else {
            for (mp_int_t row = 0; row < row_count; row++) {
                int16_t span_left = left[row];
                int16_t span_right = right[row];
                if (span_left > span_right) {
                    continue;
                }
                mp_int_t yy = top + row;
                if (yy < 0 || yy >= self->height) {
                    continue;
                }
                mp_int_t span_left_i = (mp_int_t)span_left;
                mp_int_t span_right_i = (mp_int_t)span_right;
                if (span_left_i < 0) {
                    span_left_i = 0;
                }
                if (span_right_i >= self->width) {
                    span_right_i = self->width - 1;
                }
                mp_int_t span_width = span_right_i - span_left_i + 1;
                if (span_width <= 0) {
                    continue;
                }
                uint16_t *dest = impl->framebuffer + (size_t)yy * fb_stride + span_left_i;
                rm690b0_fill_span_fast(dest, (size_t)span_width, swapped_color);
            }
        }
    } else {
        for (mp_int_t row = 0; row < row_count; row++) {
            int16_t span_left_val = left[row];
            int16_t span_right_val = right[row];
            if (span_left_val > span_right_val) {
                continue;
            }
            mp_int_t sx = (mp_int_t)span_left_val;
            mp_int_t sy = top + row;
            mp_int_t sw = (mp_int_t)(span_right_val - span_left_val + 1);
            mp_int_t sh = 1;
            if (!rm690b0_prepare_draw(self, &sx, &sy, &sw, &sh)) {
                continue;
            }
            rm690b0_fill_rect_framebuffer(impl, sx, sy, sw, sh, swapped_color);
        }
    }

    mp_int_t clip_bx = bx, clip_by = by, clip_bw = bw, clip_bh = bh;
    if (rm690b0_prepare_draw(self, &clip_bx, &clip_by, &clip_bw, &clip_bh)) {
        esp_err_t ret = rm690b0_finalize_draw(self, impl, clip_bx, clip_by, clip_bw, clip_bh);
        if (ret != ESP_OK) {
            if (heap_span != NULL) {
                heap_caps_free(heap_span);
            }
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to draw fill_circle: %s"), esp_err_to_name(ret));
        }
    }

    if (heap_span != NULL) {
        heap_caps_free(heap_span);
    }
}

// ============================================================================
// blit_buffer
// ============================================================================

void common_hal_rm690b0_rm690b0_blit_buffer(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_int_t width, mp_int_t height, mp_obj_t bitmap_data, bool dest_is_swapped, mp_int_t transparent_color, mp_int_t src_x1, mp_int_t src_y1, mp_int_t src_x2, mp_int_t src_y2) {
    CHECK_INITIALIZED();

    if (width <= 0 || height <= 0) {
        return;
    }

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(bitmap_data, &bufinfo, MP_BUFFER_READ);

    if (src_x2 < 0) {
        src_x2 = width;
    }
    if (src_y2 < 0) {
        src_y2 = height;
    }

    if (src_x1 < 0 || src_y1 < 0 || src_x2 > width || src_y2 > height || src_x1 >= src_x2 || src_y1 >= src_y2) {
        mp_raise_ValueError(MP_ERROR_TEXT("Invalid source region (must be: 0 <= x1 < x2 <= width, 0 <= y1 < y2 <= height)"));
        return;
    }

    mp_int_t src_region_w = src_x2 - src_x1;
    mp_int_t src_region_h = src_y2 - src_y1;

    size_t src_width = (size_t)width;
    size_t src_height = (size_t)height;

    size_t expected_bytes;
    if (!check_bitmap_size(src_width, src_height, &expected_bytes)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Bitmap dimensions too large (max ~32767x32767 on 32-bit systems)"));
        return;
    }

    if (bufinfo.len < expected_bytes) {
        mp_raise_ValueError(MP_ERROR_TEXT("Bitmap data too small for width * height"));
        return;
    }

    mp_int_t logical_x = x;
    mp_int_t logical_y = y;
    mp_int_t logical_w = src_region_w;
    mp_int_t logical_h = src_region_h;

    if (!clip_logical_rect(self, &logical_x, &logical_y, &logical_w, &logical_h)) {
        return;
    }

    mp_int_t crop_left = logical_x - x;
    mp_int_t crop_top = logical_y - y;
    if (crop_left < 0) {
        crop_left = 0;
    }
    if (crop_top < 0) {
        crop_top = 0;
    }

    mp_int_t phys_x = logical_x;
    mp_int_t phys_y = logical_y;
    mp_int_t phys_w = logical_w;
    mp_int_t phys_h = logical_h;

    if (!map_rect_for_rotation(self, &phys_x, &phys_y, &phys_w, &phys_h)) {
        return;
    }

    if (phys_w <= 0 || phys_h <= 0) {
        return;
    }

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || impl->framebuffer == NULL) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Invalid display handle"));
        return;
    }

    const uint16_t *src_base = (const uint16_t *)bufinfo.buf;
    size_t src_stride = src_width;
    const uint16_t *src_pixels = src_base + (size_t)(src_y1 + crop_top) * src_stride + (size_t)(src_x1 + crop_left);

    uint16_t *framebuffer = impl->framebuffer;
    size_t fb_stride = RM690B0_PANEL_WIDTH;

    bool has_transparency = (transparent_color >= 0 && transparent_color <= 0xFFFF);
    uint16_t transp = (uint16_t)transparent_color;

    switch (self->rotation) {
        case 0:
            for (mp_int_t row = 0; row < logical_h; row++) {
                const uint16_t *src_row = src_pixels + (size_t)row * src_stride;
                uint16_t *dst_row = framebuffer + (size_t)(phys_y + row) * fb_stride + phys_x;

                if (!has_transparency && dest_is_swapped) {
                    memcpy(dst_row, src_row, logical_w * sizeof(uint16_t));
                } else {
                    for (mp_int_t col = 0; col < logical_w; col++) {
                        uint16_t val = src_row[col];

                        if (has_transparency && val == transp) {
                            continue;
                        }

                        dst_row[col] = dest_is_swapped ? val : RGB565_SWAP_GB(val);
                    }
                }
            }
            break;
        case 180:
            for (mp_int_t row = 0; row < logical_h; row++) {
                const uint16_t *src_row = src_pixels + (size_t)(logical_h - 1 - row) * src_stride;
                uint16_t *dst_row = framebuffer + (size_t)(phys_y + row) * fb_stride + phys_x;
                for (mp_int_t col = 0; col < logical_w; col++) {
                    uint16_t val = src_row[logical_w - 1 - col];

                    if (has_transparency && val == transp) {
                        continue;
                    }

                    dst_row[col] = dest_is_swapped ? val : RGB565_SWAP_GB(val);
                }
            }
            break;
        case 90: {
            mp_int_t phys_h_rows = phys_h;
            mp_int_t phys_w_cols = phys_w;
            for (mp_int_t row = 0; row < phys_h_rows; row++) {
                uint16_t *dst_row = framebuffer + (size_t)(phys_y + row) * fb_stride + phys_x;
                for (mp_int_t col = 0; col < phys_w_cols; col++) {
                    mp_int_t src_row_idx = logical_h - 1 - col;
                    mp_int_t src_col_idx = row;
                    const uint16_t *src_row = src_pixels + (size_t)src_row_idx * src_stride;
                    uint16_t val = src_row[src_col_idx];

                    if (has_transparency && val == transp) {
                        continue;
                    }

                    dst_row[col] = dest_is_swapped ? val : RGB565_SWAP_GB(val);
                }
            }
            break;
        }
        case 270: {
            mp_int_t phys_h_rows = phys_h;
            mp_int_t phys_w_cols = phys_w;
            for (mp_int_t row = 0; row < phys_h_rows; row++) {
                uint16_t *dst_row = framebuffer + (size_t)(phys_y + row) * fb_stride + phys_x;
                for (mp_int_t col = 0; col < phys_w_cols; col++) {
                    mp_int_t src_row_idx = col;
                    mp_int_t src_col_idx = logical_w - 1 - row;
                    const uint16_t *src_row = src_pixels + (size_t)src_row_idx * src_stride;
                    uint16_t val = src_row[src_col_idx];

                    if (has_transparency && val == transp) {
                        continue;
                    }

                    dst_row[col] = dest_is_swapped ? val : RGB565_SWAP_GB(val);
                }
            }
            break;
        }
        default:
            mp_raise_ValueError(MP_ERROR_TEXT("Unsupported rotation"));
            return;
    }

    esp_err_t ret = rm690b0_finalize_draw(self, impl, phys_x, phys_y, phys_w, phys_h);
    if (ret != ESP_OK) {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to draw bitmap: %s"), esp_err_to_name(ret));
    }
}
