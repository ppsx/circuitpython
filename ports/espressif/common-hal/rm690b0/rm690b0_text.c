// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

// Font rendering: glyph accessors, draw_glyph_impl, text(), set_font(),
// get_font_width/height/text_width.

#include "rm690b0_internal.h"
#include "fonts/rm690b0_font_8x8.h"
#include "fonts/rm690b0_font_16x16.h"
#include "fonts/rm690b0_font_16x24.h"
#include "fonts/rm690b0_font_24x24.h"
#include "fonts/rm690b0_font_24x32.h"
#include "fonts/rm690b0_font_32x32.h"
#include "fonts/rm690b0_font_32x48.h"

// Compile-time assertions to ensure fallback character '?' exists in all fonts
_Static_assert('?' >= 32 && '?' <= 127, "Fallback character '?' must be in font range");
_Static_assert(sizeof(rm690b0_font_8x8_data) / sizeof(rm690b0_font_8x8_data[0]) == 96,
    "Font 8x8 must have exactly 96 glyphs (0x20-0x7F)");
_Static_assert(sizeof(rm690b0_font_16x16_data) / sizeof(rm690b0_font_16x16_data[0]) == 95,
    "Font 16x16 must have exactly 95 glyphs (0x20-0x7E)");

// ============================================================================
// Glyph accessors
// ============================================================================

static inline const uint8_t *rm690b0_get_8x8_glyph(uint32_t codepoint) {
    if (codepoint < 32 || codepoint > 127) {
        codepoint = '?';
    }
    return rm690b0_font_8x8_data[codepoint - 32];
}

static inline const uint8_t *rm690b0_get_16x16_glyph(uint32_t codepoint) {
    if (codepoint < 32 || codepoint > 126) {
        codepoint = '?';
    }
    return rm690b0_font_16x16_data[codepoint - 32];
}

static inline const uint8_t *rm690b0_get_16x24_glyph(uint32_t codepoint) {
    if (codepoint < 32 || codepoint > 126) {
        codepoint = '?';
    }
    return rm690b0_font_16x24_data[codepoint - 32];
}

static inline const uint8_t *rm690b0_get_24x24_glyph(uint32_t codepoint) {
    if (codepoint < 32 || codepoint > 126) {
        codepoint = '?';
    }
    return rm690b0_font_24x24_data[codepoint - 32];
}

static inline const uint8_t *rm690b0_get_24x32_glyph(uint32_t codepoint) {
    if (codepoint < 32 || codepoint > 126) {
        codepoint = '?';
    }
    return rm690b0_font_24x32_data[codepoint - 32];
}

static inline const uint8_t *rm690b0_get_32x32_glyph(uint32_t codepoint) {
    if (codepoint < 32 || codepoint > 126) {
        codepoint = '?';
    }
    return rm690b0_font_32x32_data[codepoint - 32];
}

static inline const uint8_t *rm690b0_get_32x48_glyph(uint32_t codepoint) {
    if (codepoint < 32 || codepoint > 126) {
        codepoint = '?';
    }
    return rm690b0_font_32x48_data[codepoint - 32];
}

// ============================================================================
// Unified glyph rendering
// ============================================================================

static inline uint32_t rm690b0_read_glyph_row(
    const uint8_t *glyph, int row, int bytes_per_row) {
    uint32_t bits = 0;
    const uint8_t *p = glyph + row * bytes_per_row;
    for (int i = 0; i < bytes_per_row; i++) {
        bits = (bits << 8) | p[i];
    }
    return bits;
}

static void rm690b0_draw_glyph_impl(
    rm690b0_rm690b0_obj_t *self,
    mp_int_t x, mp_int_t y,
    const uint8_t *glyph,
    uint16_t fg, bool has_bg, uint16_t bg, bool auto_flush,
    int glyph_w, int glyph_h) {

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || impl->framebuffer == NULL) {
        return;
    }

    mp_int_t clip_x = x;
    mp_int_t clip_y = y;
    mp_int_t clip_w = glyph_w;
    mp_int_t clip_h = glyph_h;
    if (!clip_logical_rect(self, &clip_x, &clip_y, &clip_w, &clip_h)) {
        return;
    }

    mp_int_t col_start = clip_x - x;
    mp_int_t col_end = col_start + clip_w;
    mp_int_t row_start = clip_y - y;
    mp_int_t row_end = row_start + clip_h;

    uint16_t fg_swapped = RGB565_SWAP_GB(fg);
    uint16_t bg_swapped = has_bg ? RGB565_SWAP_GB(bg) : 0;

    int bytes_per_row   = glyph_w / 8;
    uint32_t top_bit    = 0x80000000u >> (32 - glyph_w);

    const mp_int_t rot  = self->rotation;
    size_t fb_stride    = RM690B0_PANEL_WIDTH;
    uint16_t *fb_ptr    = impl->framebuffer;

    if (rot == 0) {
        for (int row = row_start; row < row_end; row++) {
            uint32_t bits = rm690b0_read_glyph_row(glyph, row, bytes_per_row);
            size_t row_offset = (size_t)(y + row) * fb_stride + (x + col_start);
            for (int col = col_start; col < col_end; col++) {
                if (bits & (top_bit >> col)) {
                    fb_ptr[row_offset + (col - col_start)] = fg_swapped;
                } else if (has_bg) {
                    fb_ptr[row_offset + (col - col_start)] = bg_swapped;
                }
            }
        }
    } else if (rot == 90) {
        for (int row = row_start; row < row_end; row++) {
            uint32_t bits = rm690b0_read_glyph_row(glyph, row, bytes_per_row);
            mp_int_t phys_x = RM690B0_PANEL_WIDTH - (y + row) - 1;
            size_t start_index = (size_t)(x + col_start) * fb_stride + phys_x;
            for (int col = col_start; col < col_end; col++) {
                if (bits & (top_bit >> col)) {
                    fb_ptr[start_index] = fg_swapped;
                } else if (has_bg) {
                    fb_ptr[start_index] = bg_swapped;
                }
                start_index += fb_stride;
            }
        }
    } else if (rot == 180) {
        for (int row = row_start; row < row_end; row++) {
            uint32_t bits = rm690b0_read_glyph_row(glyph, row, bytes_per_row);
            mp_int_t phys_y = RM690B0_PANEL_HEIGHT - (y + row) - 1;
            mp_int_t phys_x_start = RM690B0_PANEL_WIDTH - (x + col_start) - 1;
            size_t start_index = (size_t)phys_y * fb_stride + phys_x_start;
            for (int col = col_start; col < col_end; col++) {
                if (bits & (top_bit >> col)) {
                    fb_ptr[start_index] = fg_swapped;
                } else if (has_bg) {
                    fb_ptr[start_index] = bg_swapped;
                }
                start_index--;
            }
        }
    } else if (rot == 270) {
        for (int row = row_start; row < row_end; row++) {
            uint32_t bits = rm690b0_read_glyph_row(glyph, row, bytes_per_row);
            mp_int_t phys_x = y + row;
            mp_int_t phys_y_start = RM690B0_PANEL_HEIGHT - (x + col_start) - 1;
            size_t start_index = (size_t)phys_y_start * fb_stride + phys_x;
            for (int col = col_start; col < col_end; col++) {
                if (bits & (top_bit >> col)) {
                    fb_ptr[start_index] = fg_swapped;
                } else if (has_bg) {
                    fb_ptr[start_index] = bg_swapped;
                }
                start_index -= fb_stride;
            }
        }
    }

    mp_int_t dirty_x = x, dirty_y = y, dirty_w = glyph_w, dirty_h = glyph_h;
    if (map_rect_for_rotation(self, &dirty_x, &dirty_y, &dirty_w, &dirty_h)) {
        mark_dirty_region(impl, dirty_x, dirty_y, dirty_w, dirty_h);
        if (auto_flush && !impl->double_buffered) {
            esp_err_t ret = rm690b0_flush_region(self, dirty_x, dirty_y,
                                                  dirty_w, dirty_h);
            if (ret != ESP_OK) {
                mp_raise_msg_varg(&mp_type_RuntimeError,
                    MP_ERROR_TEXT("Failed to draw text: %s"), esp_err_to_name(ret));
            }
        }
    }
}

// Thin wrappers — pass compile-time constants so GCC can fully optimize impl
static inline void rm690b0_draw_glyph_8x8(
    rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y,
    const uint8_t *glyph, uint16_t fg, bool has_bg, uint16_t bg, bool auto_flush) {
    rm690b0_draw_glyph_impl(self, x, y, glyph, fg, has_bg, bg, auto_flush, 8, 8);
}
static inline void rm690b0_draw_glyph_16x16(
    rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y,
    const uint8_t *glyph, uint16_t fg, bool has_bg, uint16_t bg, bool auto_flush) {
    rm690b0_draw_glyph_impl(self, x, y, glyph, fg, has_bg, bg, auto_flush, 16, 16);
}
static inline void rm690b0_draw_glyph_16x24(
    rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y,
    const uint8_t *glyph, uint16_t fg, bool has_bg, uint16_t bg, bool auto_flush) {
    rm690b0_draw_glyph_impl(self, x, y, glyph, fg, has_bg, bg, auto_flush, 16, 24);
}
static inline void rm690b0_draw_glyph_24x24(
    rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y,
    const uint8_t *glyph, uint16_t fg, bool has_bg, uint16_t bg, bool auto_flush) {
    rm690b0_draw_glyph_impl(self, x, y, glyph, fg, has_bg, bg, auto_flush, 24, 24);
}
static inline void rm690b0_draw_glyph_24x32(
    rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y,
    const uint8_t *glyph, uint16_t fg, bool has_bg, uint16_t bg, bool auto_flush) {
    rm690b0_draw_glyph_impl(self, x, y, glyph, fg, has_bg, bg, auto_flush, 24, 32);
}
static inline void rm690b0_draw_glyph_32x32(
    rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y,
    const uint8_t *glyph, uint16_t fg, bool has_bg, uint16_t bg, bool auto_flush) {
    rm690b0_draw_glyph_impl(self, x, y, glyph, fg, has_bg, bg, auto_flush, 32, 32);
}
static inline void rm690b0_draw_glyph_32x48(
    rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y,
    const uint8_t *glyph, uint16_t fg, bool has_bg, uint16_t bg, bool auto_flush) {
    rm690b0_draw_glyph_impl(self, x, y, glyph, fg, has_bg, bg, auto_flush, 32, 48);
}

// ============================================================================
// Public API
// ============================================================================

void common_hal_rm690b0_rm690b0_set_font(rm690b0_rm690b0_obj_t *self, mp_int_t font_id) {
    if (font_id < RM690B0_FONT_8x8_MONO || font_id > RM690B0_FONT_32x48_MONO) {
        font_id = RM690B0_FONT_8x8_MONO;
    }
    self->font_id = font_id;
}

static inline mp_int_t rm690b0_get_current_font(const rm690b0_rm690b0_obj_t *self) {
    return self->font_id;
}

static inline void rm690b0_get_font_dims(mp_int_t font_id, mp_int_t *width, mp_int_t *height) {
    switch (font_id) {
        case RM690B0_FONT_16x16_MONO:
            *width = 16;
            *height = 16;
            break;
        case RM690B0_FONT_16x24_MONO:
            *width = 16;
            *height = 24;
            break;
        case RM690B0_FONT_24x24_MONO:
            *width = 24;
            *height = 24;
            break;
        case RM690B0_FONT_24x32_MONO:
            *width = 24;
            *height = 32;
            break;
        case RM690B0_FONT_32x32_MONO:
            *width = 32;
            *height = 32;
            break;
        case RM690B0_FONT_32x48_MONO:
            *width = 32;
            *height = 48;
            break;
        default: // RM690B0_FONT_8x8_MONO
            *width = 8;
            *height = 8;
            break;
    }
}

void common_hal_rm690b0_rm690b0_text(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y,
    const char *text, size_t text_len, uint16_t fg, bool has_bg, uint16_t bg) {

    CHECK_INITIALIZED();

    if (text_len == 0) {
        return;
    }

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    mp_int_t font_id = rm690b0_get_current_font(self);
    mp_int_t cursor_x = x;
    mp_int_t cursor_y = y;

    mp_int_t font_width, font_height;
    rm690b0_get_font_dims(font_id, &font_width, &font_height);

    mp_int_t min_x = x;
    mp_int_t max_x = x;
    mp_int_t min_y = y;
    mp_int_t max_y = rm690b0_add_mp_int_saturating(y, font_height);

    for (size_t i = 0; i < text_len; i++) {
        uint8_t ch = (uint8_t)text[i];

        if (cursor_x < min_x) {
            min_x = cursor_x;
        }

        if (ch == '\n') {
            cursor_x = x;
            cursor_y = rm690b0_add_mp_int_saturating(cursor_y, font_height);
            mp_int_t line_bottom = rm690b0_add_mp_int_saturating(cursor_y, font_height);
            if (line_bottom > max_y) {
                max_y = line_bottom;
            }
            continue;
        } else if (ch == '\r') {
            continue;
        }

        switch (font_id) {
            case RM690B0_FONT_16x16_MONO: {
                const uint8_t *glyph = rm690b0_get_16x16_glyph(ch);
                rm690b0_draw_glyph_16x16(self, cursor_x, cursor_y, glyph, fg, has_bg, bg, false);
                break;
            }
            case RM690B0_FONT_16x24_MONO: {
                const uint8_t *glyph = rm690b0_get_16x24_glyph(ch);
                rm690b0_draw_glyph_16x24(self, cursor_x, cursor_y, glyph, fg, has_bg, bg, false);
                break;
            }
            case RM690B0_FONT_24x24_MONO: {
                const uint8_t *glyph = rm690b0_get_24x24_glyph(ch);
                rm690b0_draw_glyph_24x24(self, cursor_x, cursor_y, glyph, fg, has_bg, bg, false);
                break;
            }
            case RM690B0_FONT_24x32_MONO: {
                const uint8_t *glyph = rm690b0_get_24x32_glyph(ch);
                rm690b0_draw_glyph_24x32(self, cursor_x, cursor_y, glyph, fg, has_bg, bg, false);
                break;
            }
            case RM690B0_FONT_32x32_MONO: {
                const uint8_t *glyph = rm690b0_get_32x32_glyph(ch);
                rm690b0_draw_glyph_32x32(self, cursor_x, cursor_y, glyph, fg, has_bg, bg, false);
                break;
            }
            case RM690B0_FONT_32x48_MONO: {
                const uint8_t *glyph = rm690b0_get_32x48_glyph(ch);
                rm690b0_draw_glyph_32x48(self, cursor_x, cursor_y, glyph, fg, has_bg, bg, false);
                break;
            }
            default: { // RM690B0_FONT_8x8_MONO
                const uint8_t *glyph = rm690b0_get_8x8_glyph(ch);
                rm690b0_draw_glyph_8x8(self, cursor_x, cursor_y, glyph, fg, has_bg, bg, false);
                break;
            }
        }
        cursor_x = rm690b0_add_mp_int_saturating(cursor_x, font_width);

        if (cursor_x > max_x) {
            max_x = cursor_x;
        }

        if (cursor_x >= self->width) {
            cursor_x = x;
            cursor_y = rm690b0_add_mp_int_saturating(cursor_y, font_height);
            mp_int_t line_bottom = rm690b0_add_mp_int_saturating(cursor_y, font_height);
            if (line_bottom > max_y) {
                max_y = line_bottom;
            }
        }
        if (cursor_y >= self->height) {
            break;
        }
    }

    if (!impl->double_buffered) {
        mp_int_t dirty_x = min_x;
        mp_int_t dirty_y = min_y;
        mp_int_t dirty_w = max_x - min_x;
        mp_int_t dirty_h = max_y - min_y;

        if (dirty_w > 0 && dirty_h > 0) {
            if (map_rect_for_rotation(self, &dirty_x, &dirty_y, &dirty_w, &dirty_h)) {
                esp_err_t ret = rm690b0_flush_region(self, dirty_x, dirty_y, dirty_w, dirty_h);
                if (ret != ESP_OK) {
                    mp_raise_msg_varg(&mp_type_RuntimeError,
                        MP_ERROR_TEXT("Failed to draw text: %s"), esp_err_to_name(ret));
                }
            }
        }
        impl->dirty_count = 0;
        impl->dirty_merged_valid = false;
    }
}

mp_int_t common_hal_rm690b0_rm690b0_get_font_width(rm690b0_rm690b0_obj_t *self) {
    mp_int_t w, h;
    rm690b0_get_font_dims(self->font_id, &w, &h);
    return w;
}

mp_int_t common_hal_rm690b0_rm690b0_get_font_height(rm690b0_rm690b0_obj_t *self) {
    mp_int_t w, h;
    rm690b0_get_font_dims(self->font_id, &w, &h);
    return h;
}

mp_int_t common_hal_rm690b0_rm690b0_get_text_width(rm690b0_rm690b0_obj_t *self, const char *text, size_t len) {
    mp_int_t w, h;
    rm690b0_get_font_dims(self->font_id, &w, &h);

    size_t max_visible = 0;
    size_t current_visible = 0;
    for (size_t i = 0; i < len; i++) {
        if (text[i] == '\n') {
            if (current_visible > max_visible) {
                max_visible = current_visible;
            }
            current_visible = 0;
        } else if (text[i] != '\r') {
            current_visible++;
        }
    }
    if (current_visible > max_visible) {
        max_visible = current_visible;
    }
    return (mp_int_t)max_visible * w;
}
