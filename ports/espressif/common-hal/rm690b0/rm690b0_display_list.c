// SPDX-FileCopyrightText: Copyright (c) 2026 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

// Display-list backend for framebuffer-free rendering.

#include "rm690b0_internal.h"
#include "py/misc.h"
#include "fonts/rm690b0_font_8x8.h"
#include "fonts/rm690b0_font_16x16.h"
#include "fonts/rm690b0_font_16x24.h"
#include "fonts/rm690b0_font_24x24.h"
#include "fonts/rm690b0_font_24x32.h"
#include "fonts/rm690b0_font_32x32.h"
#include "fonts/rm690b0_font_32x48.h"

#define RM690B0_DL_GLYPH_ATLAS_HINT_INVALID (0xFFu)

static inline void rm690b0_dl_plot_pixel(
    uint16_t *chunk_buffer,
    size_t chunk_width,
    size_t chunk_rows,
    mp_int_t chunk_start_x,
    mp_int_t chunk_start_y,
    mp_int_t x,
    mp_int_t y,
    uint16_t color_swapped) {
    if (x < chunk_start_x || y < chunk_start_y) {
        return;
    }
    mp_int_t local_x = x - chunk_start_x;
    mp_int_t local_y = y - chunk_start_y;
    if (local_x < 0 || local_y < 0) {
        return;
    }
    size_t local_col = (size_t)local_x;
    size_t local_row = (size_t)local_y;
    if (local_col >= chunk_width || local_row >= chunk_rows) {
        return;
    }
    chunk_buffer[local_row * chunk_width + local_col] = color_swapped;
}

static inline void rm690b0_dl_fill_span_clipped(
    uint16_t *chunk_buffer,
    size_t chunk_width,
    size_t chunk_rows,
    mp_int_t chunk_start_x,
    mp_int_t chunk_start_y,
    mp_int_t y,
    mp_int_t x0,
    mp_int_t x1,
    uint16_t color_swapped) {
    if (x1 < x0) {
        return;
    }
    if (y < chunk_start_y || y >= chunk_start_y + (mp_int_t)chunk_rows) {
        return;
    }
    mp_int_t chunk_end_x = chunk_start_x + (mp_int_t)chunk_width - 1;
    if (x0 < chunk_start_x) {
        x0 = chunk_start_x;
    }
    if (x1 > chunk_end_x) {
        x1 = chunk_end_x;
    }
    if (x1 < x0) {
        return;
    }

    size_t local_row = (size_t)(y - chunk_start_y);
    size_t local_x = (size_t)(x0 - chunk_start_x);
    uint16_t *dst = chunk_buffer + local_row * chunk_width + local_x;
    rm690b0_fill_span_fast(dst, (size_t)(x1 - x0 + 1), color_swapped);
}

static inline void rm690b0_dl_copy_transparent_runs(
    uint16_t *dst,
    const uint16_t *src,
    size_t pixel_count,
    uint16_t transparency_swapped) {
    size_t i = 0;
    while (i < pixel_count) {
        while (i < pixel_count && src[i] == transparency_swapped) {
            i++;
        }
        size_t run_start = i;
        while (i < pixel_count && src[i] != transparency_swapped) {
            i++;
        }
        size_t run_len = i - run_start;
        if (run_len > 0) {
            memcpy(dst + run_start, src + run_start, run_len * sizeof(uint16_t));
        }
    }
}

static inline uint32_t rm690b0_dl_read_glyph_row(const uint8_t *glyph, int row, int bytes_per_row) {
    uint32_t bits = 0;
    const uint8_t *p = glyph + row * bytes_per_row;
    for (int i = 0; i < bytes_per_row; i++) {
        bits = (bits << 8) | p[i];
    }
    return bits;
}

static inline uint16_t rm690b0_dl_normalize_rotation(mp_int_t rotation) {
    switch (rotation) {
        case 90:
            return 90;
        case 180:
            return 180;
        case 270:
            return 270;
        case 0:
        default:
            return 0;
    }
}

static inline uint8_t rm690b0_dl_normalize_glyph_char(uint8_t font_id, uint8_t ch) {
    if (font_id == RM690B0_FONT_8x8_MONO) {
        if (ch < 32 || ch > 127) {
            return '?';
        }
        return ch;
    }

    if (ch < 32 || ch > 126) {
        return '?';
    }
    return ch;
}

static inline bool rm690b0_dl_get_font_dims(uint8_t font_id, mp_int_t *width, mp_int_t *height) {
    switch (font_id) {
        case RM690B0_FONT_16x16_MONO:
            *width = 16;
            *height = 16;
            return true;
        case RM690B0_FONT_16x24_MONO:
            *width = 16;
            *height = 24;
            return true;
        case RM690B0_FONT_24x24_MONO:
            *width = 24;
            *height = 24;
            return true;
        case RM690B0_FONT_24x32_MONO:
            *width = 24;
            *height = 32;
            return true;
        case RM690B0_FONT_32x32_MONO:
            *width = 32;
            *height = 32;
            return true;
        case RM690B0_FONT_32x48_MONO:
            *width = 32;
            *height = 48;
            return true;
        case RM690B0_FONT_8x8_MONO:
        default:
            *width = 8;
            *height = 8;
            return true;
    }
}

static inline const uint8_t *rm690b0_dl_get_glyph_ptr_normalized(uint8_t font_id, uint8_t normalized_ch) {
    switch (font_id) {
        case RM690B0_FONT_16x16_MONO:
            return rm690b0_font_16x16_data[normalized_ch - 32];
        case RM690B0_FONT_16x24_MONO:
            return rm690b0_font_16x24_data[normalized_ch - 32];
        case RM690B0_FONT_24x24_MONO:
            return rm690b0_font_24x24_data[normalized_ch - 32];
        case RM690B0_FONT_24x32_MONO:
            return rm690b0_font_24x32_data[normalized_ch - 32];
        case RM690B0_FONT_32x32_MONO:
            return rm690b0_font_32x32_data[normalized_ch - 32];
        case RM690B0_FONT_32x48_MONO:
            return rm690b0_font_32x48_data[normalized_ch - 32];
        case RM690B0_FONT_8x8_MONO:
        default:
            return rm690b0_font_8x8_data[normalized_ch - 32];
    }
}

static inline uint64_t rm690b0_dl_mask_range(uint8_t start, uint8_t end_exclusive) {
    if (start >= end_exclusive || start >= 64) {
        return 0;
    }
    if (end_exclusive >= 64) {
        end_exclusive = 64;
    }
    uint64_t high = (end_exclusive == 64) ? UINT64_MAX : ((1ULL << end_exclusive) - 1ULL);
    uint64_t low = (start == 0) ? 0 : ((1ULL << start) - 1ULL);
    return high & ~low;
}

static inline void rm690b0_dl_map_glyph_local_rotation(
    uint16_t rotation,
    mp_int_t glyph_w,
    mp_int_t glyph_h,
    mp_int_t col,
    mp_int_t row,
    mp_int_t *out_x,
    mp_int_t *out_y) {
    switch (rotation) {
        case 90:
            *out_x = glyph_h - 1 - row;
            *out_y = col;
            break;
        case 180:
            *out_x = glyph_w - 1 - col;
            *out_y = glyph_h - 1 - row;
            break;
        case 270:
            *out_x = row;
            *out_y = glyph_w - 1 - col;
            break;
        case 0:
        default:
            *out_x = col;
            *out_y = row;
            break;
    }
}

static void rm690b0_dl_build_glyph_cache_entry(rm690b0_dl_glyph_cache_entry_t *entry) {
    if (entry == NULL) {
        return;
    }

    memset(entry->row_masks, 0, sizeof(entry->row_masks));

    mp_int_t glyph_w = 0;
    mp_int_t glyph_h = 0;
    rm690b0_dl_get_font_dims(entry->font_id, &glyph_w, &glyph_h);
    if (glyph_w <= 0 || glyph_h <= 0) {
        entry->width = 0;
        entry->height = 0;
        return;
    }

    uint16_t rotation = rm690b0_dl_normalize_rotation(entry->rotation);
    entry->rotation = rotation;
    entry->width = (uint8_t)((rotation == 90 || rotation == 270) ? glyph_h : glyph_w);
    entry->height = (uint8_t)((rotation == 90 || rotation == 270) ? glyph_w : glyph_h);

    const uint8_t *glyph = rm690b0_dl_get_glyph_ptr_normalized(entry->font_id, entry->ch);
    int bytes_per_row = (int)(glyph_w / 8);
    uint32_t top_bit = 0x80000000u >> (32 - glyph_w);

    for (mp_int_t row = 0; row < glyph_h; row++) {
        uint32_t bits = rm690b0_dl_read_glyph_row(glyph, (int)row, bytes_per_row);
        for (mp_int_t col = 0; col < glyph_w; col++) {
            bool pixel_on = (bits & (top_bit >> col)) != 0;
            if (!pixel_on) {
                continue;
            }
            mp_int_t dst_x = 0;
            mp_int_t dst_y = 0;
            rm690b0_dl_map_glyph_local_rotation(rotation, glyph_w, glyph_h, col, row, &dst_x, &dst_y);
            if (dst_x < 0 || dst_y < 0 ||
                dst_x >= RM690B0_DL_GLYPH_MASK_MAX_COLS ||
                dst_y >= RM690B0_DL_GLYPH_MASK_MAX_ROWS) {
                continue;
            }
            entry->row_masks[dst_y] |= 1ULL << (uint8_t)dst_x;
        }
    }
}

static rm690b0_dl_glyph_cache_entry_t *rm690b0_dl_get_glyph_cache_entry(
    rm690b0_dl_state_t *dl,
    uint8_t font_id,
    uint8_t ch,
    mp_int_t rotation,
    uint8_t *atlas_hint) {
    if (dl == NULL) {
        return NULL;
    }

    uint8_t normalized_ch = rm690b0_dl_normalize_glyph_char(font_id, ch);
    uint16_t normalized_rotation = rm690b0_dl_normalize_rotation(rotation);
    if (atlas_hint != NULL &&
        *atlas_hint != RM690B0_DL_GLYPH_ATLAS_HINT_INVALID &&
        *atlas_hint < RM690B0_DL_GLYPH_ATLAS_SLOTS) {
        rm690b0_dl_glyph_cache_entry_t *hint_entry = &dl->glyph_atlas[*atlas_hint];
        if (hint_entry->valid &&
            hint_entry->font_id == font_id &&
            hint_entry->ch == normalized_ch &&
            hint_entry->rotation == normalized_rotation) {
            dl->telemetry_glyph_atlas_hits++;
            hint_entry->last_used = ++dl->glyph_atlas_tick;
            return hint_entry;
        }
    }

    size_t first_free = SIZE_MAX;
    size_t lru_idx = 0;
    uint32_t lru_tick = UINT32_MAX;
    for (size_t i = 0; i < RM690B0_DL_GLYPH_ATLAS_SLOTS; i++) {
        rm690b0_dl_glyph_cache_entry_t *entry = &dl->glyph_atlas[i];
        if (!entry->valid) {
            if (first_free == SIZE_MAX) {
                first_free = i;
            }
            continue;
        }
        if (entry->font_id == font_id &&
            entry->ch == normalized_ch &&
            entry->rotation == normalized_rotation) {
            dl->telemetry_glyph_atlas_hits++;
            entry->last_used = ++dl->glyph_atlas_tick;
            if (atlas_hint != NULL) {
                *atlas_hint = (uint8_t)i;
            }
            return entry;
        }
        if (entry->last_used < lru_tick) {
            lru_tick = entry->last_used;
            lru_idx = i;
        }
    }

    dl->telemetry_glyph_atlas_misses++;
    size_t use_idx = (first_free != SIZE_MAX) ? first_free : lru_idx;
    rm690b0_dl_glyph_cache_entry_t *entry = &dl->glyph_atlas[use_idx];
    if (first_free == SIZE_MAX && entry->valid) {
        dl->telemetry_glyph_atlas_evictions++;
    }
    entry->valid = true;
    entry->font_id = font_id;
    entry->ch = normalized_ch;
    entry->rotation = normalized_rotation;
    entry->last_used = ++dl->glyph_atlas_tick;
    dl->telemetry_glyph_atlas_builds++;
    rm690b0_dl_build_glyph_cache_entry(entry);
    if (atlas_hint != NULL) {
        *atlas_hint = (uint8_t)use_idx;
    }
    return entry;
}

static inline void rm690b0_dl_map_rect_rotation(
    mp_int_t rotation,
    mp_int_t x, mp_int_t y, mp_int_t w, mp_int_t h,
    mp_int_t *out_x, mp_int_t *out_y, mp_int_t *out_w, mp_int_t *out_h) {
    switch (rotation) {
        case 90:
            *out_x = RM690B0_PANEL_WIDTH - (y + h);
            *out_y = x;
            *out_w = h;
            *out_h = w;
            break;
        case 180:
            *out_x = RM690B0_PANEL_WIDTH - (x + w);
            *out_y = RM690B0_PANEL_HEIGHT - (y + h);
            *out_w = w;
            *out_h = h;
            break;
        case 270:
            *out_x = y;
            *out_y = RM690B0_PANEL_HEIGHT - (x + w);
            *out_w = h;
            *out_h = w;
            break;
        case 0:
        default:
            *out_x = x;
            *out_y = y;
            *out_w = w;
            *out_h = h;
            break;
    }
}

static inline uint16_t rm690b0_dl_to_swapped(uint16_t pixel, bool src_is_swapped) {
    return src_is_swapped ? pixel : RGB565_SWAP_GB(pixel);
}

static void rm690b0_dl_mark_dirty(rm690b0_dl_state_t *dl, mp_int_t x, mp_int_t y, mp_int_t w, mp_int_t h) {
    if (w <= 0 || h <= 0) {
        return;
    }

    if (!dl->dirty_valid) {
        dl->dirty_x = x;
        dl->dirty_y = y;
        dl->dirty_w = w;
        dl->dirty_h = h;
        dl->dirty_valid = true;
        return;
    }

    mp_int_t x2 = dl->dirty_x + dl->dirty_w;
    mp_int_t y2 = dl->dirty_y + dl->dirty_h;
    if (x < dl->dirty_x) {
        dl->dirty_x = x;
    }
    if (y < dl->dirty_y) {
        dl->dirty_y = y;
    }
    if (x + w > x2) {
        x2 = x + w;
    }
    if (y + h > y2) {
        y2 = y + h;
    }
    dl->dirty_w = x2 - dl->dirty_x;
    dl->dirty_h = y2 - dl->dirty_y;
}

static void rm690b0_dl_release_payloads(rm690b0_dl_state_t *dl) {
    if (dl == NULL || dl->items == NULL || dl->count == 0) {
        return;
    }
    for (size_t i = 0; i < dl->count; i++) {
        rm690b0_dl_cmd_t *cmd = &dl->items[i];
        if (cmd->type == RM690B0_DL_CMD_BLIT && cmd->u.blit.pixels_swapped != NULL) {
            if (dl->payload_bytes >= cmd->u.blit.payload_bytes) {
                dl->payload_bytes -= cmd->u.blit.payload_bytes;
            } else {
                dl->payload_bytes = 0;
            }
            heap_caps_free(cmd->u.blit.pixels_swapped);
            cmd->u.blit.pixels_swapped = NULL;
            cmd->u.blit.payload_bytes = 0;
        }
    }
    dl->payload_bytes = 0;
}

static inline void rm690b0_dl_note_command_count(rm690b0_dl_state_t *dl) {
    if (dl->count > dl->telemetry_max_command_count) {
        dl->telemetry_max_command_count = dl->count;
    }
}

static inline void rm690b0_dl_note_payload_bytes(rm690b0_dl_state_t *dl) {
    if (dl->payload_bytes > dl->telemetry_max_payload_bytes) {
        dl->telemetry_max_payload_bytes = dl->payload_bytes;
    }
}

static bool rm690b0_dl_reserve(rm690b0_dl_state_t *dl, size_t needed) {
    if (needed > RM690B0_DL_MAX_COMMANDS) {
        dl->telemetry_rejected_command_limit++;
        return false;
    }
    if (needed <= dl->capacity) {
        return true;
    }

    size_t new_capacity = dl->capacity == 0 ? 16 : dl->capacity * 2;
    while (new_capacity < needed) {
        new_capacity *= 2;
    }

    rm690b0_dl_cmd_t *new_items = m_renew_maybe(
        rm690b0_dl_cmd_t,
        dl->items,
        dl->capacity,
        new_capacity,
        true);
    if (new_items == NULL) {
        dl->telemetry_allocation_failures++;
        return false;
    }
    dl->items = new_items;
    dl->capacity = new_capacity;
    return true;
}

void rm690b0_dl_init_state(rm690b0_impl_t *impl) {
    if (impl == NULL) {
        return;
    }
    memset(&impl->dl, 0, sizeof(impl->dl));
}

void rm690b0_dl_deinit_state(rm690b0_impl_t *impl) {
    if (impl == NULL) {
        return;
    }

    rm690b0_dl_release_payloads(&impl->dl);
    if (impl->dl.items != NULL) {
        m_free(impl->dl.items);
        impl->dl.items = NULL;
    }
    memset(&impl->dl, 0, sizeof(impl->dl));
}

void rm690b0_dl_reset_frame(rm690b0_impl_t *impl, bool keep_capacity) {
    if (impl == NULL) {
        return;
    }

    rm690b0_dl_release_payloads(&impl->dl);

    if (!keep_capacity && impl->dl.items != NULL) {
        m_free(impl->dl.items);
        impl->dl.items = NULL;
        impl->dl.capacity = 0;
    }

    impl->dl.count = 0;
    impl->dl.dirty_valid = false;
    impl->dl.dirty_x = 0;
    impl->dl.dirty_y = 0;
    impl->dl.dirty_w = 0;
    impl->dl.dirty_h = 0;
    impl->dl.payload_bytes = 0;
}

esp_err_t rm690b0_dl_set_mode(rm690b0_rm690b0_obj_t *self, mp_int_t mode) {
    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    if (mode != RM690B0_RENDER_FRAMEBUFFER && mode != RM690B0_RENDER_DISPLAY_LIST) {
        return ESP_ERR_INVALID_ARG;
    }

    if (self->render_mode == mode) {
        return ESP_OK;
    }

    if (self->initialized) {
        rm690b0_wait_for_all_dma(impl);
    }

    if (self->render_mode == RM690B0_RENDER_DISPLAY_LIST) {
        rm690b0_dl_reset_frame(impl, true);
        impl->dl.clear_color_valid = false;
        impl->dl.force_full_present = false;
    }

    self->render_mode = mode;
    if (mode == RM690B0_RENDER_DISPLAY_LIST) {
        impl->dl.force_full_present = true;
        impl->dl.clear_color_valid = false;
        impl->dirty_count = 0;
        impl->dirty_merged_valid = false;
    }

    return ESP_OK;
}

esp_err_t rm690b0_dl_enqueue_fill_color(rm690b0_rm690b0_obj_t *self, uint16_t color_swapped) {
    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    if (self->render_mode != RM690B0_RENDER_DISPLAY_LIST) {
        return ESP_ERR_INVALID_STATE;
    }

    rm690b0_dl_state_t *dl = &impl->dl;
    if (!rm690b0_dl_reserve(dl, 1)) {
        return ESP_ERR_NO_MEM;
    }

    rm690b0_dl_release_payloads(dl);
    dl->items[0] = (rm690b0_dl_cmd_t) {
        .type = RM690B0_DL_CMD_FILL_COLOR,
        .color_swapped = color_swapped,
        .u.rect = {
            .x = 0,
            .y = 0,
            .w = RM690B0_PANEL_WIDTH,
            .h = RM690B0_PANEL_HEIGHT,
        },
    };
    dl->count = 1;
    rm690b0_dl_note_command_count(dl);
    dl->clear_color_valid = true;
    dl->clear_color_swapped = color_swapped;
    dl->force_full_present = true;
    rm690b0_dl_mark_dirty(dl, 0, 0, RM690B0_PANEL_WIDTH, RM690B0_PANEL_HEIGHT);
    return ESP_OK;
}

esp_err_t rm690b0_dl_enqueue_fill_rect(rm690b0_rm690b0_obj_t *self,
    mp_int_t x, mp_int_t y, mp_int_t w, mp_int_t h, uint16_t color_swapped) {

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    if (self->render_mode != RM690B0_RENDER_DISPLAY_LIST) {
        return ESP_ERR_INVALID_STATE;
    }
    if (w <= 0 || h <= 0) {
        return ESP_OK;
    }

    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (x + w > RM690B0_PANEL_WIDTH) {
        w = RM690B0_PANEL_WIDTH - x;
    }
    if (y + h > RM690B0_PANEL_HEIGHT) {
        h = RM690B0_PANEL_HEIGHT - y;
    }
    if (w <= 0 || h <= 0) {
        return ESP_OK;
    }

    if (x == 0 && y == 0 && w == RM690B0_PANEL_WIDTH && h == RM690B0_PANEL_HEIGHT) {
        return rm690b0_dl_enqueue_fill_color(self, color_swapped);
    }

    rm690b0_dl_state_t *dl = &impl->dl;
    if (!rm690b0_dl_reserve(dl, dl->count + 1)) {
        return ESP_ERR_NO_MEM;
    }

    dl->items[dl->count++] = (rm690b0_dl_cmd_t) {
        .type = RM690B0_DL_CMD_FILL_RECT,
        .color_swapped = color_swapped,
        .u.rect = {
            .x = x,
            .y = y,
            .w = w,
            .h = h,
        },
    };
    rm690b0_dl_note_command_count(dl);
    rm690b0_dl_mark_dirty(dl, x, y, w, h);
    return ESP_OK;
}

esp_err_t rm690b0_dl_enqueue_pixel(rm690b0_rm690b0_obj_t *self,
    mp_int_t x, mp_int_t y, uint16_t color_swapped) {
    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || self->render_mode != RM690B0_RENDER_DISPLAY_LIST) {
        return ESP_ERR_INVALID_STATE;
    }
    if (x < 0 || x >= RM690B0_PANEL_WIDTH || y < 0 || y >= RM690B0_PANEL_HEIGHT) {
        return ESP_OK;
    }

    rm690b0_dl_state_t *dl = &impl->dl;
    if (!rm690b0_dl_reserve(dl, dl->count + 1)) {
        return ESP_ERR_NO_MEM;
    }

    dl->items[dl->count++] = (rm690b0_dl_cmd_t) {
        .type = RM690B0_DL_CMD_PIXEL,
        .color_swapped = color_swapped,
        .u.pixel = {
            .x = x,
            .y = y,
        },
    };
    rm690b0_dl_note_command_count(dl);
    rm690b0_dl_mark_dirty(dl, x, y, 1, 1);
    return ESP_OK;
}

esp_err_t rm690b0_dl_enqueue_line(rm690b0_rm690b0_obj_t *self,
    mp_int_t x0, mp_int_t y0, mp_int_t x1, mp_int_t y1, uint16_t color_swapped) {
    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || self->render_mode != RM690B0_RENDER_DISPLAY_LIST) {
        return ESP_ERR_INVALID_STATE;
    }

    rm690b0_dl_state_t *dl = &impl->dl;
    if (!rm690b0_dl_reserve(dl, dl->count + 1)) {
        return ESP_ERR_NO_MEM;
    }

    dl->items[dl->count++] = (rm690b0_dl_cmd_t) {
        .type = RM690B0_DL_CMD_LINE,
        .color_swapped = color_swapped,
        .u.line = {
            .x0 = x0,
            .y0 = y0,
            .x1 = x1,
            .y1 = y1,
        },
    };
    rm690b0_dl_note_command_count(dl);

    mp_int_t min_x = x0 < x1 ? x0 : x1;
    mp_int_t max_x = x0 > x1 ? x0 : x1;
    mp_int_t min_y = y0 < y1 ? y0 : y1;
    mp_int_t max_y = y0 > y1 ? y0 : y1;
    rm690b0_dl_mark_dirty(dl, min_x, min_y, max_x - min_x + 1, max_y - min_y + 1);
    return ESP_OK;
}

esp_err_t rm690b0_dl_enqueue_circle(rm690b0_rm690b0_obj_t *self,
    mp_int_t x, mp_int_t y, mp_int_t radius, uint16_t color_swapped) {
    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || self->render_mode != RM690B0_RENDER_DISPLAY_LIST) {
        return ESP_ERR_INVALID_STATE;
    }

    rm690b0_dl_state_t *dl = &impl->dl;
    if (!rm690b0_dl_reserve(dl, dl->count + 1)) {
        return ESP_ERR_NO_MEM;
    }

    dl->items[dl->count++] = (rm690b0_dl_cmd_t) {
        .type = RM690B0_DL_CMD_CIRCLE,
        .color_swapped = color_swapped,
        .u.circle = {
            .x = x,
            .y = y,
            .radius = radius,
        },
    };
    rm690b0_dl_note_command_count(dl);

    mp_int_t diameter = radius * 2 + 1;
    rm690b0_dl_mark_dirty(dl, x - radius, y - radius, diameter, diameter);
    return ESP_OK;
}

esp_err_t rm690b0_dl_enqueue_fill_circle(rm690b0_rm690b0_obj_t *self,
    mp_int_t x, mp_int_t y, mp_int_t radius, uint16_t color_swapped) {
    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || self->render_mode != RM690B0_RENDER_DISPLAY_LIST) {
        return ESP_ERR_INVALID_STATE;
    }

    rm690b0_dl_state_t *dl = &impl->dl;
    if (!rm690b0_dl_reserve(dl, dl->count + 1)) {
        return ESP_ERR_NO_MEM;
    }

    dl->items[dl->count++] = (rm690b0_dl_cmd_t) {
        .type = RM690B0_DL_CMD_FILL_CIRCLE,
        .color_swapped = color_swapped,
        .u.circle = {
            .x = x,
            .y = y,
            .radius = radius,
        },
    };
    rm690b0_dl_note_command_count(dl);

    mp_int_t diameter = radius * 2 + 1;
    rm690b0_dl_mark_dirty(dl, x - radius, y - radius, diameter, diameter);
    return ESP_OK;
}

esp_err_t rm690b0_dl_enqueue_blit_pixels(rm690b0_rm690b0_obj_t *self,
    mp_int_t logical_x, mp_int_t logical_y, mp_int_t logical_w, mp_int_t logical_h,
    const uint16_t *src_pixels, size_t src_stride, bool src_is_swapped,
    bool has_transparency, uint16_t transparency_src) {
    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || self->render_mode != RM690B0_RENDER_DISPLAY_LIST) {
        return ESP_ERR_INVALID_STATE;
    }
    if (src_pixels == NULL || logical_w <= 0 || logical_h <= 0) {
        return ESP_ERR_INVALID_ARG;
    }

    mp_int_t phys_x = logical_x;
    mp_int_t phys_y = logical_y;
    mp_int_t phys_w = logical_w;
    mp_int_t phys_h = logical_h;
    if (!map_rect_for_rotation(self, &phys_x, &phys_y, &phys_w, &phys_h)) {
        return ESP_OK;
    }
    if (phys_w <= 0 || phys_h <= 0) {
        return ESP_OK;
    }

    rm690b0_dl_state_t *dl = &impl->dl;
    bool replace_frame = !has_transparency &&
        phys_x == 0 &&
        phys_y == 0 &&
        phys_w == RM690B0_PANEL_WIDTH &&
        phys_h == RM690B0_PANEL_HEIGHT;
    if (replace_frame) {
        rm690b0_dl_release_payloads(dl);
        dl->count = 0;
        dl->clear_color_valid = false;
    }

    size_t payload_bytes = 0;
    if (!check_bitmap_size((size_t)phys_w, (size_t)phys_h, &payload_bytes)) {
        return ESP_ERR_NO_MEM;
    }

    if (!rm690b0_dl_reserve(dl, dl->count + 1)) {
        return ESP_ERR_NO_MEM;
    }
    if (dl->payload_bytes > RM690B0_DL_MAX_PAYLOAD_BYTES ||
        payload_bytes > (RM690B0_DL_MAX_PAYLOAD_BYTES - dl->payload_bytes)) {
        dl->telemetry_rejected_payload_limit++;
        return ESP_ERR_NO_MEM;
    }

    size_t pixel_count = payload_bytes / sizeof(uint16_t);
    uint16_t *pixels_swapped = (uint16_t *)heap_caps_malloc(pixel_count * sizeof(uint16_t),
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (pixels_swapped == NULL) {
        pixels_swapped = (uint16_t *)heap_caps_malloc(pixel_count * sizeof(uint16_t), MALLOC_CAP_8BIT);
    }
    if (pixels_swapped == NULL) {
        dl->telemetry_allocation_failures++;
        return ESP_ERR_NO_MEM;
    }

    switch (self->rotation) {
        case 0:
            for (mp_int_t row = 0; row < logical_h; row++) {
                const uint16_t *src_row = src_pixels + (size_t)row * src_stride;
                uint16_t *dst_row = pixels_swapped + (size_t)row * (size_t)phys_w;
                for (mp_int_t col = 0; col < logical_w; col++) {
                    dst_row[col] = rm690b0_dl_to_swapped(src_row[col], src_is_swapped);
                }
            }
            break;
        case 180:
            for (mp_int_t row = 0; row < phys_h; row++) {
                const uint16_t *src_row = src_pixels + (size_t)(logical_h - 1 - row) * src_stride;
                uint16_t *dst_row = pixels_swapped + (size_t)row * (size_t)phys_w;
                for (mp_int_t col = 0; col < phys_w; col++) {
                    uint16_t val = src_row[logical_w - 1 - col];
                    dst_row[col] = rm690b0_dl_to_swapped(val, src_is_swapped);
                }
            }
            break;
        case 90:
            for (mp_int_t row = 0; row < phys_h; row++) {
                uint16_t *dst_row = pixels_swapped + (size_t)row * (size_t)phys_w;
                for (mp_int_t col = 0; col < phys_w; col++) {
                    mp_int_t src_row_idx = logical_h - 1 - col;
                    mp_int_t src_col_idx = row;
                    const uint16_t *src_row = src_pixels + (size_t)src_row_idx * src_stride;
                    dst_row[col] = rm690b0_dl_to_swapped(src_row[src_col_idx], src_is_swapped);
                }
            }
            break;
        case 270:
            for (mp_int_t row = 0; row < phys_h; row++) {
                uint16_t *dst_row = pixels_swapped + (size_t)row * (size_t)phys_w;
                for (mp_int_t col = 0; col < phys_w; col++) {
                    mp_int_t src_row_idx = col;
                    mp_int_t src_col_idx = logical_w - 1 - row;
                    const uint16_t *src_row = src_pixels + (size_t)src_row_idx * src_stride;
                    dst_row[col] = rm690b0_dl_to_swapped(src_row[src_col_idx], src_is_swapped);
                }
            }
            break;
        default:
            heap_caps_free(pixels_swapped);
            return ESP_ERR_INVALID_ARG;
    }

    dl->items[dl->count++] = (rm690b0_dl_cmd_t) {
        .type = RM690B0_DL_CMD_BLIT,
        .u.blit = {
            .x = phys_x,
            .y = phys_y,
            .w = phys_w,
            .h = phys_h,
            .pixels_swapped = pixels_swapped,
            .payload_bytes = payload_bytes,
            .transparency_swapped = rm690b0_dl_to_swapped(transparency_src, src_is_swapped),
            .has_transparency = has_transparency ? 1 : 0,
        },
    };
    rm690b0_dl_note_command_count(dl);
    dl->payload_bytes += payload_bytes;
    rm690b0_dl_note_payload_bytes(dl);
    if (replace_frame) {
        dl->force_full_present = true;
    }
    rm690b0_dl_mark_dirty(dl, phys_x, phys_y, phys_w, phys_h);
    return ESP_OK;
}

esp_err_t rm690b0_dl_enqueue_glyph(rm690b0_rm690b0_obj_t *self,
    mp_int_t x, mp_int_t y, uint8_t font_id, uint8_t ch,
    uint16_t fg_swapped, bool has_bg, uint16_t bg_swapped) {
    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || self->render_mode != RM690B0_RENDER_DISPLAY_LIST) {
        return ESP_ERR_INVALID_STATE;
    }

    mp_int_t glyph_w = 0;
    mp_int_t glyph_h = 0;
    rm690b0_dl_get_font_dims(font_id, &glyph_w, &glyph_h);

    mp_int_t clip_x = x;
    mp_int_t clip_y = y;
    mp_int_t clip_w = glyph_w;
    mp_int_t clip_h = glyph_h;
    if (!clip_logical_rect(self, &clip_x, &clip_y, &clip_w, &clip_h)) {
        return ESP_OK;
    }

    mp_int_t bbox_x = x;
    mp_int_t bbox_y = y;
    mp_int_t bbox_w = glyph_w;
    mp_int_t bbox_h = glyph_h;
    if (!map_rect_for_rotation(self, &bbox_x, &bbox_y, &bbox_w, &bbox_h)) {
        return ESP_OK;
    }
    if (bbox_w <= 0 || bbox_h <= 0) {
        return ESP_OK;
    }

    uint8_t normalized_ch = rm690b0_dl_normalize_glyph_char(font_id, ch);

    rm690b0_dl_state_t *dl = &impl->dl;
    if (!rm690b0_dl_reserve(dl, dl->count + 1)) {
        return ESP_ERR_NO_MEM;
    }

    dl->items[dl->count++] = (rm690b0_dl_cmd_t) {
        .type = RM690B0_DL_CMD_GLYPH,
        .color_swapped = fg_swapped,
        .u.glyph = {
            .x = x,
            .y = y,
            .bg_swapped = bg_swapped,
            .font_id = font_id,
            .rotation = self->rotation,
            .ch = normalized_ch,
            .has_bg = has_bg ? 1 : 0,
            .atlas_hint = RM690B0_DL_GLYPH_ATLAS_HINT_INVALID,
            .bbox_x = (int16_t)bbox_x,
            .bbox_y = (int16_t)bbox_y,
            .bbox_w = (uint16_t)bbox_w,
            .bbox_h = (uint16_t)bbox_h,
        },
    };
    rm690b0_dl_note_command_count(dl);

    rm690b0_dl_mark_dirty(dl, bbox_x, bbox_y, bbox_w, bbox_h);
    return ESP_OK;
}

static bool rm690b0_dl_is_fullscreen_opaque_cmd(const rm690b0_dl_cmd_t *cmd) {
    if (cmd == NULL) {
        return false;
    }
    if (cmd->type == RM690B0_DL_CMD_FILL_COLOR) {
        return true;
    }
    if (cmd->type == RM690B0_DL_CMD_FILL_RECT) {
        return cmd->u.rect.x <= 0 &&
            cmd->u.rect.y <= 0 &&
            cmd->u.rect.x + cmd->u.rect.w >= RM690B0_PANEL_WIDTH &&
            cmd->u.rect.y + cmd->u.rect.h >= RM690B0_PANEL_HEIGHT;
    }
    if (cmd->type == RM690B0_DL_CMD_BLIT && !cmd->u.blit.has_transparency) {
        return cmd->u.blit.x <= 0 &&
            cmd->u.blit.y <= 0 &&
            cmd->u.blit.x + cmd->u.blit.w >= RM690B0_PANEL_WIDTH &&
            cmd->u.blit.y + cmd->u.blit.h >= RM690B0_PANEL_HEIGHT;
    }
    return false;
}

static bool rm690b0_dl_is_opaque_cover_cmd(const rm690b0_dl_cmd_t *cmd) {
    if (cmd == NULL) {
        return false;
    }
    if (cmd->type == RM690B0_DL_CMD_FILL_COLOR) {
        return true;
    }
    if (cmd->type == RM690B0_DL_CMD_FILL_RECT) {
        return true;
    }
    if (cmd->type == RM690B0_DL_CMD_BLIT && !cmd->u.blit.has_transparency) {
        return true;
    }
    return false;
}

static bool rm690b0_dl_get_cmd_bbox(const rm690b0_dl_cmd_t *cmd,
    mp_int_t *x, mp_int_t *y, mp_int_t *w, mp_int_t *h) {
    if (cmd == NULL || x == NULL || y == NULL || w == NULL || h == NULL) {
        return false;
    }

    switch (cmd->type) {
        case RM690B0_DL_CMD_FILL_COLOR:
            *x = 0;
            *y = 0;
            *w = RM690B0_PANEL_WIDTH;
            *h = RM690B0_PANEL_HEIGHT;
            return true;
        case RM690B0_DL_CMD_FILL_RECT:
            *x = cmd->u.rect.x;
            *y = cmd->u.rect.y;
            *w = cmd->u.rect.w;
            *h = cmd->u.rect.h;
            return *w > 0 && *h > 0;
        case RM690B0_DL_CMD_PIXEL:
            *x = cmd->u.pixel.x;
            *y = cmd->u.pixel.y;
            *w = 1;
            *h = 1;
            return true;
        case RM690B0_DL_CMD_LINE: {
            mp_int_t min_x = cmd->u.line.x0 < cmd->u.line.x1 ? cmd->u.line.x0 : cmd->u.line.x1;
            mp_int_t max_x = cmd->u.line.x0 > cmd->u.line.x1 ? cmd->u.line.x0 : cmd->u.line.x1;
            mp_int_t min_y = cmd->u.line.y0 < cmd->u.line.y1 ? cmd->u.line.y0 : cmd->u.line.y1;
            mp_int_t max_y = cmd->u.line.y0 > cmd->u.line.y1 ? cmd->u.line.y0 : cmd->u.line.y1;
            *x = min_x;
            *y = min_y;
            *w = max_x - min_x + 1;
            *h = max_y - min_y + 1;
            return true;
        }
        case RM690B0_DL_CMD_CIRCLE:
        case RM690B0_DL_CMD_FILL_CIRCLE: {
            mp_int_t radius = cmd->u.circle.radius;
            if (radius < 0) {
                return false;
            }
            *x = cmd->u.circle.x - radius;
            *y = cmd->u.circle.y - radius;
            *w = radius * 2 + 1;
            *h = radius * 2 + 1;
            return true;
        }
        case RM690B0_DL_CMD_BLIT:
            *x = cmd->u.blit.x;
            *y = cmd->u.blit.y;
            *w = cmd->u.blit.w;
            *h = cmd->u.blit.h;
            return *w > 0 && *h > 0;
        case RM690B0_DL_CMD_GLYPH: {
            *x = cmd->u.glyph.bbox_x;
            *y = cmd->u.glyph.bbox_y;
            *w = (mp_int_t)cmd->u.glyph.bbox_w;
            *h = (mp_int_t)cmd->u.glyph.bbox_h;
            return *w > 0 && *h > 0;
        }
        default:
            return false;
    }
}

static bool rm690b0_dl_rect_contains(mp_int_t outer_x, mp_int_t outer_y, mp_int_t outer_w, mp_int_t outer_h,
    mp_int_t inner_x, mp_int_t inner_y, mp_int_t inner_w, mp_int_t inner_h) {
    if (outer_w <= 0 || outer_h <= 0 || inner_w <= 0 || inner_h <= 0) {
        return false;
    }
    mp_int_t outer_x2 = outer_x + outer_w;
    mp_int_t outer_y2 = outer_y + outer_h;
    mp_int_t inner_x2 = inner_x + inner_w;
    mp_int_t inner_y2 = inner_y + inner_h;
    return inner_x >= outer_x &&
        inner_y >= outer_y &&
        inner_x2 <= outer_x2 &&
        inner_y2 <= outer_y2;
}

typedef struct {
    mp_int_t x;
    mp_int_t y;
    mp_int_t w;
    mp_int_t h;
} rm690b0_dl_cover_bbox_t;

static void rm690b0_dl_free_cmd_payload(rm690b0_dl_state_t *dl, rm690b0_dl_cmd_t *cmd) {
    if (dl == NULL || cmd == NULL) {
        return;
    }
    if (cmd->type == RM690B0_DL_CMD_BLIT && cmd->u.blit.pixels_swapped != NULL) {
        if (dl->payload_bytes >= cmd->u.blit.payload_bytes) {
            dl->payload_bytes -= cmd->u.blit.payload_bytes;
        } else {
            dl->payload_bytes = 0;
        }
        heap_caps_free(cmd->u.blit.pixels_swapped);
        cmd->u.blit.pixels_swapped = NULL;
        cmd->u.blit.payload_bytes = 0;
    }
}

esp_err_t rm690b0_dl_compact(rm690b0_rm690b0_obj_t *self) {
    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    if (self->render_mode != RM690B0_RENDER_DISPLAY_LIST) {
        return ESP_ERR_INVALID_STATE;
    }

    rm690b0_dl_state_t *dl = &impl->dl;
    if (dl->items == NULL || dl->count <= 1) {
        return ESP_OK;
    }

    bool trimmed_any = false;
    size_t keep_from = 0;
    bool found = false;
    for (size_t i = dl->count; i > 0; i--) {
        size_t idx = i - 1;
        if (rm690b0_dl_is_fullscreen_opaque_cmd(&dl->items[idx])) {
            keep_from = idx;
            found = true;
            break;
        }
    }
    if (found && keep_from > 0) {
        dl->telemetry_compact_count++;
        dl->telemetry_compact_trimmed_commands += keep_from;
        trimmed_any = true;

        for (size_t i = 0; i < keep_from; i++) {
            rm690b0_dl_free_cmd_payload(dl, &dl->items[i]);
        }

        size_t remaining = dl->count - keep_from;
        memmove(dl->items, dl->items + keep_from, remaining * sizeof(*dl->items));
        dl->count = remaining;
    }

    if (dl->count > 1) {
        bool *drop = m_new_maybe(bool, dl->count);
        if (drop != NULL) {
            size_t count_before = dl->count;
            memset(drop, 0, count_before * sizeof(bool));

            size_t trimmed_overlap = 0;
            rm690b0_dl_cover_bbox_t *cover_boxes = m_new_maybe(rm690b0_dl_cover_bbox_t, count_before);
            if (cover_boxes != NULL) {
                // Reverse scan: cover_boxes holds opaque cover bboxes from commands with index > i.
                size_t cover_count = 0;
                for (size_t rev = count_before; rev > 0; rev--) {
                    size_t i = rev - 1;
                    const rm690b0_dl_cmd_t *cmd = &dl->items[i];
                    mp_int_t ix = 0, iy = 0, iw = 0, ih = 0;
                    bool has_bbox = rm690b0_dl_get_cmd_bbox(cmd, &ix, &iy, &iw, &ih);

                    if (has_bbox) {
                        for (size_t c = 0; c < cover_count; c++) {
                            const rm690b0_dl_cover_bbox_t *cover = &cover_boxes[c];
                            if (rm690b0_dl_rect_contains(
                                cover->x, cover->y, cover->w, cover->h,
                                ix, iy, iw, ih)) {
                                drop[i] = true;
                                trimmed_overlap++;
                                break;
                            }
                        }
                    }

                    if (!rm690b0_dl_is_opaque_cover_cmd(cmd) || !has_bbox) {
                        continue;
                    }

                    // If an existing later cover already contains this one, adding it is redundant.
                    bool redundant_cover = false;
                    for (size_t c = 0; c < cover_count; c++) {
                        const rm690b0_dl_cover_bbox_t *cover = &cover_boxes[c];
                        if (rm690b0_dl_rect_contains(
                            cover->x, cover->y, cover->w, cover->h,
                            ix, iy, iw, ih)) {
                            redundant_cover = true;
                            break;
                        }
                    }
                    if (redundant_cover) {
                        continue;
                    }

                    // Prune dominated covers to keep future containment scans short.
                    size_t write_idx = 0;
                    for (size_t c = 0; c < cover_count; c++) {
                        const rm690b0_dl_cover_bbox_t *cover = &cover_boxes[c];
                        if (rm690b0_dl_rect_contains(
                            ix, iy, iw, ih,
                            cover->x, cover->y, cover->w, cover->h)) {
                            continue;
                        }
                        if (write_idx != c) {
                            cover_boxes[write_idx] = *cover;
                        }
                        write_idx++;
                    }
                    cover_count = write_idx;
                    cover_boxes[cover_count++] = (rm690b0_dl_cover_bbox_t) {
                        .x = ix,
                        .y = iy,
                        .w = iw,
                        .h = ih,
                    };
                }
                m_del(rm690b0_dl_cover_bbox_t, cover_boxes, count_before);
            } else {
                // Fallback when temporary memory for optimized pass is unavailable.
                for (size_t i = 0; i + 1 < count_before; i++) {
                    mp_int_t ix = 0, iy = 0, iw = 0, ih = 0;
                    if (!rm690b0_dl_get_cmd_bbox(&dl->items[i], &ix, &iy, &iw, &ih)) {
                        continue;
                    }
                    for (size_t j = i + 1; j < count_before; j++) {
                        if (!rm690b0_dl_is_opaque_cover_cmd(&dl->items[j])) {
                            continue;
                        }
                        mp_int_t jx = 0, jy = 0, jw = 0, jh = 0;
                        if (!rm690b0_dl_get_cmd_bbox(&dl->items[j], &jx, &jy, &jw, &jh)) {
                            continue;
                        }
                        if (rm690b0_dl_rect_contains(jx, jy, jw, jh, ix, iy, iw, ih)) {
                            drop[i] = true;
                            trimmed_overlap++;
                            break;
                        }
                    }
                }
            }

            if (trimmed_overlap > 0) {
                trimmed_any = true;
                dl->telemetry_compact_count++;
                dl->telemetry_compact_trimmed_commands += trimmed_overlap;

                size_t write_idx = 0;
                for (size_t i = 0; i < count_before; i++) {
                    if (drop[i]) {
                        rm690b0_dl_free_cmd_payload(dl, &dl->items[i]);
                        continue;
                    }
                    if (write_idx != i) {
                        dl->items[write_idx] = dl->items[i];
                    }
                    write_idx++;
                }
                dl->count = write_idx;
            }

            m_del(bool, drop, count_before);
        }
    }

    if (!trimmed_any) {
        return ESP_OK;
    }

    if (dl->count > 0 && dl->items[0].type == RM690B0_DL_CMD_FILL_COLOR) {
        dl->clear_color_valid = true;
        dl->clear_color_swapped = dl->items[0].color_swapped;
    } else {
        dl->clear_color_valid = false;
    }

    dl->force_full_present = true;
    rm690b0_dl_mark_dirty(dl, 0, 0, RM690B0_PANEL_WIDTH, RM690B0_PANEL_HEIGHT);
    return ESP_OK;
}

esp_err_t rm690b0_dl_present(rm690b0_rm690b0_obj_t *self, bool keep_commands) {
    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || impl->panel_handle == NULL || impl->transfer_done_sem == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    rm690b0_dl_state_t *dl = &impl->dl;
    if (!dl->force_full_present && !dl->dirty_valid) {
        // In retained mode nothing changed, so there is no panel update to perform.
        if (keep_commands || dl->count == 0) {
            return ESP_OK;
        }
    }

    const size_t panel_width = RM690B0_PANEL_WIDTH;
    const size_t panel_height = RM690B0_PANEL_HEIGHT;
    mp_int_t present_x = 0;
    mp_int_t present_y = 0;
    mp_int_t present_w = (mp_int_t)panel_width;
    mp_int_t present_h = (mp_int_t)panel_height;

    // Dirty-limited transfer applies whenever dirty bounds are valid and full present is not forced.
    if (!dl->force_full_present && dl->dirty_valid) {
        present_x = dl->dirty_x;
        present_y = dl->dirty_y;
        present_w = dl->dirty_w;
        present_h = dl->dirty_h;
        if (!expand_even_region(&present_x, &present_y, &present_w, &present_h)) {
            return ESP_OK;
        }
    }
    if (present_w <= 0 || present_h <= 0) {
        return ESP_OK;
    }
    mp_int_t present_x_end = present_x + present_w;
    mp_int_t present_y_end = present_y + present_h;
    bool full_present = present_x == 0 &&
        present_y == 0 &&
        present_w == (mp_int_t)panel_width &&
        present_h == (mp_int_t)panel_height;
    size_t chunk_width = (size_t)present_w;

    size_t chunk_rows = RM690B0_MAX_CHUNK_ROWS;
    if (impl->chunk_buffers[0] != NULL && impl->chunk_buffer_pixels > 0) {
        size_t static_rows = impl->chunk_buffer_pixels / chunk_width;
        if (static_rows > 0 && static_rows < chunk_rows) {
            chunk_rows = static_rows;
        }
    }
    if (chunk_rows == 0) {
        chunk_rows = 1;
    }
    if (chunk_rows > (size_t)present_h) {
        chunk_rows = (size_t)present_h;
    }

    size_t chunk_pixels = chunk_width * chunk_rows;
    bool use_static_buffers = impl->chunk_buffers[0] != NULL &&
        impl->chunk_buffer_pixels >= chunk_pixels;
    bool use_ping_pong = use_static_buffers &&
        impl->chunk_buffers[1] != NULL &&
        impl->chunk_buffer_pixels >= chunk_pixels;
    uint16_t *alloc_buffer = NULL;
    if (!use_static_buffers) {
        alloc_buffer = (uint16_t *)heap_caps_malloc(chunk_pixels * sizeof(uint16_t), MALLOC_CAP_DMA);
        if (alloc_buffer == NULL) {
            dl->telemetry_allocation_failures++;
            return ESP_ERR_NO_MEM;
        }
        impl->dma_alloc_buffer_ptr = alloc_buffer;
    }

    uint16_t clear_color = dl->clear_color_valid ? dl->clear_color_swapped : 0;
    esp_err_t ret = ESP_OK;
    int buf_idx = 0;

    for (mp_int_t start_y = present_y; start_y < present_y_end && ret == ESP_OK; start_y += (mp_int_t)chunk_rows) {
        size_t rows_this_chunk = (size_t)(present_y_end - start_y);
        if (rows_this_chunk > chunk_rows) {
            rows_this_chunk = chunk_rows;
        }

        rm690b0_wait_for_dma_slot(impl);

        uint16_t *chunk_buffer = NULL;
        uint8_t pending_id = RM690B0_PENDING_BUFFER_TEMP;
        if (use_static_buffers) {
            uint8_t current_idx = use_ping_pong ? (uint8_t)buf_idx : 0;
            while (impl->dma_buffer_in_use[current_idx]) {
                rm690b0_wait_for_dma_completion(impl);
            }
            chunk_buffer = impl->chunk_buffers[current_idx];
            impl->dma_buffer_in_use[current_idx] = true;
            pending_id = current_idx;
            if (use_ping_pong) {
                buf_idx = (buf_idx + 1) % 2;
            } else {
                buf_idx = 0;
            }
        } else {
            while (impl->dma_alloc_buffer_in_use) {
                rm690b0_wait_for_dma_completion(impl);
            }
            chunk_buffer = alloc_buffer;
            impl->dma_alloc_buffer_in_use = true;
            pending_id = RM690B0_PENDING_BUFFER_ALLOC;
        }

        rm690b0_fill_span_fast(chunk_buffer, chunk_width * rows_this_chunk, clear_color);

        for (size_t i = 0; i < dl->count; i++) {
            rm690b0_dl_cmd_t *cmd = &dl->items[i];
            if (cmd->type == RM690B0_DL_CMD_FILL_COLOR) {
                rm690b0_fill_span_fast(chunk_buffer, chunk_width * rows_this_chunk, cmd->color_swapped);
                continue;
            }

            if (cmd->type == RM690B0_DL_CMD_FILL_RECT) {
                mp_int_t cmd_y0 = cmd->u.rect.y;
                mp_int_t cmd_y1 = cmd->u.rect.y + cmd->u.rect.h;
                mp_int_t chunk_y0 = start_y;
                mp_int_t chunk_y1 = start_y + (mp_int_t)rows_this_chunk;

                if (cmd_y1 <= chunk_y0 || cmd_y0 >= chunk_y1) {
                    continue;
                }

                mp_int_t draw_y0 = cmd_y0 > chunk_y0 ? cmd_y0 : chunk_y0;
                mp_int_t draw_y1 = cmd_y1 < chunk_y1 ? cmd_y1 : chunk_y1;
                mp_int_t draw_x0 = cmd->u.rect.x;
                if (draw_x0 < present_x) {
                    draw_x0 = present_x;
                }
                mp_int_t draw_x1 = cmd->u.rect.x + cmd->u.rect.w;
                if (draw_x1 > present_x_end) {
                    draw_x1 = present_x_end;
                }
                if (draw_x1 <= draw_x0) {
                    continue;
                }

                size_t span_w = (size_t)(draw_x1 - draw_x0);
                for (mp_int_t yy = draw_y0; yy < draw_y1; yy++) {
                    size_t local_row = (size_t)(yy - start_y);
                    size_t local_x = (size_t)(draw_x0 - present_x);
                    uint16_t *dst = chunk_buffer + local_row * chunk_width + local_x;
                    rm690b0_fill_span_fast(dst, span_w, cmd->color_swapped);
                }
                continue;
            }

            if (cmd->type == RM690B0_DL_CMD_PIXEL) {
                mp_int_t px = cmd->u.pixel.x;
                mp_int_t py = cmd->u.pixel.y;
                if (py >= start_y && py < start_y + (mp_int_t)rows_this_chunk &&
                    px >= present_x && px < present_x_end) {
                    rm690b0_dl_plot_pixel(
                        chunk_buffer,
                        chunk_width,
                        rows_this_chunk,
                        present_x,
                        start_y,
                        px,
                        py,
                        cmd->color_swapped);
                }
                continue;
            }

            if (cmd->type == RM690B0_DL_CMD_LINE) {
                mp_int_t x0 = cmd->u.line.x0;
                mp_int_t y0 = cmd->u.line.y0;
                mp_int_t x1 = cmd->u.line.x1;
                mp_int_t y1 = cmd->u.line.y1;
                mp_int_t chunk_y0 = start_y;
                mp_int_t chunk_y1 = start_y + (mp_int_t)rows_this_chunk;
                mp_int_t line_min_x = x0 < x1 ? x0 : x1;
                mp_int_t line_max_x = x0 > x1 ? x0 : x1;
                mp_int_t line_min_y = y0 < y1 ? y0 : y1;
                mp_int_t line_max_y = y0 > y1 ? y0 : y1;
                if (line_max_x < present_x || line_min_x >= present_x_end ||
                    line_max_y < chunk_y0 || line_min_y >= chunk_y1) {
                    continue;
                }

                mp_int_t dx = labs(x1 - x0);
                mp_int_t dy = labs(y1 - y0);
                mp_int_t sx = (x0 < x1) ? 1 : -1;
                mp_int_t sy = (y0 < y1) ? 1 : -1;
                mp_int_t err = dx - dy;

                while (true) {
                    if (y0 >= start_y && y0 < start_y + (mp_int_t)rows_this_chunk &&
                        x0 >= present_x && x0 < present_x_end) {
                        rm690b0_dl_plot_pixel(
                            chunk_buffer,
                            chunk_width,
                            rows_this_chunk,
                            present_x,
                            start_y,
                            x0,
                            y0,
                            cmd->color_swapped);
                    }
                    if (x0 == x1 && y0 == y1) {
                        break;
                    }
                    mp_int_t e2 = err << 1;
                    if (e2 > -dy) {
                        err -= dy;
                        x0 += sx;
                    }
                    if (e2 < dx) {
                        err += dx;
                        y0 += sy;
                    }
                }
                continue;
            }

            if (cmd->type == RM690B0_DL_CMD_CIRCLE) {
                mp_int_t cx = cmd->u.circle.x;
                mp_int_t cy = cmd->u.circle.y;
                mp_int_t radius = cmd->u.circle.radius;
                mp_int_t chunk_y0 = start_y;
                mp_int_t chunk_y1 = start_y + (mp_int_t)rows_this_chunk;
                mp_int_t circle_x0 = cx - radius;
                mp_int_t circle_x1 = cx + radius;
                mp_int_t circle_y0 = cy - radius;
                mp_int_t circle_y1 = cy + radius;
                if (circle_x1 < present_x || circle_x0 >= present_x_end ||
                    circle_y1 < chunk_y0 || circle_y0 >= chunk_y1) {
                    continue;
                }
                mp_int_t x = 0;
                mp_int_t y = radius;
                mp_int_t d = 1 - radius;

                while (x <= y) {
                    rm690b0_dl_plot_pixel(chunk_buffer, chunk_width, rows_this_chunk, present_x, start_y, cx + x, cy + y, cmd->color_swapped);
                    rm690b0_dl_plot_pixel(chunk_buffer, chunk_width, rows_this_chunk, present_x, start_y, cx - x, cy + y, cmd->color_swapped);
                    rm690b0_dl_plot_pixel(chunk_buffer, chunk_width, rows_this_chunk, present_x, start_y, cx + x, cy - y, cmd->color_swapped);
                    rm690b0_dl_plot_pixel(chunk_buffer, chunk_width, rows_this_chunk, present_x, start_y, cx - x, cy - y, cmd->color_swapped);
                    rm690b0_dl_plot_pixel(chunk_buffer, chunk_width, rows_this_chunk, present_x, start_y, cx + y, cy + x, cmd->color_swapped);
                    rm690b0_dl_plot_pixel(chunk_buffer, chunk_width, rows_this_chunk, present_x, start_y, cx - y, cy + x, cmd->color_swapped);
                    rm690b0_dl_plot_pixel(chunk_buffer, chunk_width, rows_this_chunk, present_x, start_y, cx + y, cy - x, cmd->color_swapped);
                    rm690b0_dl_plot_pixel(chunk_buffer, chunk_width, rows_this_chunk, present_x, start_y, cx - y, cy - x, cmd->color_swapped);

                    x++;
                    if (d < 0) {
                        d += (x << 1) + 1;
                    } else {
                        y--;
                        d += ((x - y) << 1) + 1;
                    }
                }
                continue;
            }

            if (cmd->type == RM690B0_DL_CMD_FILL_CIRCLE) {
                mp_int_t cx = cmd->u.circle.x;
                mp_int_t cy = cmd->u.circle.y;
                mp_int_t radius = cmd->u.circle.radius;
                mp_int_t chunk_y0 = start_y;
                mp_int_t chunk_y1 = start_y + (mp_int_t)rows_this_chunk;
                mp_int_t circle_x0 = cx - radius;
                mp_int_t circle_x1 = cx + radius;
                mp_int_t circle_y0 = cy - radius;
                mp_int_t circle_y1 = cy + radius;
                if (circle_x1 < present_x || circle_x0 >= present_x_end ||
                    circle_y1 < chunk_y0 || circle_y0 >= chunk_y1) {
                    continue;
                }
                mp_int_t x = 0;
                mp_int_t y = radius;
                mp_int_t d = 1 - radius;

                while (x <= y) {
                    rm690b0_dl_fill_span_clipped(
                        chunk_buffer, chunk_width, rows_this_chunk,
                        present_x, start_y,
                        cy + y, cx - x, cx + x, cmd->color_swapped);
                    rm690b0_dl_fill_span_clipped(
                        chunk_buffer, chunk_width, rows_this_chunk,
                        present_x, start_y,
                        cy - y, cx - x, cx + x, cmd->color_swapped);

                    if (x != y) {
                        rm690b0_dl_fill_span_clipped(
                            chunk_buffer, chunk_width, rows_this_chunk,
                            present_x, start_y,
                            cy + x, cx - y, cx + y, cmd->color_swapped);
                        rm690b0_dl_fill_span_clipped(
                            chunk_buffer, chunk_width, rows_this_chunk,
                            present_x, start_y,
                            cy - x, cx - y, cx + y, cmd->color_swapped);
                    }

                    x++;
                    if (d < 0) {
                        d += (x << 1) + 1;
                    } else {
                        y--;
                        d += ((x - y) << 1) + 1;
                    }
                }
                continue;
            }

            if (cmd->type == RM690B0_DL_CMD_BLIT) {
                mp_int_t cmd_x0 = cmd->u.blit.x;
                mp_int_t cmd_y0 = cmd->u.blit.y;
                mp_int_t cmd_x1 = cmd->u.blit.x + cmd->u.blit.w;
                mp_int_t cmd_y1 = cmd->u.blit.y + cmd->u.blit.h;
                mp_int_t chunk_y0 = start_y;
                mp_int_t chunk_y1 = start_y + (mp_int_t)rows_this_chunk;

                if (cmd_y1 <= chunk_y0 || cmd_y0 >= chunk_y1) {
                    continue;
                }

                mp_int_t draw_y0 = cmd_y0 > chunk_y0 ? cmd_y0 : chunk_y0;
                mp_int_t draw_y1 = cmd_y1 < chunk_y1 ? cmd_y1 : chunk_y1;
                mp_int_t draw_x0 = cmd_x0;
                if (draw_x0 < present_x) {
                    draw_x0 = present_x;
                }
                mp_int_t draw_x1 = cmd_x1 > present_x_end ? present_x_end : cmd_x1;
                if (draw_x1 <= draw_x0) {
                    continue;
                }

                size_t span_w = (size_t)(draw_x1 - draw_x0);
                size_t src_x_offset = (size_t)(draw_x0 - cmd_x0);
                uint16_t transparency = cmd->u.blit.transparency_swapped;

                for (mp_int_t yy = draw_y0; yy < draw_y1; yy++) {
                    size_t src_row = (size_t)(yy - cmd_y0);
                    const uint16_t *src = cmd->u.blit.pixels_swapped + src_row * (size_t)cmd->u.blit.w + src_x_offset;
                    size_t local_x = (size_t)(draw_x0 - present_x);
                    uint16_t *dst = chunk_buffer + (size_t)(yy - start_y) * chunk_width + local_x;
                    if (!cmd->u.blit.has_transparency) {
                        memcpy(dst, src, span_w * sizeof(uint16_t));
                    } else {
                        rm690b0_dl_copy_transparent_runs(dst, src, span_w, transparency);
                    }
                }
                continue;
            }

            if (cmd->type == RM690B0_DL_CMD_GLYPH) {
                rm690b0_dl_glyph_cache_entry_t *glyph_entry = rm690b0_dl_get_glyph_cache_entry(
                    dl,
                    cmd->u.glyph.font_id,
                    cmd->u.glyph.ch,
                    cmd->u.glyph.rotation,
                    &cmd->u.glyph.atlas_hint);
                if (glyph_entry == NULL || !glyph_entry->valid || glyph_entry->width == 0 || glyph_entry->height == 0) {
                    continue;
                }

                mp_int_t bbox_x = cmd->u.glyph.bbox_x;
                mp_int_t bbox_y = cmd->u.glyph.bbox_y;
                mp_int_t bbox_w = (mp_int_t)cmd->u.glyph.bbox_w;
                mp_int_t bbox_h = (mp_int_t)cmd->u.glyph.bbox_h;
                mp_int_t chunk_y0 = start_y;
                mp_int_t chunk_y1 = start_y + (mp_int_t)rows_this_chunk;
                if (bbox_w <= 0 || bbox_h <= 0 ||
                    bbox_x + bbox_w <= present_x || bbox_x >= present_x_end ||
                    bbox_y + bbox_h <= chunk_y0 || bbox_y >= chunk_y1) {
                    continue;
                }

                if (cmd->u.glyph.has_bg) {
                    mp_int_t bg_x0 = bbox_x;
                    if (bg_x0 < present_x) {
                        bg_x0 = present_x;
                    }
                    mp_int_t bg_x1 = bbox_x + bbox_w;
                    if (bg_x1 > present_x_end) {
                        bg_x1 = present_x_end;
                    }
                    mp_int_t bg_y0 = bbox_y;
                    if (bg_y0 < chunk_y0) {
                        bg_y0 = chunk_y0;
                    }
                    mp_int_t bg_y1 = bbox_y + bbox_h;
                    if (bg_y1 > chunk_y1) {
                        bg_y1 = chunk_y1;
                    }
                    if (bg_x1 > bg_x0 && bg_y1 > bg_y0) {
                        size_t span_w = (size_t)(bg_x1 - bg_x0);
                        for (mp_int_t yy = bg_y0; yy < bg_y1; yy++) {
                            size_t local_row = (size_t)(yy - start_y);
                            size_t local_x = (size_t)(bg_x0 - present_x);
                            uint16_t *dst = chunk_buffer + local_row * chunk_width + local_x;
                            rm690b0_fill_span_fast(dst, span_w, cmd->u.glyph.bg_swapped);
                        }
                    }
                }

                mp_int_t fg_x0 = bbox_x;
                if (fg_x0 < present_x) {
                    fg_x0 = present_x;
                }
                mp_int_t fg_x1 = bbox_x + bbox_w;
                if (fg_x1 > present_x_end) {
                    fg_x1 = present_x_end;
                }
                mp_int_t fg_y0 = bbox_y;
                if (fg_y0 < chunk_y0) {
                    fg_y0 = chunk_y0;
                }
                mp_int_t fg_y1 = bbox_y + bbox_h;
                if (fg_y1 > chunk_y1) {
                    fg_y1 = chunk_y1;
                }
                if (fg_x1 <= fg_x0 || fg_y1 <= fg_y0) {
                    continue;
                }

                uint8_t local_x0 = (uint8_t)(fg_x0 - bbox_x);
                uint8_t local_x1 = (uint8_t)(fg_x1 - bbox_x);
                uint64_t x_mask = rm690b0_dl_mask_range(local_x0, local_x1);
                if (x_mask == 0) {
                    continue;
                }

                for (mp_int_t yy = fg_y0; yy < fg_y1; yy++) {
                    uint8_t local_row = (uint8_t)(yy - bbox_y);
                    uint64_t row_mask = glyph_entry->row_masks[local_row] & x_mask;
                    if (row_mask == 0) {
                        continue;
                    }
                    size_t local_row_off = (size_t)(yy - start_y) * chunk_width;
                    while (row_mask != 0) {
                        uint8_t bit = (uint8_t)__builtin_ctzll(row_mask);
                        size_t local_col = (size_t)(bbox_x + bit - present_x);
                        chunk_buffer[local_row_off + local_col] = cmd->color_swapped;
                        row_mask &= row_mask - 1;
                    }
                }
                continue;
            }
        }

        ret = esp_lcd_panel_draw_bitmap(
            impl->panel_handle,
            present_x,
            start_y,
            present_x_end,
            start_y + (mp_int_t)rows_this_chunk,
            chunk_buffer);

        if (ret != ESP_OK) {
            if (pending_id < 2) {
                impl->dma_buffer_in_use[pending_id] = false;
            } else if (pending_id == RM690B0_PENDING_BUFFER_ALLOC) {
                impl->dma_alloc_buffer_in_use = false;
            }
            break;
        }

        rm690b0_dma_pending_push(&impl->dma_pending, pending_id);
        impl->dma_inflight++;
        if (!use_static_buffers) {
            rm690b0_wait_for_dma_completion(impl);
        }
    }

    // With static DMA buffers we can return while the tail of queued transfers
    // is still in flight. This allows Python-side frame preparation to overlap
    // with panel DMA. We still fully drain on error or when using a temporary
    // allocated DMA buffer (must not be freed while in flight).
    bool must_drain_dma = (ret != ESP_OK) || !use_static_buffers;
    if (must_drain_dma) {
        rm690b0_wait_for_all_dma(impl);
    }

    if (!use_static_buffers && alloc_buffer != NULL) {
        while (impl->dma_alloc_buffer_in_use) {
            rm690b0_wait_for_dma_completion(impl);
        }
    }

    if (alloc_buffer != NULL) {
        heap_caps_free(alloc_buffer);
        impl->dma_alloc_buffer_ptr = NULL;
    }

    if (ret == ESP_OK) {
        dl->telemetry_present_count++;
        if (full_present) {
            dl->telemetry_present_full++;
        } else {
            dl->telemetry_present_partial++;
        }
        dl->dirty_valid = false;
        dl->force_full_present = false;
        if (!keep_commands) {
            rm690b0_dl_reset_frame(impl, true);
            dl->clear_color_valid = false;
        }
    }

    return ret;
}
