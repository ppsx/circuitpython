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
    mp_int_t end_x = rm690b0_add_mp_int_saturating(start_x, *width);
    mp_int_t end_y = rm690b0_add_mp_int_saturating(start_y, *height);

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
        } else {
            return false;
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

// Keep framebuffer allocations/copies alignment-friendly for GDMA-based memcpy.
#define RM690B0_FB_DMA_ALIGNMENT      (16)
#define RM690B0_FB_COPY_TIMEOUT_MS    (1000)
#define RM690B0_FB_COPY_NEAR_FULLWIDTH_MARGIN (16)

static size_t rm690b0_align_up_size(size_t value, size_t alignment) {
    if (alignment == 0) {
        return value;
    }
    size_t mask = alignment - 1;
    return (value + mask) & ~mask;
}

static size_t rm690b0_normalize_chunk_rows(size_t rows, size_t max_rows, size_t frame_rows) {
    if (max_rows == 0) {
        max_rows = 1;
    }
    if (frame_rows == 0) {
        frame_rows = 1;
    }

    if (rows == 0 || rows > max_rows) {
        rows = max_rows;
    }
    if (rows > frame_rows) {
        rows = frame_rows;
    }
    if (rows == 0) {
        rows = 1;
    }

    if ((rows & 1U) != 0U) {
        if (rows < frame_rows && rows < max_rows) {
            rows += 1;
        } else if (rows > 1) {
            rows -= 1;
        }
    }

    if (rows == 0) {
        rows = 1;
    }
    return rows;
}

static size_t rm690b0_choose_flush_chunk_rows(
    const rm690b0_impl_t *impl,
    size_t flush_width,
    size_t flush_height,
    size_t max_chunk_rows,
    bool direct_dma) {

    if (flush_height == 0) {
        return 1;
    }

    size_t rows = flush_height;
    if (rows > max_chunk_rows) {
        rows = max_chunk_rows;
    }

    bool can_overlap_prepare_with_dma = !direct_dma &&
        impl != NULL &&
        impl->double_buffered &&
        impl->chunk_buffers[0] != NULL &&
        impl->chunk_buffers[1] != NULL;

    if (can_overlap_prepare_with_dma && flush_width > 0) {
        size_t target_bytes = (flush_width == RM690B0_PANEL_WIDTH)
            ? RM690B0_FB_FLUSH_TARGET_BYTES_FULLWIDTH
            : RM690B0_FB_FLUSH_TARGET_BYTES_PARTIAL;

        size_t bytes_per_row = flush_width * sizeof(uint16_t);
        if (bytes_per_row > 0) {
            size_t target_rows = target_bytes / bytes_per_row;
            if (target_rows == 0) {
                target_rows = 1;
            }
            if (rows > target_rows) {
                rows = target_rows;
            }
        }
    }

    return rm690b0_normalize_chunk_rows(rows, max_chunk_rows, flush_height);
}

static uint16_t *rm690b0_alloc_framebuffer_bytes(size_t framebuffer_bytes) {
    size_t aligned_bytes = rm690b0_align_up_size(framebuffer_bytes, RM690B0_FB_DMA_ALIGNMENT);
    uint16_t *framebuffer = heap_caps_aligned_alloc(
        RM690B0_FB_DMA_ALIGNMENT,
        aligned_bytes,
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    if (framebuffer != NULL) {
        return framebuffer;
    }

    // Fallback keeps previous behavior if aligned allocation fails.
    return heap_caps_malloc(framebuffer_bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

#if SOC_ASYNC_MEMCPY_SUPPORTED
static bool IRAM_ATTR rm690b0_on_fb_copy_done(
    async_memcpy_handle_t mcp_hdl,
    async_memcpy_event_t *event,
    void *cb_args) {
    (void)mcp_hdl;
    (void)event;

    SemaphoreHandle_t done_sem = (SemaphoreHandle_t)cb_args;
    BaseType_t high_task_awoken = pdFALSE;
    if (done_sem != NULL) {
        xSemaphoreGiveFromISR(done_sem, &high_task_awoken);
    }
    return high_task_awoken == pdTRUE;
}

static void rm690b0_release_fb_copy_dma(rm690b0_impl_t *impl) {
    if (impl == NULL) {
        return;
    }

    if (impl->fb_copy_dma_handle != NULL) {
        esp_err_t ret = esp_async_memcpy_uninstall(impl->fb_copy_dma_handle);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Failed to uninstall framebuffer-copy DMA handle: %s",
                esp_err_to_name(ret));
        }
        impl->fb_copy_dma_handle = NULL;
    }

    if (impl->fb_copy_done_sem != NULL) {
        vSemaphoreDelete(impl->fb_copy_done_sem);
        impl->fb_copy_done_sem = NULL;
    }

    impl->fb_copy_dma_disabled = false;
}

static bool rm690b0_try_init_fb_copy_dma(rm690b0_impl_t *impl) {
    if (impl == NULL || impl->fb_copy_dma_disabled) {
        return false;
    }
    if (impl->fb_copy_dma_handle != NULL) {
        return true;
    }

    if (impl->fb_copy_done_sem == NULL) {
        impl->fb_copy_done_sem = xSemaphoreCreateBinary();
        if (impl->fb_copy_done_sem == NULL) {
            ESP_LOGW(TAG, "Unable to create framebuffer-copy DMA semaphore; using CPU memcpy");
            impl->fb_copy_dma_disabled = true;
            return false;
        }
    }

    async_memcpy_config_t cfg = ASYNC_MEMCPY_DEFAULT_CONFIG();
    cfg.backlog = 1;

    esp_err_t ret = esp_async_memcpy_install(&cfg, &impl->fb_copy_dma_handle);
    if (ret != ESP_OK || impl->fb_copy_dma_handle == NULL) {
        ESP_LOGW(TAG, "Unable to initialize framebuffer-copy DMA (%s); using CPU memcpy",
            esp_err_to_name(ret));
        rm690b0_release_fb_copy_dma(impl);
        impl->fb_copy_dma_disabled = true;
        return false;
    }

    return true;
}

static bool rm690b0_try_copy_framebuffer_dma(
    rm690b0_impl_t *impl,
    void *dst,
    const void *src,
    size_t bytes) {

    if (bytes == 0 || dst == NULL || src == NULL) {
        return true;
    }

    if (!rm690b0_try_init_fb_copy_dma(impl)) {
        return false;
    }

    while (xSemaphoreTake(impl->fb_copy_done_sem, 0) == pdTRUE) {
        // Drain stale completion tokens before enqueuing a fresh transaction.
    }

    esp_err_t ret = esp_async_memcpy(
        impl->fb_copy_dma_handle,
        dst,
        (void *)src,
        bytes,
        rm690b0_on_fb_copy_done,
        impl->fb_copy_done_sem);

    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Framebuffer DMA copy request rejected (%s); switching to CPU memcpy",
            esp_err_to_name(ret));
        rm690b0_release_fb_copy_dma(impl);
        impl->fb_copy_dma_disabled = true;
        return false;
    }

    if (xSemaphoreTake(impl->fb_copy_done_sem, pdMS_TO_TICKS(RM690B0_FB_COPY_TIMEOUT_MS)) != pdTRUE) {
        ESP_LOGE(TAG, "Framebuffer DMA copy timed out; falling back to CPU memcpy");
        rm690b0_release_fb_copy_dma(impl);
        impl->fb_copy_dma_disabled = true;
        return false;
    }

    return true;
}
#else
static void rm690b0_release_fb_copy_dma(rm690b0_impl_t *impl) {
    (void)impl;
}

static bool rm690b0_try_copy_framebuffer_dma(
    rm690b0_impl_t *impl,
    void *dst,
    const void *src,
    size_t bytes) {
    (void)impl;
    (void)dst;
    (void)src;
    (void)bytes;
    return false;
}
#endif

static void rm690b0_copy_framebuffer_region(
    rm690b0_impl_t *impl,
    uint16_t *dst,
    const uint16_t *src,
    mp_int_t x,
    mp_int_t y,
    mp_int_t width,
    mp_int_t height) {

    if (dst == NULL || src == NULL || width <= 0 || height <= 0) {
        return;
    }

    mp_int_t copy_x = x;
    mp_int_t copy_y = y;
    mp_int_t copy_w = width;
    mp_int_t copy_h = height;
    if (!expand_even_region(&copy_x, &copy_y, &copy_w, &copy_h)) {
        return;
    }

    // Promote near-full-width regions to full-width so copy=True can use the
    // contiguous fast path (DMA or memcpy block copy) instead of row slices.
    if (copy_w < RM690B0_PANEL_WIDTH) {
        mp_int_t copy_x2 = rm690b0_add_mp_int_saturating(copy_x, copy_w);
        if (copy_x >= 0 && copy_x2 <= RM690B0_PANEL_WIDTH) {
            mp_int_t left_gap = copy_x;
            mp_int_t right_gap = RM690B0_PANEL_WIDTH - copy_x2;
            if (left_gap <= RM690B0_FB_COPY_NEAR_FULLWIDTH_MARGIN &&
                right_gap <= RM690B0_FB_COPY_NEAR_FULLWIDTH_MARGIN) {
                copy_x = 0;
                copy_w = RM690B0_PANEL_WIDTH;
            }
        }
    }

    const size_t fb_stride = RM690B0_PANEL_WIDTH;
    const size_t row_bytes = (size_t)copy_w * sizeof(uint16_t);
    uint16_t *dst_base = dst + (size_t)copy_y * fb_stride + (size_t)copy_x;
    const uint16_t *src_base = src + (size_t)copy_y * fb_stride + (size_t)copy_x;

    if (copy_x == 0 && copy_w == RM690B0_PANEL_WIDTH) {
        size_t copy_bytes = (size_t)copy_h * row_bytes;
        if (!rm690b0_try_copy_framebuffer_dma(impl, dst_base, src_base, copy_bytes)) {
            memcpy(dst_base, src_base, copy_bytes);
        }
        return;
    }

    for (mp_int_t row = 0; row < copy_h; row++) {
        uint16_t *dst_row = dst_base + (size_t)row * fb_stride;
        const uint16_t *src_row = src_base + (size_t)row * fb_stride;
        memcpy(dst_row, src_row, row_bytes);
    }
}

static esp_err_t rm690b0_tx_param(const rm690b0_impl_t *impl, uint8_t cmd, const void *param, size_t param_size) {
    if (impl == NULL || impl->io_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint32_t packed_cmd = ((uint32_t)RM690B0_OPCODE_WRITE_CMD << 24) | ((uint32_t)cmd << 8);
    return esp_lcd_panel_io_tx_param(impl->io_handle, packed_cmd, param, param_size);
}

static bool rm690b0_try_wait_for_all_dma(rm690b0_impl_t *impl, const char *context) {
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        rm690b0_wait_for_all_dma(impl);
        nlr_pop();
        return true;
    }

    ESP_LOGW(TAG, "%s: DMA wait raised, continuing deinit cleanup", context);
    return false;
}

static bool rm690b0_try_flush_region(
    rm690b0_rm690b0_obj_t *self,
    mp_int_t x, mp_int_t y, mp_int_t width, mp_int_t height,
    esp_err_t *out_ret) {

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        *out_ret = rm690b0_flush_region(self, x, y, width, height);
        nlr_pop();
        return true;
    }

    *out_ret = ESP_FAIL;
    return false;
}

// ============================================================================
// Dirty region tracking
// ============================================================================

#define RM690B0_DIRTY_TRANSFER_OVERHEAD_PIXELS      (4096u)
#define RM690B0_DIRTY_COALESCE_GAP_PIXELS           (2)
#define RM690B0_DIRTY_COALESCE_EXTRA_LIMIT_PIXELS   (4096u)

typedef struct {
    rm690b0_dirty_rect_t rects[RM690B0_MAX_DIRTY_RECTS];
    size_t rect_count;
    rm690b0_dirty_rect_t merged_rect;
    bool use_merged;
} rm690b0_dirty_flush_plan_t;

static inline bool rm690b0_dirty_rect_is_valid(const rm690b0_dirty_rect_t *rect) {
    return rect != NULL && rect->w > 0 && rect->h > 0;
}

static uint64_t rm690b0_dirty_rect_area_u64(const rm690b0_dirty_rect_t *rect) {
    if (!rm690b0_dirty_rect_is_valid(rect)) {
        return 0;
    }
    return (uint64_t)(size_t)rect->w * (uint64_t)(size_t)rect->h;
}

static rm690b0_dirty_rect_t rm690b0_dirty_rect_union(
    const rm690b0_dirty_rect_t *a,
    const rm690b0_dirty_rect_t *b) {

    rm690b0_dirty_rect_t out = {0, 0, 0, 0};
    if (!rm690b0_dirty_rect_is_valid(a)) {
        return *b;
    }
    if (!rm690b0_dirty_rect_is_valid(b)) {
        return *a;
    }

    mp_int_t ax2 = rm690b0_add_mp_int_saturating(a->x, a->w);
    mp_int_t ay2 = rm690b0_add_mp_int_saturating(a->y, a->h);
    mp_int_t bx2 = rm690b0_add_mp_int_saturating(b->x, b->w);
    mp_int_t by2 = rm690b0_add_mp_int_saturating(b->y, b->h);

    out.x = (a->x < b->x) ? a->x : b->x;
    out.y = (a->y < b->y) ? a->y : b->y;
    mp_int_t out_x2 = (ax2 > bx2) ? ax2 : bx2;
    mp_int_t out_y2 = (ay2 > by2) ? ay2 : by2;
    out.w = out_x2 - out.x;
    out.h = out_y2 - out.y;
    return out;
}

static void rm690b0_dirty_rect_distance(
    const rm690b0_dirty_rect_t *a,
    const rm690b0_dirty_rect_t *b,
    mp_int_t *out_dx,
    mp_int_t *out_dy) {

    mp_int_t ax2 = rm690b0_add_mp_int_saturating(a->x, a->w);
    mp_int_t ay2 = rm690b0_add_mp_int_saturating(a->y, a->h);
    mp_int_t bx2 = rm690b0_add_mp_int_saturating(b->x, b->w);
    mp_int_t by2 = rm690b0_add_mp_int_saturating(b->y, b->h);

    if (ax2 < b->x) {
        *out_dx = b->x - ax2;
    } else if (bx2 < a->x) {
        *out_dx = a->x - bx2;
    } else {
        *out_dx = 0;
    }

    if (ay2 < b->y) {
        *out_dy = b->y - ay2;
    } else if (by2 < a->y) {
        *out_dy = a->y - by2;
    } else {
        *out_dy = 0;
    }
}

static bool rm690b0_should_merge_dirty_rects(
    const rm690b0_dirty_rect_t *a,
    const rm690b0_dirty_rect_t *b) {

    if (!rm690b0_dirty_rect_is_valid(a) || !rm690b0_dirty_rect_is_valid(b)) {
        return false;
    }

    mp_int_t dx = 0;
    mp_int_t dy = 0;
    rm690b0_dirty_rect_distance(a, b, &dx, &dy);

    if (dx == 0 && dy == 0) {
        return true;
    }
    if (dx > RM690B0_DIRTY_COALESCE_GAP_PIXELS ||
        dy > RM690B0_DIRTY_COALESCE_GAP_PIXELS) {
        return false;
    }

    uint64_t area_sum = rm690b0_dirty_rect_area_u64(a) + rm690b0_dirty_rect_area_u64(b);
    rm690b0_dirty_rect_t union_rect = rm690b0_dirty_rect_union(a, b);
    uint64_t union_area = rm690b0_dirty_rect_area_u64(&union_rect);

    uint64_t max_extra = (area_sum >> 2); // +25%
    if (max_extra > RM690B0_DIRTY_COALESCE_EXTRA_LIMIT_PIXELS) {
        max_extra = RM690B0_DIRTY_COALESCE_EXTRA_LIMIT_PIXELS;
    }
    return union_area <= (area_sum + max_extra);
}

static size_t rm690b0_coalesce_dirty_rects(rm690b0_dirty_rect_t *rects, size_t count) {
    if (count < 2) {
        return count;
    }

    bool merged_any = true;
    while (merged_any && count > 1) {
        merged_any = false;
        for (size_t i = 0; i < count; i++) {
            for (size_t j = i + 1; j < count; j++) {
                if (!rm690b0_should_merge_dirty_rects(&rects[i], &rects[j])) {
                    continue;
                }
                rects[i] = rm690b0_dirty_rect_union(&rects[i], &rects[j]);
                rects[j] = rects[count - 1];
                count--;
                merged_any = true;
                goto next_pass;
            }
        }
next_pass:
        ;
    }
    return count;
}

static bool rm690b0_build_dirty_flush_plan(
    const rm690b0_impl_t *impl,
    rm690b0_dirty_flush_plan_t *plan) {

    if (impl == NULL || plan == NULL) {
        return false;
    }

    memset(plan, 0, sizeof(*plan));

    for (size_t i = 0; i < impl->dirty_count && plan->rect_count < RM690B0_MAX_DIRTY_RECTS; i++) {
        rm690b0_dirty_rect_t rect = impl->dirty_rects[i];
        if (!rm690b0_dirty_rect_is_valid(&rect)) {
            continue;
        }
        plan->rects[plan->rect_count++] = rect;
    }

    if (plan->rect_count == 0) {
        if (!impl->dirty_merged_valid) {
            return false;
        }
        rm690b0_dirty_rect_t merged = {
            impl->dirty_merged_x,
            impl->dirty_merged_y,
            impl->dirty_merged_w,
            impl->dirty_merged_h,
        };
        if (!rm690b0_dirty_rect_is_valid(&merged)) {
            return false;
        }
        plan->merged_rect = merged;
        plan->use_merged = true;
        return true;
    }

    plan->rect_count = rm690b0_coalesce_dirty_rects(plan->rects, plan->rect_count);
    if (plan->rect_count == 0) {
        return false;
    }

    plan->merged_rect = plan->rects[0];
    uint64_t individual_area = rm690b0_dirty_rect_area_u64(&plan->rects[0]);
    for (size_t i = 1; i < plan->rect_count; i++) {
        plan->merged_rect = rm690b0_dirty_rect_union(&plan->merged_rect, &plan->rects[i]);
        individual_area += rm690b0_dirty_rect_area_u64(&plan->rects[i]);
    }

    if (plan->rect_count == 1) {
        plan->use_merged = true;
        return true;
    }

    uint64_t merged_area = rm690b0_dirty_rect_area_u64(&plan->merged_rect);
    uint64_t individual_cost = individual_area +
        ((uint64_t)plan->rect_count * RM690B0_DIRTY_TRANSFER_OVERHEAD_PIXELS);
    uint64_t merged_cost = merged_area + RM690B0_DIRTY_TRANSFER_OVERHEAD_PIXELS;
    plan->use_merged = merged_cost <= individual_cost;
    return true;
}

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
    // In single-buffer mode, we disable direct DMA to prevent Python from
    // overwriting the framebuffer while DMA is still reading it.
    bool direct_dma = impl->double_buffered && (fx == 0 && fw == RM690B0_PANEL_WIDTH && x == 0 && width == RM690B0_PANEL_WIDTH);

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
    size_t chunk_height = rm690b0_choose_flush_chunk_rows(
        impl, fw_sz, fh_sz, max_chunk_height, direct_dma);

    size_t max_chunk_pixels = fw_sz * chunk_height;

    bool use_static_buffers = false;
    uint16_t *alloc_buffer = NULL;
    if (!direct_dma) {
        use_static_buffers = (impl->chunk_buffers[0] != NULL &&
            impl->chunk_buffer_pixels >= max_chunk_pixels);

        if (!use_static_buffers) {
            alloc_buffer = heap_caps_malloc(max_chunk_pixels * sizeof(uint16_t), MALLOC_CAP_DMA);
            if (alloc_buffer == NULL) {
                return ESP_ERR_NO_MEM;
            }
            impl->dma_alloc_buffer_ptr = alloc_buffer;
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
                if (impl->chunk_buffers[1] != NULL) {
                    buf_idx = (buf_idx + 1) % 2;
                } else {
                    buf_idx = 0;
                }
            } else {
                while (impl->dma_alloc_buffer_in_use) {
                    rm690b0_wait_for_dma_completion(impl);
                }
                current_buffer = alloc_buffer;
                impl->dma_alloc_buffer_in_use = true;
                pending_id = RM690B0_PENDING_BUFFER_ALLOC;
            }

            dma_buffer = current_buffer;

            if (fx == 0 && fw == RM690B0_PANEL_WIDTH) {
                size_t bytes_per_row = fb_stride * sizeof(uint16_t);
                size_t chunk_bytes = rows_this_chunk * bytes_per_row;
                const uint16_t *src = framebuffer + (size_t)start_y * fb_stride;
                memcpy(current_buffer, src, chunk_bytes);
            } else {
                // expand_even_region() clamps to panel bounds, so the flush window
                // can always be copied row-by-row without edge replication.
                const uint16_t *src = framebuffer + (size_t)start_y * fb_stride + (size_t)fx;
                size_t row_bytes = fw_sz * sizeof(uint16_t);
                size_t dest_index = 0;
                for (size_t row = 0; row < rows_this_chunk; row++) {
                    memcpy(&current_buffer[dest_index], src + row * fb_stride, row_bytes);
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
        impl->dma_alloc_buffer_ptr = NULL;
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

    rm690b0_wait_for_all_dma(impl);

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
        impl->dma_alloc_buffer_ptr = dma_buffer;
    }

    const size_t dma_pixels = line_pixels * dma_lines;
    rm690b0_fill_span_fast(dma_buffer, dma_pixels, swapped);

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
        impl->dma_alloc_buffer_ptr = NULL;
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

    rm690b0_wait_for_all_dma(impl);

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
        impl->dma_alloc_buffer_ptr = dma_buffer;
    }

    const size_t dma_pixels = line_pixels * dma_lines;
    rm690b0_fill_span_fast(dma_buffer, dma_pixels, swapped_color);

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
        impl->dma_alloc_buffer_ptr = NULL;
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
    if (rm690b0_singleton != NULL && rm690b0_singleton != self) {
        ESP_LOGI(TAG, "Cleaning up previous singleton before creating new instance");
        common_hal_rm690b0_rm690b0_deinit_all();
    }

    self->initialized = false;
    self->width = RM690B0_PANEL_WIDTH;
    self->height = RM690B0_PANEL_HEIGHT;
    self->rotation = 0;
    self->brightness_raw = 0xFF;
    self->font_id = RM690B0_FONT_8x8_MONO;
    self->buffer_mode = RM690B0_BUFFER_DOUBLE;
    self->render_mode = RM690B0_RENDER_FRAMEBUFFER;
    self->impl = m_malloc(sizeof(rm690b0_impl_t));
    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    memset(impl, 0, sizeof(rm690b0_impl_t));
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
    impl->front_buffer_alloc_failed = false;
#if SOC_ASYNC_MEMCPY_SUPPORTED
    impl->fb_copy_dma_handle = NULL;
    impl->fb_copy_done_sem = NULL;
    impl->fb_copy_dma_disabled = false;
#endif
    impl->circle_span_cache = NULL;
    impl->circle_span_capacity = 0;

    impl->dirty_count = 0;
    impl->dirty_merged_valid = false;
    rm690b0_dl_init_state(impl);

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
    impl->fatal_dma_error = false;

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

    bool singleton_owned = (rm690b0_singleton == self);
    bool dma_wait_failed = false;

    if (impl->panel_handle != NULL && impl->framebuffer != NULL) {
        ESP_LOGI(TAG, "Clearing display to black before deinit");

        size_t framebuffer_pixels = (size_t)RM690B0_PANEL_WIDTH * RM690B0_PANEL_HEIGHT;
        memset(impl->framebuffer, 0, framebuffer_pixels * sizeof(uint16_t));

        esp_err_t ret = ESP_OK;
        bool flush_completed = rm690b0_try_flush_region(
            self, 0, 0, RM690B0_PANEL_WIDTH, RM690B0_PANEL_HEIGHT, &ret);
        if (flush_completed && ret == ESP_OK) {
            if (rm690b0_try_wait_for_all_dma(impl, "deinit clear")) {
                vTaskDelay(pdMS_TO_TICKS(10));
                ESP_LOGI(TAG, "Screen cleared successfully");
            } else {
                ESP_LOGW(TAG, "Screen clear started but DMA drain failed during deinit");
                dma_wait_failed = true;
            }
        } else if (flush_completed) {
            ESP_LOGW(TAG, "Failed to clear screen before deinit (%s)", esp_err_to_name(ret));
        } else {
            ESP_LOGW(TAG, "Failed to clear screen before deinit (exception raised)");
            dma_wait_failed = true;
        }
    }
    
    self->initialized = false;

    if (!rm690b0_try_wait_for_all_dma(impl, "deinit pre-free")) {
        dma_wait_failed = true;
    }

    if (dma_wait_failed) {
        ESP_LOGE(TAG, "deinit aborted: DMA state uncertain after timeout/exception; hardware reset required");
        // We still mark it uninitialized so init_display can be attempted, 
        // but we must clean up panel and IO to avoid handle leaks on re-init.
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
        mp_raise_msg(&mp_type_RuntimeError,
            MP_ERROR_TEXT("Deinit aborted: DMA state uncertain; reset hardware and retry"));
        return;
    }
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

    if (impl->dma_alloc_buffer_ptr) {
        ESP_LOGI(TAG, "Freeing orphaned DMA alloc buffer");
        heap_caps_free(impl->dma_alloc_buffer_ptr);
        impl->dma_alloc_buffer_ptr = NULL;
        impl->dma_alloc_buffer_in_use = false;
    }

    if (impl->circle_span_cache) {
        ESP_LOGI(TAG, "Freeing cached circle spans");
        heap_caps_free(impl->circle_span_cache);
        impl->circle_span_cache = NULL;
        impl->circle_span_capacity = 0;
    }

    rm690b0_dl_deinit_state(impl);
    rm690b0_release_fb_copy_dma(impl);

    if (impl->transfer_done_sem) {
        vSemaphoreDelete(impl->transfer_done_sem);
        impl->transfer_done_sem = NULL;
    }

    if (LCD_PWR_PIN != GPIO_NUM_NC) {
        ESP_LOGI(TAG, "Turning off display power");
        int off_level = LCD_PWR_ON_LEVEL ? 0 : 1;
        esp_err_t pwr_ret = gpio_set_level(LCD_PWR_PIN, off_level);
        if (pwr_ret != ESP_OK) {
            ESP_LOGW(TAG, "Failed to drive power GPIO low: %s", esp_err_to_name(pwr_ret));
        }
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

    m_free(impl);
    self->impl = NULL;
    if (singleton_owned) {
        rm690b0_singleton = NULL;
    }

    ESP_LOGI(TAG, "RM690B0 deinit complete - all resources freed");
}

void common_hal_rm690b0_rm690b0_deinit_all(void) {
    if (rm690b0_singleton != NULL) {
        ESP_LOGI(TAG, "Static deinit: cleaning up singleton instance");
        rm690b0_rm690b0_obj_t *active = rm690b0_singleton;
        common_hal_rm690b0_rm690b0_deinit(active);
        if (rm690b0_singleton == NULL) {
            ESP_LOGI(TAG, "Static deinit: singleton cleaned up");
        } else {
            ESP_LOGW(TAG, "Static deinit: singleton still active after cleanup attempt");
        }
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

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL) {
        mp_raise_msg(&mp_type_RuntimeError,
            MP_ERROR_TEXT("Display object was deinitialized; create a new RM690B0() instance"));
        return;
    }
    if (impl->fatal_dma_error) {
        mp_raise_msg(&mp_type_RuntimeError,
            MP_ERROR_TEXT("Display is in fatal DMA state; power-cycle and create a new RM690B0()"));
        return;
    }
    if (impl->panel_handle != NULL || impl->io_handle != NULL || impl->bus_initialized) {
        mp_raise_msg(&mp_type_RuntimeError,
            MP_ERROR_TEXT("Display object has stale hardware state; call deinit() and create a new RM690B0()"));
        return;
    }

    esp_err_t ret = ESP_OK;

    ESP_LOGI(TAG, "Initializing RM690B0 display (%s mode)", LCD_USE_QSPI ? "QSPI" : "SPI");

    if (LCD_PWR_PIN != GPIO_NUM_NC) {
        gpio_config_t io_conf = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = (1ULL << LCD_PWR_PIN),
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        ret = gpio_config(&io_conf);
        if (ret != ESP_OK) {
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to configure power GPIO: %s"), esp_err_to_name(ret));
            return;
        }
        ret = gpio_set_level(LCD_PWR_PIN, LCD_PWR_ON_LEVEL);
        if (ret != ESP_OK) {
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to enable display power: %s"), esp_err_to_name(ret));
            return;
        }
        ESP_LOGI(TAG, "Display power enabled on GPIO%d", LCD_PWR_PIN);
    } else {
        ESP_LOGI(TAG, "No display power control pin configured; assuming panel is already powered");
    }

    vTaskDelay(pdMS_TO_TICKS(200));

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

    ret = esp_lcd_panel_set_gap(impl->panel_handle, RM690B0_X_GAP, RM690B0_Y_GAP);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set panel gap: %s", esp_err_to_name(ret));
        goto cleanup;
    }
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
            impl->chunk_buffers[1] = NULL;
            if (impl->chunk_buffers[0] != NULL) {
                // Try to get a second DMA chunk buffer even in single framebuffer mode.
                // In single mode this is optional (for better DMA/CPU overlap in DL).
                impl->chunk_buffers[1] = heap_caps_malloc(buf_size, MALLOC_CAP_DMA);
            }

            bool dual_required = self->buffer_mode != RM690B0_BUFFER_SINGLE;
            if (impl->chunk_buffers[0] != NULL && (!dual_required || impl->chunk_buffers[1] != NULL)) {
                impl->chunk_buffer_pixels = buf_pixels;
                ESP_LOGI(TAG, "Allocated %s DMA chunk buffer(s) (%zu pixels each, %zu rows)",
                    impl->chunk_buffers[1] != NULL ? "dual" : "single", impl->chunk_buffer_pixels, current_chunk_rows);
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
            ESP_LOGW(TAG, "Unable to allocate %s DMA chunk buffer(s); operations will use slower allocation per draw",
                self->buffer_mode == RM690B0_BUFFER_SINGLE ? "single" : "dual");
        }
    }

    size_t framebuffer_pixels = (size_t)RM690B0_PANEL_WIDTH * RM690B0_PANEL_HEIGHT;
    size_t framebuffer_bytes = framebuffer_pixels * sizeof(uint16_t);
    if (impl->framebuffer == NULL) {
        impl->framebuffer = rm690b0_alloc_framebuffer_bytes(framebuffer_bytes);
        if (impl->framebuffer == NULL) {
            ESP_LOGE(TAG, "Unable to allocate PSRAM framebuffer");
            goto cleanup;
        }
        impl->framebuffer_pixels = framebuffer_pixels;
        ESP_LOGI(TAG, "Allocated back framebuffer (%zu pixels, %zu KB)",
            framebuffer_pixels, (framebuffer_pixels * sizeof(uint16_t)) / 1024);
    }

    memset(impl->framebuffer, 0, framebuffer_bytes);
    esp_err_t clear_ret = ESP_OK;
    bool flush_completed = rm690b0_try_flush_region(
        self, 0, 0, RM690B0_PANEL_WIDTH, RM690B0_PANEL_HEIGHT, &clear_ret);
    
    if (!flush_completed || clear_ret != ESP_OK) {
        ret = clear_ret;
        goto cleanup;
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
    impl->front_buffer_alloc_failed = false;
    impl->fatal_dma_error = false;
    self->initialized = true;
    ESP_LOGI(TAG, "RM690B0 display initialization complete");
    return;

cleanup:
    if (impl->framebuffer_front) {
        heap_caps_free(impl->framebuffer_front);
        impl->framebuffer_front = NULL;
        impl->double_buffered = false;
    }
    if (impl->framebuffer) {
        heap_caps_free(impl->framebuffer);
        impl->framebuffer = NULL;
        impl->framebuffer_pixels = 0;
    }
    if (impl->circle_span_cache) {
        heap_caps_free(impl->circle_span_cache);
        impl->circle_span_cache = NULL;
        impl->circle_span_capacity = 0;
    }

    if (impl->chunk_buffers[0]) {
        heap_caps_free(impl->chunk_buffers[0]);
        impl->chunk_buffers[0] = NULL;
    }
    if (impl->chunk_buffers[1]) {
        heap_caps_free(impl->chunk_buffers[1]);
        impl->chunk_buffers[1] = NULL;
    }
    impl->chunk_buffer_pixels = 0;

    if (impl->dma_alloc_buffer_ptr) {
        heap_caps_free(impl->dma_alloc_buffer_ptr);
        impl->dma_alloc_buffer_ptr = NULL;
        impl->dma_alloc_buffer_in_use = false;
    }

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

void common_hal_rm690b0_rm690b0_set_render_mode(rm690b0_rm690b0_obj_t *self, mp_int_t mode) {
    if (mode != RM690B0_RENDER_FRAMEBUFFER && mode != RM690B0_RENDER_DISPLAY_LIST) {
        mp_raise_ValueError(MP_ERROR_TEXT("render_mode must be RENDER_FRAMEBUFFER or RENDER_DISPLAY_LIST"));
        return;
    }

    if (self->render_mode == mode) {
        return;
    }

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Invalid display handle"));
        return;
    }

    if (!self->initialized) {
        self->render_mode = mode;
        return;
    }

    esp_err_t ret = rm690b0_dl_set_mode(self, mode);
    if (ret != ESP_OK) {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to switch render mode: %s"), esp_err_to_name(ret));
    }
}

mp_int_t common_hal_rm690b0_rm690b0_get_render_mode(const rm690b0_rm690b0_obj_t *self) {
    return self->render_mode;
}

void common_hal_rm690b0_rm690b0_compact_display_list(rm690b0_rm690b0_obj_t *self) {
    CHECK_INITIALIZED();

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Invalid display handle"));
        return;
    }
    if (self->render_mode != RM690B0_RENDER_DISPLAY_LIST) {
        return;
    }

    rm690b0_wait_for_all_dma(impl);
    esp_err_t ret = rm690b0_dl_compact(self);
    if (ret != ESP_OK) {
        mp_raise_msg_varg(&mp_type_RuntimeError,
            MP_ERROR_TEXT("Failed to compact display list: %s"), esp_err_to_name(ret));
    }
}

void common_hal_rm690b0_rm690b0_get_display_list_stats(rm690b0_rm690b0_obj_t *self, rm690b0_display_list_stats_t *out) {
    if (out == NULL) {
        return;
    }
    memset(out, 0, sizeof(*out));

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL) {
        return;
    }

    const rm690b0_dl_state_t *dl = &impl->dl;
    out->command_count = dl->count;
    out->payload_bytes = dl->payload_bytes;
    out->max_command_count = dl->telemetry_max_command_count;
    out->max_payload_bytes = dl->telemetry_max_payload_bytes;
    out->rejected_command_limit = dl->telemetry_rejected_command_limit;
    out->rejected_payload_limit = dl->telemetry_rejected_payload_limit;
    out->allocation_failures = dl->telemetry_allocation_failures;
    out->present_count = dl->telemetry_present_count;
    out->present_full = dl->telemetry_present_full;
    out->present_partial = dl->telemetry_present_partial;
    out->compact_count = dl->telemetry_compact_count;
    out->compact_trimmed_commands = dl->telemetry_compact_trimmed_commands;
    out->auto_compact_trigger_periodic = dl->telemetry_auto_compact_trigger_periodic;
    out->auto_compact_trigger_command_guard = dl->telemetry_auto_compact_trigger_command_guard;
    out->auto_compact_trigger_payload_guard = dl->telemetry_auto_compact_trigger_payload_guard;
    out->glyph_atlas_hits = dl->telemetry_glyph_atlas_hits;
    out->glyph_atlas_misses = dl->telemetry_glyph_atlas_misses;
    out->glyph_atlas_builds = dl->telemetry_glyph_atlas_builds;
    out->glyph_atlas_evictions = dl->telemetry_glyph_atlas_evictions;
}

void common_hal_rm690b0_rm690b0_reset_display_list_stats(rm690b0_rm690b0_obj_t *self) {
    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;
    if (impl == NULL) {
        return;
    }

    rm690b0_dl_state_t *dl = &impl->dl;
    dl->telemetry_max_command_count = dl->count;
    dl->telemetry_max_payload_bytes = dl->payload_bytes;
    dl->telemetry_rejected_command_limit = 0;
    dl->telemetry_rejected_payload_limit = 0;
    dl->telemetry_allocation_failures = 0;
    dl->telemetry_present_count = 0;
    dl->telemetry_present_full = 0;
    dl->telemetry_present_partial = 0;
    dl->telemetry_compact_count = 0;
    dl->telemetry_compact_trimmed_commands = 0;
    dl->telemetry_auto_compact_trigger_periodic = 0;
    dl->telemetry_auto_compact_trigger_command_guard = 0;
    dl->telemetry_auto_compact_trigger_payload_guard = 0;
    dl->telemetry_glyph_atlas_hits = 0;
    dl->telemetry_glyph_atlas_misses = 0;
    dl->telemetry_glyph_atlas_builds = 0;
    dl->telemetry_glyph_atlas_evictions = 0;
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

static void rm690b0_dl_maybe_auto_compact(rm690b0_rm690b0_obj_t *self, rm690b0_impl_t *impl, bool keep_commands) {
#if RM690B0_DL_AUTO_COMPACT_ENABLE
    if (self == NULL || impl == NULL || self->render_mode != RM690B0_RENDER_DISPLAY_LIST) {
        return;
    }

    rm690b0_dl_state_t *dl = &impl->dl;
    if (!keep_commands) {
        dl->auto_compact_frames_since_attempt = 0;
        return;
    }

    if (dl->count < RM690B0_DL_AUTO_COMPACT_MIN_COMMANDS) {
        dl->auto_compact_frames_since_attempt = 0;
        return;
    }

    if (dl->auto_compact_frames_since_attempt < UINT16_MAX) {
        dl->auto_compact_frames_since_attempt++;
    }

    size_t command_guard = RM690B0_DL_AUTO_COMPACT_GUARD_COMMANDS;
    if (command_guard > RM690B0_DL_MAX_COMMANDS) {
        command_guard = RM690B0_DL_MAX_COMMANDS;
    }
    size_t payload_guard = RM690B0_DL_AUTO_COMPACT_GUARD_PAYLOAD_BYTES;
    if (payload_guard > RM690B0_DL_MAX_PAYLOAD_BYTES) {
        payload_guard = RM690B0_DL_MAX_PAYLOAD_BYTES;
    }

    bool periodic_trigger = RM690B0_DL_AUTO_COMPACT_EVERY_N_FRAMES > 0 &&
        dl->auto_compact_frames_since_attempt >= RM690B0_DL_AUTO_COMPACT_EVERY_N_FRAMES;
    bool near_limit_trigger = command_guard > 0 &&
        dl->count >= command_guard &&
        dl->auto_compact_frames_since_attempt >= RM690B0_DL_AUTO_COMPACT_GUARD_COOLDOWN_FRAMES;
    bool near_payload_trigger = payload_guard > 0 &&
        dl->payload_bytes >= payload_guard &&
        dl->auto_compact_frames_since_attempt >= RM690B0_DL_AUTO_COMPACT_GUARD_COOLDOWN_FRAMES;
    if (!periodic_trigger && !near_limit_trigger && !near_payload_trigger) {
        return;
    }

    if (periodic_trigger) {
        dl->telemetry_auto_compact_trigger_periodic++;
    }
    if (near_limit_trigger) {
        dl->telemetry_auto_compact_trigger_command_guard++;
    }
    if (near_payload_trigger) {
        dl->telemetry_auto_compact_trigger_payload_guard++;
    }

    // Compact is best-effort and should never abort rendering.
    // In the no-trim case rm690b0_dl_compact() is O(1)-ish bookkeeping only.
    (void)rm690b0_dl_compact(self);
    dl->auto_compact_frames_since_attempt = 0;
#else
    (void)self;
    (void)impl;
    (void)keep_commands;
#endif
}

// ============================================================================
// swap_buffers
// ============================================================================

void common_hal_rm690b0_rm690b0_swap_buffers(rm690b0_rm690b0_obj_t *self, bool copy) {
    CHECK_INITIALIZED();

    rm690b0_impl_t *impl = (rm690b0_impl_t *)self->impl;

    if (self->render_mode == RM690B0_RENDER_DISPLAY_LIST) {
        if (impl->dl.count > 0 || impl->dl.dirty_valid || impl->dl.force_full_present) {
            if (impl->dirty_count > 0) {
                mp_raise_msg(&mp_type_RuntimeError,
                    MP_ERROR_TEXT("Mixed framebuffer and display-list drawing is not supported yet"));
            }
            esp_err_t dl_ret = rm690b0_dl_present(self, copy);
            if (dl_ret != ESP_OK) {
                mp_raise_msg_varg(&mp_type_RuntimeError,
                    MP_ERROR_TEXT("Failed to refresh display list: %s (0x%x)"),
                    esp_err_to_name(dl_ret), dl_ret);
            }
            rm690b0_dl_maybe_auto_compact(self, impl, copy);
            return;
        }
    }

    if (!impl->double_buffered && impl->framebuffer_front == NULL
        && !impl->front_buffer_alloc_failed
        && self->buffer_mode != RM690B0_BUFFER_SINGLE) {
        size_t framebuffer_pixels = (size_t)RM690B0_PANEL_WIDTH * RM690B0_PANEL_HEIGHT;
        size_t framebuffer_bytes = framebuffer_pixels * sizeof(uint16_t);
        impl->framebuffer_front = rm690b0_alloc_framebuffer_bytes(framebuffer_bytes);

        if (impl->framebuffer_front == NULL) {
            if (!impl->front_buffer_alloc_failed) {
                ESP_LOGW(TAG, "Unable to allocate front framebuffer - falling back to single-buffered refresh");
                impl->front_buffer_alloc_failed = true;
            }
        } else {
            impl->double_buffered = true;
            impl->front_buffer_alloc_failed = false;
            ESP_LOGI(TAG, "Allocated front framebuffer (%zu KB) - double-buffering enabled",
                framebuffer_bytes / 1024);

            memcpy(impl->framebuffer_front, impl->framebuffer,
                framebuffer_bytes);
        }
    }

    if (!impl->double_buffered || impl->framebuffer_front == NULL) {
        rm690b0_dirty_flush_plan_t dirty_plan;
        if (rm690b0_build_dirty_flush_plan(impl, &dirty_plan)) {
            if (dirty_plan.use_merged) {
                esp_err_t ret = rm690b0_flush_region(
                    self,
                    dirty_plan.merged_rect.x,
                    dirty_plan.merged_rect.y,
                    dirty_plan.merged_rect.w,
                    dirty_plan.merged_rect.h);
                if (ret != ESP_OK) {
                    mp_raise_msg_varg(&mp_type_RuntimeError,
                        MP_ERROR_TEXT("Failed to refresh display: %s (0x%x)"),
                        esp_err_to_name(ret), ret);
                }
            } else {
                for (size_t i = 0; i < dirty_plan.rect_count; i++) {
                    rm690b0_dirty_rect_t *r = &dirty_plan.rects[i];
                    esp_err_t ret = rm690b0_flush_region(self, r->x, r->y, r->w, r->h);
                    if (ret != ESP_OK) {
                        mp_raise_msg_varg(&mp_type_RuntimeError,
                            MP_ERROR_TEXT("Failed to refresh display: %s (0x%x)"),
                            esp_err_to_name(ret), ret);
                    }
                }
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
    mp_int_t flush_w = RM690B0_PANEL_WIDTH;
    mp_int_t flush_h = RM690B0_PANEL_HEIGHT;
    bool copy_full_frame = true;
    rm690b0_dirty_rect_t copy_rects[RM690B0_MAX_DIRTY_RECTS];
    size_t copy_rect_count = 0;

    rm690b0_dirty_flush_plan_t dirty_plan;
    if (rm690b0_build_dirty_flush_plan(impl, &dirty_plan)) {
        copy_full_frame = false;
        if (dirty_plan.use_merged) {
            rm690b0_dirty_rect_t *r = &dirty_plan.merged_rect;
            copy_rects[0] = *r;
            copy_rect_count = 1;
            esp_err_t ret = rm690b0_flush_region(self, r->x, r->y, r->w, r->h);
            if (ret != ESP_OK) {
                mp_raise_msg_varg(&mp_type_RuntimeError,
                    MP_ERROR_TEXT("Failed to refresh display: %s (0x%x)"),
                    esp_err_to_name(ret), ret);
            }
        } else {
            for (size_t i = 0; i < dirty_plan.rect_count; i++) {
                rm690b0_dirty_rect_t *r = &dirty_plan.rects[i];
                if (copy_rect_count < RM690B0_MAX_DIRTY_RECTS) {
                    copy_rects[copy_rect_count++] = *r;
                }
                esp_err_t ret = rm690b0_flush_region(self, r->x, r->y, r->w, r->h);
                if (ret != ESP_OK) {
                    mp_raise_msg_varg(&mp_type_RuntimeError,
                        MP_ERROR_TEXT("Failed to refresh display: %s (0x%x)"),
                        esp_err_to_name(ret), ret);
                }
            }
        }
    } else {
        esp_err_t ret = rm690b0_flush_region(self, flush_x, flush_y, flush_w, flush_h);
        if (ret != ESP_OK) {
            mp_raise_msg_varg(&mp_type_RuntimeError,
                MP_ERROR_TEXT("Failed to refresh display: %s (0x%x)"),
                esp_err_to_name(ret), ret);
        }
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
        if (copy_full_frame) {
            size_t framebuffer_pixels = (size_t)RM690B0_PANEL_WIDTH * RM690B0_PANEL_HEIGHT;
            size_t framebuffer_bytes = framebuffer_pixels * sizeof(uint16_t);
            if (!rm690b0_try_copy_framebuffer_dma(
                    impl,
                    impl->framebuffer,
                    impl->framebuffer_front,
                    framebuffer_bytes)) {
                memcpy(impl->framebuffer, impl->framebuffer_front, framebuffer_bytes);
            }
        } else {
            for (size_t i = 0; i < copy_rect_count; i++) {
                rm690b0_dirty_rect_t *r = &copy_rects[i];
                rm690b0_copy_framebuffer_region(
                    impl,
                    impl->framebuffer,
                    impl->framebuffer_front,
                    r->x,
                    r->y,
                    r->w,
                    r->h);
            }
        }
    }
}
