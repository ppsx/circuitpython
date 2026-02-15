// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

// Core: lifecycle (construct/deinit/init_display), DMA, flush, properties,
// swap_buffers.  Drawing, text, and image functions live in separate files.

#include "rm690b0_internal.h"

// ============================================================================
// Global definitions (extern'd in rm690b0_internal.h)
// ============================================================================

const char *TAG = "rm690b0";
portMUX_TYPE rm690b0_spinlock = portMUX_INITIALIZER_UNLOCKED;
rm690b0_rm690b0_obj_t *rm690b0_singleton = NULL;

// ============================================================================
// ISR callback
// ============================================================================

bool IRAM_ATTR rm690b0_on_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
    rm690b0_impl_t *impl = (rm690b0_impl_t *)user_ctx;
    BaseType_t high_task_awoken = pdFALSE;
    xSemaphoreGiveFromISR(impl->transfer_done_sem, &high_task_awoken);
    return high_task_awoken == pdTRUE;
}

// ============================================================================
// Cross-file helpers (non-inline, declared in rm690b0_internal.h)
// ============================================================================

void rm690b0_span_update(rm690b0_span_accumulator_t *acc, mp_int_t row_y, mp_int_t x_val) {
    mp_int_t idx = row_y - acc->top;
    if (idx < 0 || idx >= acc->row_count) {
        return;
    }
    if (x_val < acc->left[idx]) {
        acc->left[idx] = (int16_t)x_val;
    }
    if (x_val > acc->right[idx]) {
        acc->right[idx] = (int16_t)x_val;
    }
}

int16_t *rm690b0_acquire_span_cache(rm690b0_impl_t *impl, size_t needed_rows) {
    if (impl == NULL || needed_rows == 0) {
        return NULL;
    }

    if (impl->circle_span_capacity < needed_rows) {
        size_t new_capacity = needed_rows;
        size_t total_entries = new_capacity * 2;
        int16_t *new_cache = (int16_t *)heap_caps_realloc(
            impl->circle_span_cache,
            total_entries * sizeof(int16_t),
            MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (new_cache == NULL) {
            return NULL;
        }
        impl->circle_span_cache = new_cache;
        impl->circle_span_capacity = new_capacity;
    }
    return impl->circle_span_cache;
}

// ============================================================================
// expand_even_region — 2-pixel alignment for RM690B0 hardware
// ============================================================================

bool expand_even_region(mp_int_t *x, mp_int_t *y, mp_int_t *width, mp_int_t *height) {
    mp_int_t start_x = *x;
    mp_int_t start_y = *y;
    mp_int_t end_x = start_x + *width;
    mp_int_t end_y = start_y + *height;

    if (start_x < 0) {
        start_x = 0;
    }
    if (start_y < 0) {
        start_y = 0;
    }
    if (end_x > RM690B0_PANEL_WIDTH) {
        end_x = RM690B0_PANEL_WIDTH;
    }
    if (end_y > RM690B0_PANEL_HEIGHT) {
        end_y = RM690B0_PANEL_HEIGHT;
    }

    if (start_x >= end_x || start_y >= end_y) {
        return false;
    }

    if (start_x & 1) {
        if (start_x > 0) {
            start_x -= 1;
        } else if (end_x < RM690B0_PANEL_WIDTH) {
            end_x += 1;
        }
    }
    if (end_x & 1) {
        if (end_x < RM690B0_PANEL_WIDTH) {
            end_x += 1;
        } else if (start_x > 0) {
            start_x -= 1;
        }
    }
    if (((end_x - start_x) & 1) != 0) {
        if (end_x < RM690B0_PANEL_WIDTH) {
            end_x += 1;
        } else if (start_x > 0) {
            start_x -= 1;
        }
    }

    if (start_y & 1) {
        if (start_y > 0) {
            start_y -= 1;
        } else if (end_y < RM690B0_PANEL_HEIGHT) {
            end_y += 1;
        }
    }
    if (end_y & 1) {
        if (end_y < RM690B0_PANEL_HEIGHT) {
            end_y += 1;
        } else if (start_y > 0) {
            start_y -= 1;
        }
    }
    if (((end_y - start_y) & 1) != 0) {
        if (end_y < RM690B0_PANEL_HEIGHT) {
            end_y += 1;
        } else if (start_y > 0) {
            start_y -= 1;
        }
    }

    if ((end_y - start_y) < 2) {
        if (end_y < RM690B0_PANEL_HEIGHT) {
            end_y = start_y + 2;
        } else if (start_y >= 1) {
            start_y = end_y - 2;
        }
    }

    if (start_x >= end_x || start_y >= end_y) {
        return false;
    }

    *x = start_x;
    *y = start_y;
    *width = end_x - start_x;
    *height = end_y - start_y;
    return true;
}

// ============================================================================
// LCD init commands
// ============================================================================

static const rm690b0_lcd_init_cmd_t lcd_init_cmds[] = {
    {0xFE, (uint8_t []) {0x20}, 1, 0},
    {0x26, (uint8_t []) {0x0A}, 1, 0},
    {0x24, (uint8_t []) {0x80}, 1, 0},
    {0xFE, (uint8_t []) {0x13}, 1, 0},
    {0xEB, (uint8_t []) {0x0E}, 1, 0},
    {0xFE, (uint8_t []) {0x00}, 1, 0},
    {0x3A, (uint8_t []) {0x55}, 1, 0},
    {0xC2, (uint8_t []) {0x00}, 1, 10},
    {0x35, (uint8_t []) {0x00}, 0, 0},
    {0x51, (uint8_t []) {0x00}, 1, 10},
    {0x11, (uint8_t []) {0x00}, 0, 80},
    {0x2A, (uint8_t []) {0x00, 0x10, 0x01, 0xD1}, 4, 0},
    {0x2B, (uint8_t []) {0x00, 0x00, 0x02, 0x57}, 4, 0},
    {0x29, (uint8_t []) {0x00}, 0, 10},
    {0x36, (uint8_t []) {0x30}, 1, 10}, // MADCTL for Landscape with RGB color order
    {0x51, (uint8_t []) {0xFF}, 1, 0},
};

// ============================================================================
// tx_param helper
// ============================================================================

static esp_err_t rm690b0_tx_param(const rm690b0_impl_t *impl, uint8_t cmd, const void *param, size_t param_size) {
    if (impl == NULL || impl->io_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint32_t packed_cmd = ((uint32_t)RM690B0_OPCODE_WRITE_CMD << 24) | ((uint32_t)cmd << 8);
    return esp_lcd_panel_io_tx_param(impl->io_handle, packed_cmd, param, param_size);
}

// ============================================================================
// Dirty region tracking
// ============================================================================

void mark_dirty_region(rm690b0_impl_t *impl, mp_int_t x, mp_int_t y, mp_int_t w, mp_int_t h) {
    if (w <= 0 || h <= 0) {
        return;
    }

    if (!impl->dirty_merged_valid) {
        impl->dirty_merged_x = x;
        impl->dirty_merged_y = y;
        impl->dirty_merged_w = w;
        impl->dirty_merged_h = h;
        impl->dirty_merged_valid = true;
    } else {
        mp_int_t x2 = impl->dirty_merged_x + impl->dirty_merged_w;
        mp_int_t y2 = impl->dirty_merged_y + impl->dirty_merged_h;
        impl->dirty_merged_x = (x < impl->dirty_merged_x) ? x : impl->dirty_merged_x;
        impl->dirty_merged_y = (y < impl->dirty_merged_y) ? y : impl->dirty_merged_y;
        x2 = (x + w > x2) ? x + w : x2;
        y2 = (y + h > y2) ? y + h : y2;
        impl->dirty_merged_w = x2 - impl->dirty_merged_x;
        impl->dirty_merged_h = y2 - impl->dirty_merged_y;
    }

    if (impl->dirty_count < RM690B0_MAX_DIRTY_RECTS) {
        impl->dirty_rects[impl->dirty_count++] = (rm690b0_dirty_rect_t){x, y, w, h};
        return;
    }

    size_t merge_idx = 0;
    mp_int_t min_dist = INT_MAX;
    mp_int_t cx = x + w / 2;
    mp_int_t cy = y + h / 2;
    for (size_t i = 0; i < RM690B0_MAX_DIRTY_RECTS; i++) {
        rm690b0_dirty_rect_t *r = &impl->dirty_rects[i];
        mp_int_t dx = cx - (r->x + r->w / 2);
        mp_int_t dy = cy - (r->y + r->h / 2);
        mp_int_t dist = dx * dx + dy * dy;
        if (dist < min_dist) {
            min_dist = dist;
            merge_idx = i;
        }
    }
    rm690b0_dirty_rect_t *t = &impl->dirty_rects[merge_idx];
    mp_int_t tx2 = t->x + t->w;
    mp_int_t ty2 = t->y + t->h;
    t->x = (x < t->x) ? x : t->x;
    t->y = (y < t->y) ? y : t->y;
    t->w = ((x + w > tx2) ? x + w : tx2) - t->x;
    t->h = ((y + h > ty2) ? y + h : ty2) - t->y;
}

// ============================================================================
// Flush region to display
// ============================================================================

esp_err_t rm690b0_flush_region(rm690b0_rm690b0_obj_t *self,
    mp_int_t x, mp_int_t y, mp_int_t width, mp_int_t height) {

    if (width <= 0 || height <= 0) {
        return ESP_OK;
    }

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || impl->panel_handle == NULL || impl->framebuffer == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    mp_int_t fx = x;
    mp_int_t fy = y;
    mp_int_t fw = width;
    mp_int_t fh = height;

    if (!expand_even_region(&fx, &fy, &fw, &fh)) {
        return ESP_OK;
    }

    size_t fb_stride = RM690B0_PANEL_WIDTH;
    size_t fw_sz = (size_t)fw;
    size_t fh_sz = (size_t)fh;

    // Check if direct DMA from framebuffer is possible (full-width flush).
    // Direct DMA avoids copying to chunk buffers entirely.
    bool direct_dma = (fx == 0 && fw == RM690B0_PANEL_WIDTH && x == 0 && width == RM690B0_PANEL_WIDTH);

    size_t available_pixels;
    if (direct_dma) {
        // Direct DMA: limited only by SPI bus max_transfer_sz, not chunk buffers.
        available_pixels = RM690B0_MAX_CHUNK_PIXELS;
    } else {
        available_pixels = RM690B0_MAX_CHUNK_PIXELS;
        if (impl->chunk_buffers[0] != NULL) {
            available_pixels = impl->chunk_buffer_pixels;
        }
    }

    size_t max_chunk_height = available_pixels / fw_sz;
    if (max_chunk_height == 0) {
        max_chunk_height = 1;
    }

    size_t chunk_height = fh_sz;
    if (chunk_height > max_chunk_height) {
        chunk_height = max_chunk_height;
    }
    if (chunk_height == 0) {
        chunk_height = 1;
    }
    if (chunk_height & 1) {
        if (chunk_height < fh_sz) {
            chunk_height += 1;
        } else if (chunk_height > 1) {
            chunk_height -= 1;
        }
    }

    size_t max_chunk_pixels = fw_sz * chunk_height;

    bool use_static_buffers = false;
    uint16_t *alloc_buffer = NULL;
    if (!direct_dma) {
        use_static_buffers = (impl->chunk_buffers[0] != NULL &&
            impl->chunk_buffers[1] != NULL &&
            impl->chunk_buffer_pixels >= max_chunk_pixels);

        if (!use_static_buffers) {
            alloc_buffer = heap_caps_malloc(max_chunk_pixels * sizeof(uint16_t), MALLOC_CAP_DMA);
            if (alloc_buffer == NULL) {
                return ESP_ERR_NO_MEM;
            }
        }
    }

    esp_err_t ret = ESP_OK;
    uint16_t *framebuffer = impl->framebuffer;
    int buf_idx = 0;

    for (mp_int_t start_y = fy; start_y < fy + fh && ret == ESP_OK; start_y += (mp_int_t)chunk_height) {
        size_t rows_this_chunk = (size_t)((fy + fh) - start_y);
        if (rows_this_chunk > chunk_height) {
            rows_this_chunk = chunk_height;
        }

        rm690b0_wait_for_dma_slot(impl);

        const void *dma_buffer = NULL;
        uint8_t pending_id = RM690B0_PENDING_BUFFER_FRAMEBUFFER;

        if (direct_dma) {
            dma_buffer = framebuffer + (size_t)start_y * fb_stride;
        } else {
            uint16_t *current_buffer = NULL;
            if (use_static_buffers) {
                uint8_t current_idx = (uint8_t)buf_idx;
                while (impl->dma_buffer_in_use[current_idx]) {
                    rm690b0_wait_for_dma_completion(impl);
                }
                current_buffer = impl->chunk_buffers[current_idx];
                impl->dma_buffer_in_use[current_idx] = true;
                pending_id = current_idx;
                buf_idx = (buf_idx + 1) % 2;
            } else {
                while (impl->dma_alloc_buffer_in_use) {
                    rm690b0_wait_for_dma_completion(impl);
                }
                current_buffer = alloc_buffer;
                impl->dma_alloc_buffer_in_use = true;
                pending_id = RM690B0_PENDING_BUFFER_ALLOC;
            }

            dma_buffer = current_buffer;

            if (x == 0 && width == RM690B0_PANEL_WIDTH) {
                size_t bytes_per_row = fb_stride * sizeof(uint16_t);
                size_t chunk_bytes = rows_this_chunk * bytes_per_row;
                const uint16_t *src = framebuffer + (size_t)start_y * fb_stride;
                memcpy(current_buffer, src, chunk_bytes);
            } else {
                size_t dest_index = 0;
                for (size_t row = 0; row < rows_this_chunk; row++) {
                    mp_int_t phys_row = start_y + (mp_int_t)row;
                    mp_int_t src_row = phys_row;
                    if (src_row < 0) {
                        src_row = 0;
                    } else if (src_row >= RM690B0_PANEL_HEIGHT) {
                        src_row = RM690B0_PANEL_HEIGHT - 1;
                    }

                    const uint16_t *row_base = framebuffer + (size_t)src_row * fb_stride;
                    mp_int_t dest_col = 0;
                    mp_int_t flush_end_x = fx + fw;

                    mp_int_t src_left = x;
                    if (src_left < 0) {
                        src_left = 0;
                    }
                    mp_int_t src_right = x + width - 1;
                    if (src_right >= RM690B0_PANEL_WIDTH) {
                        src_right = RM690B0_PANEL_WIDTH - 1;
                    }

                    for (mp_int_t phys_col = fx; phys_col < src_left; phys_col++) {
                        mp_int_t safe_col = (phys_col < 0) ? 0 : ((phys_col >= RM690B0_PANEL_WIDTH) ? RM690B0_PANEL_WIDTH - 1 : phys_col);
                        current_buffer[dest_index + dest_col++] = row_base[safe_col];
                    }

                    if (src_left <= src_right) {
                        size_t middle_len = (size_t)(src_right - src_left + 1);
                        memcpy(&current_buffer[dest_index + dest_col], &row_base[src_left], middle_len * sizeof(uint16_t));
                        dest_col += (mp_int_t)middle_len;
                    }

                    for (mp_int_t phys_col = src_right + 1; phys_col < flush_end_x; phys_col++) {
                        mp_int_t safe_col = (phys_col < 0) ? 0 : ((phys_col >= RM690B0_PANEL_WIDTH) ? RM690B0_PANEL_WIDTH - 1 : phys_col);
                        current_buffer[dest_index + dest_col++] = row_base[safe_col];
                    }
                    dest_index += fw_sz;
                }
            }
        }

        ret = esp_lcd_panel_draw_bitmap(
            impl->panel_handle,
            fx,
            start_y,
            fx + fw,
            start_y + (mp_int_t)rows_this_chunk,
            dma_buffer);

        if (ret != ESP_OK) {
            if (!direct_dma) {
                if (pending_id < 2) {
                    impl->dma_buffer_in_use[pending_id] = false;
                } else if (pending_id == RM690B0_PENDING_BUFFER_ALLOC) {
                    impl->dma_alloc_buffer_in_use = false;
                }
            }
            break;
        }

        rm690b0_dma_pending_push(&impl->dma_pending, pending_id);
        impl->dma_inflight++;
        if (!use_static_buffers && !direct_dma) {
            rm690b0_wait_for_dma_completion(impl);
        }
    }

    if (!use_static_buffers && alloc_buffer != NULL) {
        while (impl->dma_alloc_buffer_in_use) {
            rm690b0_wait_for_dma_completion(impl);
        }
    }

    if (alloc_buffer != NULL) {
        heap_caps_free(alloc_buffer);
    }

    return ret;
}

// ============================================================================
// Framebuffer fill rect helper
// ============================================================================

// ============================================================================
// Direct fill helpers (bypass framebuffer for single-buffer mode)
// ============================================================================

void rm690b0_fill_color_direct(rm690b0_rm690b0_obj_t *self, uint16_t color) {
    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || impl->panel_handle == NULL || impl->transfer_done_sem == NULL) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Invalid display handle"));
        return;
    }

    uint16_t swapped = RGB565_SWAP_GB(color);
    size_t dma_lines = RM690B0_MAX_CHUNK_ROWS;
    if (dma_lines > RM690B0_PANEL_HEIGHT) {
        dma_lines = RM690B0_PANEL_HEIGHT;
    }
    const size_t line_pixels = RM690B0_PANEL_WIDTH;

    uint16_t *dma_buffer = NULL;
    bool allocated = false;

    if (impl->chunk_buffers[0] != NULL && impl->chunk_buffer_pixels > 0) {
        size_t max_lines = impl->chunk_buffer_pixels / line_pixels;
        if (max_lines > 0 && max_lines < dma_lines) {
            dma_lines = max_lines;
        }
        dma_buffer = impl->chunk_buffers[0];
    } else {
        const size_t dma_pixels = line_pixels * dma_lines;
        const size_t dma_bytes = dma_pixels * sizeof(uint16_t);
        dma_buffer = heap_caps_malloc(dma_bytes, MALLOC_CAP_DMA);
        if (dma_buffer == NULL) {
            mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("fill_color: unable to allocate DMA buffer"));
            return;
        }
        allocated = true;
    }

    const size_t dma_pixels = line_pixels * dma_lines;
    for (size_t i = 0; i < dma_pixels; i++) {
        dma_buffer[i] = swapped;
    }

    size_t rows_remaining = RM690B0_PANEL_HEIGHT;
    mp_int_t current_y = 0;
    esp_err_t ret = ESP_OK;

    while (rows_remaining > 0 && ret == ESP_OK) {
        size_t rows_this_pass = rows_remaining > dma_lines ? dma_lines : rows_remaining;

        rm690b0_wait_for_dma_slot(impl);

        ret = esp_lcd_panel_draw_bitmap(
            impl->panel_handle,
            0,
            current_y,
            RM690B0_PANEL_WIDTH,
            current_y + (mp_int_t)rows_this_pass,
            dma_buffer);

        if (ret != ESP_OK) {
            break;
        }

        rm690b0_dma_pending_push(&impl->dma_pending, RM690B0_PENDING_BUFFER_TEMP);
        impl->dma_inflight++;
        current_y += (mp_int_t)rows_this_pass;
        rows_remaining -= rows_this_pass;
    }

    rm690b0_wait_for_all_dma(impl);
    if (ret == ESP_OK && impl->framebuffer != NULL) {
        rm690b0_fill_rect_framebuffer(impl, 0, 0,
            RM690B0_PANEL_WIDTH, RM690B0_PANEL_HEIGHT, swapped);
        impl->dirty_count = 0;
        impl->dirty_merged_valid = false;
    }

    if (allocated) {
        heap_caps_free(dma_buffer);
    }

    if (ret != ESP_OK) {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("fill_color failed: %s"), esp_err_to_name(ret));
    }
}

esp_err_t rm690b0_fill_rect_direct_fullwidth(rm690b0_rm690b0_obj_t *self,
    mp_int_t start_y, mp_int_t rows, uint16_t swapped_color) {

    if (rows <= 0) {
        return ESP_OK;
    }

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || impl->panel_handle == NULL || impl->transfer_done_sem == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    size_t dma_lines = RM690B0_MAX_CHUNK_ROWS;
    if (dma_lines > (size_t)rows) {
        dma_lines = rows;
    }
    const size_t line_pixels = RM690B0_PANEL_WIDTH;

    uint16_t *dma_buffer = NULL;
    bool allocated = false;

    if (impl->chunk_buffers[0] != NULL && impl->chunk_buffer_pixels > 0) {
        size_t max_lines = impl->chunk_buffer_pixels / line_pixels;
        if (max_lines > 0 && max_lines < dma_lines) {
            dma_lines = max_lines;
        }
        dma_buffer = impl->chunk_buffers[0];
    } else {
        const size_t dma_pixels = line_pixels * dma_lines;
        const size_t dma_bytes = dma_pixels * sizeof(uint16_t);
        dma_buffer = heap_caps_malloc(dma_bytes, MALLOC_CAP_DMA);
        if (dma_buffer == NULL) {
            return ESP_ERR_NO_MEM;
        }
        allocated = true;
    }

    const size_t dma_pixels = line_pixels * dma_lines;
    for (size_t i = 0; i < dma_pixels; i++) {
        dma_buffer[i] = swapped_color;
    }

    size_t rows_remaining = (size_t)rows;
    mp_int_t current_y = start_y;
    esp_err_t ret = ESP_OK;

    while (rows_remaining > 0 && ret == ESP_OK) {
        size_t rows_this_pass = rows_remaining > dma_lines ? dma_lines : rows_remaining;

        rm690b0_wait_for_dma_slot(impl);

        ret = esp_lcd_panel_draw_bitmap(
            impl->panel_handle,
            0,
            current_y,
            RM690B0_PANEL_WIDTH,
            current_y + (mp_int_t)rows_this_pass,
            dma_buffer);

        if (ret != ESP_OK) {
            break;
        }

        rm690b0_dma_pending_push(&impl->dma_pending, RM690B0_PENDING_BUFFER_TEMP);
        impl->dma_inflight++;
        current_y += (mp_int_t)rows_this_pass;
        rows_remaining -= rows_this_pass;
    }

    rm690b0_wait_for_all_dma(impl);

    if (allocated) {
        heap_caps_free(dma_buffer);
    }

    if (ret == ESP_OK && impl->framebuffer != NULL) {
        rm690b0_fill_rect_framebuffer(impl, 0, start_y, RM690B0_PANEL_WIDTH, rows, swapped_color);
    }

    return ret;
}

// ============================================================================
// Lifecycle: construct, deinit, init_display
// ============================================================================

void common_hal_rm690b0_rm690b0_construct(rm690b0_rm690b0_obj_t *self) {
    self->initialized = false;
    self->width = RM690B0_PANEL_WIDTH;
    self->height = RM690B0_PANEL_HEIGHT;
    self->rotation = 0;
    self->brightness_raw = 0xFF;
    self->font_id = RM690B0_FONT_8x8_MONO;
    self->buffer_mode = RM690B0_BUFFER_DOUBLE;
    self->impl = m_malloc(sizeof(rm690b0_impl_t));
    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    impl->io_handle = NULL;
    impl->panel_handle = NULL;
    impl->bus_initialized = false;
    impl->chunk_buffers[0] = NULL;
    impl->chunk_buffers[1] = NULL;
    impl->chunk_buffer_pixels = 0;
    impl->framebuffer = NULL;
    impl->framebuffer_pixels = 0;
    impl->framebuffer_front = NULL;
    impl->double_buffered = false;
    impl->circle_span_cache = NULL;
    impl->circle_span_capacity = 0;

    impl->dirty_count = 0;
    impl->dirty_merged_valid = false;

    impl->transfer_done_sem = xSemaphoreCreateCounting(RM690B0_PANEL_IO_QUEUE_DEPTH, 0);
    if (impl->transfer_done_sem == NULL) {
        m_free(impl);
        self->impl = NULL;
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create DMA semaphore"));
    }
    impl->dma_buffer_in_use[0] = false;
    impl->dma_buffer_in_use[1] = false;
    impl->dma_alloc_buffer_in_use = false;
    impl->dma_inflight = 0;
    rm690b0_dma_pending_init(&impl->dma_pending);

    rm690b0_singleton = self;

    ESP_LOGI(TAG, "RM690B0 module constructed");
}

void common_hal_rm690b0_rm690b0_deinit(rm690b0_rm690b0_obj_t *self) {
    if (!self) {
        ESP_LOGW(TAG, "deinit called with NULL self pointer");
        return;
    }

    if (!self->initialized) {
        ESP_LOGI(TAG, "deinit called on already deinitialized instance");
        return;
    }

    ESP_LOGI(TAG, "Starting RM690B0 deinit");

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (!impl) {
        ESP_LOGW(TAG, "deinit: impl pointer is NULL, nothing to free");
        self->initialized = false;
        if (rm690b0_singleton == self) {
            rm690b0_singleton = NULL;
        }
        return;
    }

    if (rm690b0_singleton == self) {
        rm690b0_singleton = NULL;
    }

    self->initialized = false;

    if (impl->panel_handle != NULL && impl->framebuffer != NULL) {
        ESP_LOGI(TAG, "Clearing display to black before deinit");

        size_t framebuffer_pixels = (size_t)RM690B0_PANEL_WIDTH * RM690B0_PANEL_HEIGHT;
        memset(impl->framebuffer, 0, framebuffer_pixels * sizeof(uint16_t));

        esp_err_t ret = rm690b0_flush_region(self, 0, 0, RM690B0_PANEL_WIDTH, RM690B0_PANEL_HEIGHT);
        if (ret == ESP_OK) {
            rm690b0_wait_for_all_dma(impl);
            vTaskDelay(pdMS_TO_TICKS(10));
            ESP_LOGI(TAG, "Screen cleared successfully");
        } else {
            ESP_LOGW(TAG, "Failed to clear screen before deinit (non-critical)");
        }
    }

    rm690b0_wait_for_all_dma(impl);

    if (impl->framebuffer_front) {
        ESP_LOGI(TAG, "Freeing front framebuffer");
        heap_caps_free(impl->framebuffer_front);
        impl->framebuffer_front = NULL;
        impl->double_buffered = false;
    }

    if (impl->framebuffer) {
        ESP_LOGI(TAG, "Freeing back framebuffer");
        heap_caps_free(impl->framebuffer);
        impl->framebuffer = NULL;
        impl->framebuffer_pixels = 0;
    }

    if (impl->chunk_buffers[0]) {
        ESP_LOGI(TAG, "Freeing chunk buffer A");
        heap_caps_free(impl->chunk_buffers[0]);
        impl->chunk_buffers[0] = NULL;
    }
    if (impl->chunk_buffers[1]) {
        ESP_LOGI(TAG, "Freeing chunk buffer B");
        heap_caps_free(impl->chunk_buffers[1]);
        impl->chunk_buffers[1] = NULL;
    }
    impl->chunk_buffer_pixels = 0;

    if (impl->circle_span_cache) {
        ESP_LOGI(TAG, "Freeing cached circle spans");
        heap_caps_free(impl->circle_span_cache);
        impl->circle_span_cache = NULL;
        impl->circle_span_capacity = 0;
    }

    if (impl->transfer_done_sem) {
        vSemaphoreDelete(impl->transfer_done_sem);
        impl->transfer_done_sem = NULL;
    }

    if (LCD_PWR_PIN != GPIO_NUM_NC) {
        ESP_LOGI(TAG, "Turning off display power");
        int off_level = LCD_PWR_ON_LEVEL ? 0 : 1;
        gpio_set_level(LCD_PWR_PIN, off_level);
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    if (impl->panel_handle != NULL) {
        ESP_LOGI(TAG, "Deleting LCD panel handle");
        esp_err_t ret = esp_lcd_panel_del(impl->panel_handle);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Failed to delete LCD panel: %s", esp_err_to_name(ret));
        }
        impl->panel_handle = NULL;
    }

    if (impl->io_handle != NULL) {
        ESP_LOGI(TAG, "Deleting LCD IO handle");
        esp_err_t ret = esp_lcd_panel_io_del(impl->io_handle);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Failed to delete LCD IO: %s", esp_err_to_name(ret));
        }
        impl->io_handle = NULL;
    }

    if (impl->bus_initialized) {
        ESP_LOGI(TAG, "Freeing SPI bus");
        esp_err_t ret = spi_bus_free(SPI2_HOST);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Failed to free SPI bus: %s (may be in use)", esp_err_to_name(ret));
        }
        impl->bus_initialized = false;
    }

    self->impl = NULL;

    ESP_LOGI(TAG, "RM690B0 deinit complete - all resources freed");
}

void common_hal_rm690b0_rm690b0_deinit_all(void) {
    if (rm690b0_singleton != NULL) {
        ESP_LOGI(TAG, "Static deinit: cleaning up singleton instance");
        common_hal_rm690b0_rm690b0_deinit(rm690b0_singleton);
        rm690b0_singleton = NULL;
        ESP_LOGI(TAG, "Static deinit: singleton cleaned up");
    } else {
        ESP_LOGI(TAG, "Static deinit: no active instance to clean up");
    }
}

esp_lcd_panel_handle_t rm690b0_get_panel_handle(void) {
    if (rm690b0_singleton == NULL) {
        ESP_LOGW(TAG, "rm690b0_get_panel_handle: no active display instance");
        return NULL;
    }

    if (!rm690b0_singleton->initialized) {
        ESP_LOGW(TAG, "rm690b0_get_panel_handle: display not initialized");
        return NULL;
    }

    rm690b0_impl_t *impl = (rm690b0_impl_t *)rm690b0_singleton->impl;
    if (impl == NULL) {
        ESP_LOGE(TAG, "rm690b0_get_panel_handle: impl is NULL");
        return NULL;
    }

    return impl->panel_handle;
}

void common_hal_rm690b0_rm690b0_init_display(rm690b0_rm690b0_obj_t *self) {
    if (self->initialized) {
        ESP_LOGW(TAG, "Display already initialized");
        return;
    }

    ESP_LOGI(TAG, "Initializing RM690B0 display (%s mode)", LCD_USE_QSPI ? "QSPI" : "SPI");

    if (LCD_PWR_PIN != GPIO_NUM_NC) {
        gpio_config_t io_conf = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = (1ULL << LCD_PWR_PIN),
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&io_conf);
        gpio_set_level(LCD_PWR_PIN, LCD_PWR_ON_LEVEL);
        ESP_LOGI(TAG, "Display power enabled on GPIO%d", LCD_PWR_PIN);
    } else {
        ESP_LOGI(TAG, "No display power control pin configured; assuming panel is already powered");
    }

    vTaskDelay(pdMS_TO_TICKS(200));

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    esp_err_t ret = ESP_OK;

    if (!impl->bus_initialized) {
        size_t max_transfer_bytes = RM690B0_MAX_CHUNK_PIXELS * sizeof(uint16_t);
        const spi_bus_config_t buscfg = LCD_USE_QSPI ?
            RM690B0_PANEL_BUS_QSPI_CONFIG(LCD_SCK_PIN, LCD_D0_PIN, LCD_D1_PIN, LCD_D2_PIN, LCD_D3_PIN, max_transfer_bytes) :
            RM690B0_PANEL_BUS_SPI_CONFIG(LCD_SCK_PIN, LCD_D0_PIN, max_transfer_bytes);
        ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
        if (ret != ESP_OK) {
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to initialize SPI bus: %s"), esp_err_to_name(ret));
            return;
        }
        impl->bus_initialized = true;
        ESP_LOGI(TAG, "%s bus initialized", LCD_USE_QSPI ? "QSPI" : "SPI");
    }

    if (LCD_USE_QSPI) {
        const esp_lcd_panel_io_spi_config_t io_config = RM690B0_PANEL_IO_QSPI_CONFIG(LCD_CS_PIN, rm690b0_on_color_trans_done, impl, LCD_PIXEL_CLOCK_HZ);
        ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &impl->io_handle);
    } else {
        const esp_lcd_panel_io_spi_config_t io_config = RM690B0_PANEL_IO_SPI_CONFIG(LCD_CS_PIN, -1, rm690b0_on_color_trans_done, impl, LCD_PIXEL_CLOCK_HZ);
        ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &impl->io_handle);
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create panel I/O: %s", esp_err_to_name(ret));
        spi_bus_free(SPI2_HOST);
        impl->bus_initialized = false;
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to create panel I/O"));
        return;
    }
    ESP_LOGI(TAG, "Panel I/O created");

    rm690b0_vendor_config_t vendor_config = {
        .init_cmds = lcd_init_cmds,
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(rm690b0_lcd_init_cmd_t),
        .flags = {
            .use_qspi_interface = LCD_USE_QSPI,
        },
    };

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST_PIN,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = LCD_BIT_PER_PIXEL,
        .flags = {
            .reset_active_high = false,
        },
        .vendor_config = &vendor_config,
    };

    ret = esp_lcd_new_panel_rm690b0(impl->io_handle, &panel_config, &impl->panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create RM690B0 panel: %s", esp_err_to_name(ret));
        esp_lcd_panel_io_del(impl->io_handle);
        impl->io_handle = NULL;
        spi_bus_free(SPI2_HOST);
        impl->bus_initialized = false;
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to create RM690B0 panel"));
        return;
    }
    ESP_LOGI(TAG, "RM690B0 panel created");

    ret = esp_lcd_panel_reset(impl->panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reset panel: %s", esp_err_to_name(ret));
        goto cleanup;
    }
    ESP_LOGI(TAG, "Panel reset complete");

    ret = esp_lcd_panel_init(impl->panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize panel: %s", esp_err_to_name(ret));
        goto cleanup;
    }
    ESP_LOGI(TAG, "Panel initialization complete");

    esp_lcd_panel_set_gap(impl->panel_handle, RM690B0_X_GAP, RM690B0_Y_GAP);
    ESP_LOGI(TAG, "Display gap set to (0, 16)");

    ret = esp_lcd_panel_disp_on_off(impl->panel_handle, true);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to turn on display: %s", esp_err_to_name(ret));
        goto cleanup;
    }
    ESP_LOGI(TAG, "Display turned on");

    if (impl->chunk_buffers[0] == NULL) {
        size_t current_chunk_rows = RM690B0_MAX_CHUNK_ROWS;

        while (current_chunk_rows >= 2) {
            size_t buf_pixels = (size_t)RM690B0_PANEL_WIDTH * current_chunk_rows;
            size_t buf_size = buf_pixels * sizeof(uint16_t);

            impl->chunk_buffers[0] = heap_caps_malloc(buf_size, MALLOC_CAP_DMA);
            impl->chunk_buffers[1] = heap_caps_malloc(buf_size, MALLOC_CAP_DMA);

            if (impl->chunk_buffers[0] != NULL && impl->chunk_buffers[1] != NULL) {
                impl->chunk_buffer_pixels = buf_pixels;
                ESP_LOGI(TAG, "Allocated dual DMA chunk buffers (%zu pixels each, %zu rows)",
                    impl->chunk_buffer_pixels, current_chunk_rows);
                break;
            }

            if (impl->chunk_buffers[0]) {
                heap_caps_free(impl->chunk_buffers[0]);
            }
            if (impl->chunk_buffers[1]) {
                heap_caps_free(impl->chunk_buffers[1]);
            }
            impl->chunk_buffers[0] = NULL;
            impl->chunk_buffers[1] = NULL;

            current_chunk_rows /= 2;
        }

        if (impl->chunk_buffers[0] == NULL) {
            impl->chunk_buffer_pixels = 0;
            ESP_LOGW(TAG, "Unable to allocate dual DMA chunk buffers; operations will use slower allocation per draw");
        }
    }

    size_t framebuffer_pixels = (size_t)RM690B0_PANEL_WIDTH * RM690B0_PANEL_HEIGHT;
    if (impl->framebuffer == NULL) {
        impl->framebuffer = heap_caps_malloc(framebuffer_pixels * sizeof(uint16_t),
            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (impl->framebuffer == NULL) {
            ESP_LOGE(TAG, "Unable to allocate PSRAM framebuffer");
            goto cleanup;
        }
        impl->framebuffer_pixels = framebuffer_pixels;
        ESP_LOGI(TAG, "Allocated back framebuffer (%zu pixels, %zu KB)",
            framebuffer_pixels, (framebuffer_pixels * sizeof(uint16_t)) / 1024);
    }

    for (size_t i = 0; i < framebuffer_pixels; i++) {
        impl->framebuffer[i] = 0x0000;
    }
    esp_err_t clear_ret = rm690b0_flush_region(self, 0, 0, RM690B0_PANEL_WIDTH, RM690B0_PANEL_HEIGHT);
    if (clear_ret != ESP_OK) {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to clear display: %s"), esp_err_to_name(clear_ret));
        return;
    }
    ESP_LOGI(TAG, "Display filled with black");

    if (impl->circle_span_cache == NULL) {
        size_t max_span_rows = RM690B0_PANEL_HEIGHT;
        impl->circle_span_cache = (int16_t *)heap_caps_malloc(
            max_span_rows * 2 * sizeof(int16_t),
            MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (impl->circle_span_cache == NULL) {
            ESP_LOGW(TAG, "Failed to pre-allocate span cache - will use dynamic alloc");
            impl->circle_span_capacity = 0;
        } else {
            impl->circle_span_capacity = max_span_rows;
            ESP_LOGI(TAG, "Pre-allocated span cache: %zu rows (%zu bytes)",
                     max_span_rows, max_span_rows * 2 * sizeof(int16_t));
        }
    }

    self->rotation = 0;
    self->width = RM690B0_PANEL_WIDTH;
    self->height = RM690B0_PANEL_HEIGHT;
    self->brightness_raw = 0xFF;
    self->initialized = true;
    ESP_LOGI(TAG, "RM690B0 display initialization complete");
    return;

cleanup:
    if (impl->chunk_buffers[0]) {
        heap_caps_free(impl->chunk_buffers[0]);
        impl->chunk_buffers[0] = NULL;
    }
    if (impl->chunk_buffers[1]) {
        heap_caps_free(impl->chunk_buffers[1]);
        impl->chunk_buffers[1] = NULL;
    }
    impl->chunk_buffer_pixels = 0;

    if (impl->panel_handle) {
        esp_lcd_panel_del(impl->panel_handle);
        impl->panel_handle = NULL;
    }

    if (impl->io_handle) {
        esp_lcd_panel_io_del(impl->io_handle);
        impl->io_handle = NULL;
    }

    if (impl->bus_initialized) {
        spi_bus_free(SPI2_HOST);
        impl->bus_initialized = false;
    }

    if (ret == ESP_OK) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Display initialization failed: Out of memory"));
    } else {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Display initialization failed: %s"), esp_err_to_name(ret));
    }
}

// ============================================================================
// Properties: rotation, brightness
// ============================================================================

void common_hal_rm690b0_rm690b0_set_rotation(rm690b0_rm690b0_obj_t *self, mp_int_t degrees) {
    CHECK_INITIALIZED();

    mp_int_t normalized = ((degrees % 360) + 360) % 360;
    switch (normalized) {
        case 0:
        case 180:
            self->width = RM690B0_PANEL_WIDTH;
            self->height = RM690B0_PANEL_HEIGHT;
            break;
        case 90:
        case 270:
            self->width = RM690B0_PANEL_HEIGHT;
            self->height = RM690B0_PANEL_WIDTH;
            break;
        default:
            mp_raise_ValueError(MP_ERROR_TEXT("Rotation must be 0, 90, 180, or 270"));
            return;
    }

    self->rotation = normalized;
    ESP_LOGI(TAG, "Display rotation set to %d (logical size %dx%d)", (int)normalized, (int)self->width, (int)self->height);
}

mp_int_t common_hal_rm690b0_rm690b0_get_rotation(const rm690b0_rm690b0_obj_t *self) {
    return self->rotation;
}

void common_hal_rm690b0_rm690b0_set_brightness(rm690b0_rm690b0_obj_t *self, mp_float_t value) {
    CHECK_INITIALIZED();
    if (value < 0.0f || value > 1.0f) {
        mp_raise_ValueError(MP_ERROR_TEXT("Brightness must be between 0.0 and 1.0"));
        return;
    }

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL || impl->io_handle == NULL) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Invalid display handle"));
        return;
    }

    uint8_t brightness;
    if (value <= 0.0f) {
        brightness = 0;
    } else if (value >= 1.0f) {
        brightness = 0xFF;
    } else {
        brightness = (uint8_t)(value * 255.0f + 0.5f);
    }

    if (brightness == self->brightness_raw) {
        return;
    }

    uint8_t page_cmd = 0x00;
    esp_err_t err = rm690b0_tx_param(impl, 0xFE, &page_cmd, 1);
    if (err != ESP_OK) {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to select brightness page: %s"), esp_err_to_name(err));
        return;
    }

    err = rm690b0_tx_param(impl, 0x51, &brightness, 1);
    if (err != ESP_OK) {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to write brightness: %s"), esp_err_to_name(err));
        return;
    }

    uint8_t ctrl_display = (brightness == 0) ? 0x20 : 0x2C;
    err = rm690b0_tx_param(impl, 0x53, &ctrl_display, 1);
    if (err != ESP_OK) {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to latch brightness: %s"), esp_err_to_name(err));
        return;
    }

    self->brightness_raw = brightness;
    ESP_LOGI(TAG, "Set brightness to %u/255 (%.3f)", brightness, (double)value);
}

mp_float_t common_hal_rm690b0_rm690b0_get_brightness(const rm690b0_rm690b0_obj_t *self) {
    uint8_t raw = self->brightness_raw;
    return (mp_float_t)raw / 255.0f;
}

// ============================================================================
// swap_buffers
// ============================================================================

void common_hal_rm690b0_rm690b0_swap_buffers(rm690b0_rm690b0_obj_t *self, bool copy) {
    CHECK_INITIALIZED();

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;

    if (!impl->double_buffered && impl->framebuffer_front == NULL
        && self->buffer_mode != RM690B0_BUFFER_SINGLE) {
        size_t framebuffer_pixels = (size_t)RM690B0_PANEL_WIDTH * RM690B0_PANEL_HEIGHT;
        impl->framebuffer_front = heap_caps_malloc(framebuffer_pixels * sizeof(uint16_t),
            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

        if (impl->framebuffer_front == NULL) {
            ESP_LOGW(TAG, "Unable to allocate front framebuffer - falling back to single-buffered refresh");
        } else {
            impl->double_buffered = true;
            ESP_LOGI(TAG, "Allocated front framebuffer (%zu KB) - double-buffering enabled",
                (framebuffer_pixels * sizeof(uint16_t)) / 1024);

            memcpy(impl->framebuffer_front, impl->framebuffer,
                framebuffer_pixels * sizeof(uint16_t));
        }
    }

    if (!impl->double_buffered || impl->framebuffer_front == NULL) {
        if (impl->dirty_count > 0) {
            size_t individual_area = 0;
            for (size_t i = 0; i < impl->dirty_count; i++) {
                individual_area += (size_t)impl->dirty_rects[i].w * (size_t)impl->dirty_rects[i].h;
            }
            size_t merged_area = (size_t)impl->dirty_merged_w * (size_t)impl->dirty_merged_h;

            esp_err_t ret;
            if (merged_area > individual_area * 3 / 2) {
                for (size_t i = 0; i < impl->dirty_count; i++) {
                    rm690b0_dirty_rect_t *r = &impl->dirty_rects[i];
                    ret = rm690b0_flush_region(self, r->x, r->y, r->w, r->h);
                    (void)ret;
                }
            } else {
                ret = rm690b0_flush_region(self, impl->dirty_merged_x, impl->dirty_merged_y,
                                           impl->dirty_merged_w, impl->dirty_merged_h);
                if (ret != ESP_OK) {
                    mp_raise_msg_varg(&mp_type_RuntimeError,
                        MP_ERROR_TEXT("Failed to refresh display: %s (0x%x)"),
                        esp_err_to_name(ret), ret);
                }
            }
        } else {
            esp_err_t ret = rm690b0_flush_region(self, 0, 0, self->width, self->height);
            if (ret != ESP_OK) {
                mp_raise_msg_varg(&mp_type_RuntimeError,
                    MP_ERROR_TEXT("Failed to refresh display: %s (0x%x)"),
                    esp_err_to_name(ret), ret);
            }
        }
        impl->dirty_count = 0;
        impl->dirty_merged_valid = false;
        return;
    }

    // Wait for any in-flight DMA from the previous swap to complete.
    // With double buffering this is safe: previous DMA reads from the current
    // front buffer while Python drew to the back buffer — no conflict.
    // If draw time > DMA time (typical), this returns instantly.
    rm690b0_wait_for_all_dma(impl);

    mp_int_t flush_x = 0;
    mp_int_t flush_y = 0;
    mp_int_t flush_w = self->width;
    mp_int_t flush_h = self->height;

    if (impl->dirty_count > 0) {
        size_t individual_area = 0;
        for (size_t i = 0; i < impl->dirty_count; i++) {
            individual_area += (size_t)impl->dirty_rects[i].w * (size_t)impl->dirty_rects[i].h;
        }
        size_t merged_area = (size_t)impl->dirty_merged_w * (size_t)impl->dirty_merged_h;

        if (merged_area > individual_area * 3 / 2) {
            for (size_t i = 0; i < impl->dirty_count; i++) {
                rm690b0_dirty_rect_t *r = &impl->dirty_rects[i];
                rm690b0_flush_region(self, r->x, r->y, r->w, r->h);
            }
        } else {
            flush_x = impl->dirty_merged_x;
            flush_y = impl->dirty_merged_y;
            flush_w = impl->dirty_merged_w;
            flush_h = impl->dirty_merged_h;
            rm690b0_flush_region(self, flush_x, flush_y, flush_w, flush_h);
        }
    } else {
        rm690b0_flush_region(self, flush_x, flush_y, flush_w, flush_h);
    }

    // Swap buffer pointers. DMA may still be in flight reading from the
    // back buffer (which becomes the new front). Python draws to the new
    // back buffer (old front) — different memory, no conflict.
    uint16_t *temp = impl->framebuffer_front;
    impl->framebuffer_front = impl->framebuffer;
    impl->framebuffer = temp;

    impl->dirty_count = 0;
    impl->dirty_merged_valid = false;

    if (copy) {
        // Must wait for DMA to finish before copying — the front buffer
        // is still being read by DMA.
        rm690b0_wait_for_all_dma(impl);
        size_t framebuffer_pixels = (size_t)RM690B0_PANEL_WIDTH * RM690B0_PANEL_HEIGHT;
        memcpy(impl->framebuffer, impl->framebuffer_front,
            framebuffer_pixels * sizeof(uint16_t));
    }
}
