// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "py/obj.h"

// Buffer mode constants
#define RM690B0_BUFFER_SINGLE  0   // 1 framebuffer — dirty-tracked flush, saves 540 KB
#define RM690B0_BUFFER_DOUBLE  1   // Optional 2nd framebuffer allocated on first swap (default)

// Render backend mode constants
#define RM690B0_RENDER_FRAMEBUFFER   0
#define RM690B0_RENDER_DISPLAY_LIST  1

// Forward declaration for the rm690b0 object type
typedef struct {
    mp_obj_base_t base;
    bool initialized;
    mp_int_t width;
    mp_int_t height;
    mp_int_t rotation;
    uint8_t brightness_raw;
    mp_int_t font_id;  // Current font ID for text rendering
    mp_int_t buffer_mode;  // RM690B0_BUFFER_SINGLE or RM690B0_BUFFER_DOUBLE
    mp_int_t render_mode;  // RM690B0_RENDER_FRAMEBUFFER or RM690B0_RENDER_DISPLAY_LIST
    // Port-specific implementation data will be added by common-hal
    void *impl;
} rm690b0_rm690b0_obj_t;

typedef struct {
    size_t command_count;
    size_t payload_bytes;
    size_t max_command_count;
    size_t max_payload_bytes;
    size_t rejected_command_limit;
    size_t rejected_payload_limit;
    size_t allocation_failures;
    size_t present_count;
    size_t present_full;
    size_t present_partial;
    size_t compact_count;
    size_t compact_trimmed_commands;
    size_t auto_compact_trigger_periodic;
    size_t auto_compact_trigger_command_guard;
    size_t auto_compact_trigger_payload_guard;
    size_t glyph_atlas_hits;
    size_t glyph_atlas_misses;
    size_t glyph_atlas_builds;
    size_t glyph_atlas_evictions;
} rm690b0_display_list_stats_t;

extern const mp_obj_type_t rm690b0_rm690b0_type;

// Common HAL function declarations
void common_hal_rm690b0_rm690b0_construct(rm690b0_rm690b0_obj_t *self);
void common_hal_rm690b0_rm690b0_deinit(rm690b0_rm690b0_obj_t *self);
void common_hal_rm690b0_rm690b0_init_display(rm690b0_rm690b0_obj_t *self);
void common_hal_rm690b0_rm690b0_fill_color(rm690b0_rm690b0_obj_t *self, uint16_t color);
void common_hal_rm690b0_rm690b0_set_brightness(rm690b0_rm690b0_obj_t *self, mp_float_t brightness);
mp_float_t common_hal_rm690b0_rm690b0_get_brightness(const rm690b0_rm690b0_obj_t *self);
void common_hal_rm690b0_rm690b0_pixel(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_int_t color);
void common_hal_rm690b0_rm690b0_fill_rect(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_int_t width, mp_int_t height, mp_int_t color);
void common_hal_rm690b0_rm690b0_hline(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_int_t width, mp_int_t color);
void common_hal_rm690b0_rm690b0_vline(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_int_t height, mp_int_t color);
void common_hal_rm690b0_rm690b0_line(rm690b0_rm690b0_obj_t *self, mp_int_t x0, mp_int_t y0, mp_int_t x1, mp_int_t y1, mp_int_t color);
void common_hal_rm690b0_rm690b0_rect(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_int_t width, mp_int_t height, mp_int_t color);
void common_hal_rm690b0_rm690b0_circle(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_int_t radius, mp_int_t color);
void common_hal_rm690b0_rm690b0_fill_circle(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_int_t radius, mp_int_t color);
void common_hal_rm690b0_rm690b0_set_rotation(rm690b0_rm690b0_obj_t *self, mp_int_t degrees);
mp_int_t common_hal_rm690b0_rm690b0_get_rotation(const rm690b0_rm690b0_obj_t *self);
void common_hal_rm690b0_rm690b0_set_render_mode(rm690b0_rm690b0_obj_t *self, mp_int_t mode);
mp_int_t common_hal_rm690b0_rm690b0_get_render_mode(const rm690b0_rm690b0_obj_t *self);
void common_hal_rm690b0_rm690b0_blit_buffer(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_int_t width, mp_int_t height, mp_obj_t bitmap_data, bool dest_is_swapped, mp_int_t transparent_color, mp_int_t src_x1, mp_int_t src_y1, mp_int_t src_x2, mp_int_t src_y2);
void common_hal_rm690b0_rm690b0_blit_bmp(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_obj_t bmp_data);
void common_hal_rm690b0_rm690b0_blit_jpeg(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y, mp_obj_t jpeg_data);
void common_hal_rm690b0_rm690b0_convert_bmp(rm690b0_rm690b0_obj_t *self, mp_obj_t src_data, mp_obj_t dest_bitmap);
void common_hal_rm690b0_rm690b0_swap_buffers(rm690b0_rm690b0_obj_t *self, bool copy);
void common_hal_rm690b0_rm690b0_compact_display_list(rm690b0_rm690b0_obj_t *self);
void common_hal_rm690b0_rm690b0_get_display_list_stats(rm690b0_rm690b0_obj_t *self, rm690b0_display_list_stats_t *out);
void common_hal_rm690b0_rm690b0_reset_display_list_stats(rm690b0_rm690b0_obj_t *self);
void common_hal_rm690b0_rm690b0_deinit_all(void);
void common_hal_rm690b0_rm690b0_set_font(rm690b0_rm690b0_obj_t *self, mp_int_t font_id);
void common_hal_rm690b0_rm690b0_text(rm690b0_rm690b0_obj_t *self, mp_int_t x, mp_int_t y,
    const char *text, size_t text_len, uint16_t fg, bool has_bg, uint16_t bg);
mp_int_t common_hal_rm690b0_rm690b0_get_font_width(rm690b0_rm690b0_obj_t *self);
mp_int_t common_hal_rm690b0_rm690b0_get_font_height(rm690b0_rm690b0_obj_t *self);
mp_int_t common_hal_rm690b0_rm690b0_get_text_width(rm690b0_rm690b0_obj_t *self, const char *text, size_t len);
