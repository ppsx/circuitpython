// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

// ============================================================================
// Internal header — shared across RM690B0.c, rm690b0_text.c,
// rm690b0_image.c, rm690b0_draw.c.  NOT part of the public API.
// ============================================================================

#include "shared-bindings/rm690b0/RM690B0.h"
#include "common-hal/rm690b0/RM690B0.h"
#include "shared-bindings/microcontroller/Pin.h"
#include "py/runtime.h"
#include "py/mperrno.h"

#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_rom_sys.h"

#include "esp-idf/components/esp_lcd/include/esp_lcd_panel_io.h"
#include "esp-idf/components/esp_lcd/include/esp_lcd_panel_vendor.h"
#include "esp-idf/components/esp_lcd/include/esp_lcd_panel_ops.h"
#include "esp-idf/components/esp_lcd/include/esp_lcd_panel_commands.h"
#include "esp_lcd_rm690b0.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include "esp_attr.h"

// ============================================================================
// Board configuration macros
// ============================================================================

#if !defined(CIRCUITPY_RM690B0_QSPI_CS) || !defined(CIRCUITPY_RM690B0_QSPI_CLK) || \
    !defined(CIRCUITPY_RM690B0_QSPI_D0) || !defined(CIRCUITPY_RM690B0_QSPI_D1) || \
    !defined(CIRCUITPY_RM690B0_QSPI_D2) || !defined(CIRCUITPY_RM690B0_QSPI_D3) || \
    !defined(CIRCUITPY_RM690B0_RESET) || !defined(CIRCUITPY_RM690B0_WIDTH) || \
    !defined(CIRCUITPY_RM690B0_HEIGHT) || !defined(CIRCUITPY_RM690B0_BITS_PER_PIXEL)
#error "Board must define CIRCUITPY_RM690B0_* macros to describe the RM690B0 hardware"
#endif

#ifndef CIRCUITPY_RM690B0_POWER
#define CIRCUITPY_RM690B0_POWER (NULL)
#endif

#ifndef CIRCUITPY_RM690B0_POWER_ON_LEVEL
#define CIRCUITPY_RM690B0_POWER_ON_LEVEL (1)
#endif

#ifndef CIRCUITPY_RM690B0_USE_QSPI
#define CIRCUITPY_RM690B0_USE_QSPI (0)
#endif

#ifndef CIRCUITPY_RM690B0_X_GAP
#define CIRCUITPY_RM690B0_X_GAP (0)
#endif

#ifndef CIRCUITPY_RM690B0_Y_GAP
#define CIRCUITPY_RM690B0_Y_GAP (16)
#endif

#ifndef CIRCUITPY_RM690B0_PIXEL_CLOCK_HZ
#define CIRCUITPY_RM690B0_PIXEL_CLOCK_HZ (80 * 1000 * 1000)
#endif

#define PIN_GPIO(pin_obj) ((pin_obj) == NULL ? (gpio_num_t)GPIO_NUM_NC : (gpio_num_t)(pin_obj)->number)

#define LCD_CS_PIN          PIN_GPIO(CIRCUITPY_RM690B0_QSPI_CS)
#define LCD_SCK_PIN         PIN_GPIO(CIRCUITPY_RM690B0_QSPI_CLK)
#define LCD_D0_PIN          PIN_GPIO(CIRCUITPY_RM690B0_QSPI_D0)
#define LCD_D1_PIN          PIN_GPIO(CIRCUITPY_RM690B0_QSPI_D1)
#define LCD_D2_PIN          PIN_GPIO(CIRCUITPY_RM690B0_QSPI_D2)
#define LCD_D3_PIN          PIN_GPIO(CIRCUITPY_RM690B0_QSPI_D3)
#define LCD_RST_PIN         PIN_GPIO(CIRCUITPY_RM690B0_RESET)
#define LCD_PWR_PIN         PIN_GPIO(CIRCUITPY_RM690B0_POWER)
#define LCD_PWR_ON_LEVEL    (CIRCUITPY_RM690B0_POWER_ON_LEVEL)

#define LCD_H_RES           (CIRCUITPY_RM690B0_WIDTH)
#define LCD_V_RES           (CIRCUITPY_RM690B0_HEIGHT)
#define LCD_BIT_PER_PIXEL   (CIRCUITPY_RM690B0_BITS_PER_PIXEL)
#define LCD_USE_QSPI        (CIRCUITPY_RM690B0_USE_QSPI)
#define LCD_PIXEL_CLOCK_HZ  (CIRCUITPY_RM690B0_PIXEL_CLOCK_HZ)

// ============================================================================
// Panel constants
// ============================================================================

#define RGB565_SWAP_GB(c) (__builtin_bswap16(c))

#define RM690B0_OPCODE_WRITE_CMD   (0x02U)

#define RM690B0_PANEL_WIDTH          LCD_H_RES
#define RM690B0_PANEL_HEIGHT         LCD_V_RES
#define RM690B0_X_GAP                (CIRCUITPY_RM690B0_X_GAP)
#define RM690B0_Y_GAP                (CIRCUITPY_RM690B0_Y_GAP)
#define RM690B0_MAX_CHUNK_ROWS       (32)
#define RM690B0_MAX_CHUNK_PIXELS     (LCD_H_RES * RM690B0_MAX_CHUNK_ROWS)
#define RM690B0_MAX_DIAMETER         ((RM690B0_PANEL_WIDTH * 2) + 1)
#define RM690B0_PANEL_IO_QUEUE_DEPTH (16)

#define RM690B0_PENDING_BUFFER_FRAMEBUFFER   (0xFF)
#define RM690B0_PENDING_BUFFER_ALLOC         (0xFE)
#define RM690B0_PENDING_BUFFER_TEMP          (0xFD)

// Built-in font identifiers (must match shared-bindings docs)
#define RM690B0_FONT_8x8_MONO       (0)
#define RM690B0_FONT_16x16_MONO     (1)
#define RM690B0_FONT_16x24_MONO     (2)
#define RM690B0_FONT_24x24_MONO     (3)
#define RM690B0_FONT_24x32_MONO     (4)
#define RM690B0_FONT_32x32_MONO     (5)
#define RM690B0_FONT_32x48_MONO     (6)

// Macro to check if display is initialized
#define CHECK_INITIALIZED() \
    do { \
        if (!self->initialized) { \
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Display not initialized. Call init_display() first")); \
            return; \
        } \
    } while (0)

// ============================================================================
// Internal structs
// ============================================================================

typedef struct {
    mp_int_t top;
    mp_int_t row_count;
    int16_t *left;
    int16_t *right;
} rm690b0_span_accumulator_t;

typedef struct {
    size_t head;
    size_t tail;
    size_t count;
    uint8_t ids[RM690B0_PANEL_IO_QUEUE_DEPTH];
} rm690b0_dma_pending_list_t;

#define RM690B0_MAX_DIRTY_RECTS  (8)

typedef struct {
    mp_int_t x, y, w, h;
} rm690b0_dirty_rect_t;

typedef struct rm690b0_impl {
    esp_lcd_panel_io_handle_t io_handle;
    esp_lcd_panel_handle_t panel_handle;
    bool bus_initialized;
    uint16_t *chunk_buffers[2];
    size_t chunk_buffer_pixels;
    uint16_t *framebuffer;
    size_t framebuffer_pixels;
    uint16_t *framebuffer_front;
    bool double_buffered;
    size_t dirty_count;
    bool dirty_merged_valid;
    mp_int_t dirty_merged_x, dirty_merged_y, dirty_merged_w, dirty_merged_h;
    rm690b0_dirty_rect_t dirty_rects[RM690B0_MAX_DIRTY_RECTS];
    SemaphoreHandle_t transfer_done_sem;
    bool dma_buffer_in_use[2];
    bool dma_alloc_buffer_in_use;
    size_t dma_inflight;
    rm690b0_dma_pending_list_t dma_pending;
    int16_t *circle_span_cache;
    size_t circle_span_capacity;
} rm690b0_impl_t;

typedef struct {
    rm690b0_rm690b0_obj_t *self;
    rm690b0_impl_t *impl;
    mp_int_t origin_x;
    mp_int_t origin_y;
    mp_int_t clip_x;
    mp_int_t clip_y;
    mp_int_t clip_w;
    mp_int_t clip_h;
    bool rotation_zero;
} rm690b0_jpeg_draw_ctx_t;

// ============================================================================
// Extern globals (defined in RM690B0.c)
// ============================================================================

extern const char *TAG;
extern portMUX_TYPE rm690b0_spinlock;
extern rm690b0_rm690b0_obj_t *rm690b0_singleton;

// ============================================================================
// Inline helpers
// ============================================================================

static inline void rm690b0_dma_pending_init(rm690b0_dma_pending_list_t *list) {
    list->head = 0;
    list->tail = 0;
    list->count = 0;
}

static inline void rm690b0_dma_pending_push(rm690b0_dma_pending_list_t *list, uint8_t id) {
    portENTER_CRITICAL(&rm690b0_spinlock);
    list->ids[list->tail] = id;
    list->tail = (list->tail + 1) % RM690B0_PANEL_IO_QUEUE_DEPTH;
    list->count++;
    portEXIT_CRITICAL(&rm690b0_spinlock);
}

static inline uint8_t rm690b0_dma_pending_pop(rm690b0_dma_pending_list_t *list) {
    portENTER_CRITICAL(&rm690b0_spinlock);
    uint8_t id = list->ids[list->head];
    list->head = (list->head + 1) % RM690B0_PANEL_IO_QUEUE_DEPTH;
    list->count--;
    portEXIT_CRITICAL(&rm690b0_spinlock);
    return id;
}

static inline void rm690b0_wait_for_dma_completion(rm690b0_impl_t *impl) {
    if (impl->dma_inflight == 0) {
        return;
    }

    if (impl->transfer_done_sem) {
        if (xSemaphoreTake(impl->transfer_done_sem, pdMS_TO_TICKS(1000)) != pdTRUE) {
            ESP_LOGE("RM690B0", "DMA wait timeout! Halting to prevent memory corruption.");
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("DMA transfer timed out - hardware requires reset"));
            return;
        }
    } else {
        esp_rom_delay_us(50);
    }

    if (impl->dma_inflight > 0) {
        impl->dma_inflight--;
    }

    if (impl->dma_pending.count > 0) {
        uint8_t id = rm690b0_dma_pending_pop(&impl->dma_pending);
        if (id < 2) {
            impl->dma_buffer_in_use[id] = false;
        } else if (id == RM690B0_PENDING_BUFFER_ALLOC) {
            impl->dma_alloc_buffer_in_use = false;
        }
    }
}

static inline void rm690b0_wait_for_dma_slot(rm690b0_impl_t *impl) {
    while (impl->dma_inflight >= RM690B0_PANEL_IO_QUEUE_DEPTH) {
        rm690b0_wait_for_dma_completion(impl);
    }
}

static inline void rm690b0_wait_for_all_dma(rm690b0_impl_t *impl) {
    while (impl->dma_inflight > 0) {
        rm690b0_wait_for_dma_completion(impl);
    }
}

static inline void rm690b0_fill_span_fast(uint16_t *dest, size_t span_width, uint16_t color) {
    if (span_width == 0) {
        return;
    }

    if (((uintptr_t)dest & 0x2) != 0) {
        *dest++ = color;
        span_width--;
        if (span_width == 0) {
            return;
        }
    }

    uint32_t color_word = ((uint32_t)color << 16) | color;
    uint32_t *word_ptr = (uint32_t *)(uintptr_t)dest;
    size_t word_count = span_width / 2;

    while (word_count >= 4) {
        word_ptr[0] = color_word;
        word_ptr[1] = color_word;
        word_ptr[2] = color_word;
        word_ptr[3] = color_word;
        word_ptr += 4;
        word_count -= 4;
    }
    while (word_count > 0) {
        *word_ptr++ = color_word;
        word_count--;
    }

    if (span_width & 1) {
        *((uint16_t *)word_ptr) = color;
    }
}

static inline bool map_rect_for_rotation(const rm690b0_rm690b0_obj_t *self,
    mp_int_t *x, mp_int_t *y,
    mp_int_t *width, mp_int_t *height) {
    mp_int_t rx = *x;
    mp_int_t ry = *y;
    mp_int_t rw = *width;
    mp_int_t rh = *height;

    switch (self->rotation) {
        case 0:
            break;
        case 90:
            *x = RM690B0_PANEL_WIDTH - (ry + rh);
            *y = rx;
            *width = rh;
            *height = rw;
            break;
        case 180:
            *x = RM690B0_PANEL_WIDTH - (rx + rw);
            *y = RM690B0_PANEL_HEIGHT - (ry + rh);
            break;
        case 270:
            *x = ry;
            *y = RM690B0_PANEL_HEIGHT - (rx + rw);
            *width = rh;
            *height = rw;
            break;
        default:
            return false;
    }

    if (*width <= 0 || *height <= 0) {
        return false;
    }
    return true;
}

static inline bool check_bitmap_size(size_t width, size_t height, size_t *out_bytes) {
    uint64_t pixels = (uint64_t)width * (uint64_t)height;
    uint64_t bytes = pixels * sizeof(uint16_t);

    if (bytes > SIZE_MAX) {
        return false;
    }

    *out_bytes = (size_t)bytes;
    return true;
}

static inline bool clip_logical_rect(const rm690b0_rm690b0_obj_t *self,
    mp_int_t *x, mp_int_t *y,
    mp_int_t *width, mp_int_t *height) {
    if (*width <= 0 || *height <= 0) {
        return false;
    }

    mp_int_t x0 = *x;
    mp_int_t y0 = *y;
    mp_int_t x1 = x0 + *width;
    mp_int_t y1 = y0 + *height;

    if (x1 <= 0 || y1 <= 0 || x0 >= self->width || y0 >= self->height) {
        return false;
    }

    if (x0 < 0) {
        x0 = 0;
    }
    if (y0 < 0) {
        y0 = 0;
    }
    if (x1 > self->width) {
        x1 = self->width;
    }
    if (y1 > self->height) {
        y1 = self->height;
    }

    mp_int_t new_width = x1 - x0;
    mp_int_t new_height = y1 - y0;
    if (new_width <= 0 || new_height <= 0) {
        return false;
    }

    *x = x0;
    *y = y0;
    *width = new_width;
    *height = new_height;
    return true;
}

static inline mp_int_t clamp_int(mp_int_t v, mp_int_t lo, mp_int_t hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static inline bool rm690b0_prepare_draw(const rm690b0_rm690b0_obj_t *self,
    mp_int_t *x, mp_int_t *y, mp_int_t *w, mp_int_t *h) {
    return clip_logical_rect(self, x, y, w, h) &&
           map_rect_for_rotation(self, x, y, w, h);
}

static inline void rm690b0_map_point(const rm690b0_rm690b0_obj_t *self,
    mp_int_t logical_x, mp_int_t logical_y,
    mp_int_t *phys_x, mp_int_t *phys_y) {
    switch (self->rotation) {
        case 0:
            *phys_x = logical_x;
            *phys_y = logical_y;
            break;
        case 90:
            *phys_x = RM690B0_PANEL_WIDTH - logical_y - 1;
            *phys_y = logical_x;
            break;
        case 180:
            *phys_x = RM690B0_PANEL_WIDTH - logical_x - 1;
            *phys_y = RM690B0_PANEL_HEIGHT - logical_y - 1;
            break;
        case 270:
            *phys_x = logical_y;
            *phys_y = RM690B0_PANEL_HEIGHT - logical_x - 1;
            break;
        default:
            *phys_x = logical_x;
            *phys_y = logical_y;
            break;
    }
}

static inline void rm690b0_write_pixel_rotated(
    rm690b0_rm690b0_obj_t *self,
    rm690b0_impl_t *impl,
    mp_int_t logical_x, mp_int_t logical_y,
    uint16_t color) {

    mp_int_t phys_x, phys_y;
    rm690b0_map_point(self, logical_x, logical_y, &phys_x, &phys_y);

    if (phys_x < 0 || phys_x >= RM690B0_PANEL_WIDTH ||
        phys_y < 0 || phys_y >= RM690B0_PANEL_HEIGHT) {
        return;
    }

    impl->framebuffer[phys_y * RM690B0_PANEL_WIDTH + phys_x] = color;
}

// ============================================================================
// Non-inline cross-file function declarations (defined in RM690B0.c)
// ============================================================================

void mark_dirty_region(rm690b0_impl_t *impl, mp_int_t x, mp_int_t y, mp_int_t w, mp_int_t h);
esp_err_t rm690b0_flush_region(rm690b0_rm690b0_obj_t *self,
    mp_int_t x, mp_int_t y, mp_int_t width, mp_int_t height);
static inline void rm690b0_fill_rect_framebuffer(rm690b0_impl_t *impl,
    mp_int_t bx, mp_int_t by, mp_int_t bw, mp_int_t bh, uint16_t swapped_color) {

    uint16_t *base_ptr = impl->framebuffer + (size_t)by * RM690B0_PANEL_WIDTH + bx;

    rm690b0_fill_span_fast(base_ptr, (size_t)bw, swapped_color);

    mp_int_t filled_rows = 1;
    size_t row_bytes = (size_t)bw * sizeof(uint16_t);
    size_t fb_stride = RM690B0_PANEL_WIDTH;

    while (filled_rows < bh) {
        mp_int_t rows_to_copy = filled_rows;
        if (filled_rows + rows_to_copy > bh) {
            rows_to_copy = bh - filled_rows;
        }

        for (mp_int_t i = 0; i < rows_to_copy; i++) {
            uint16_t *src_row = base_ptr + (size_t)i * fb_stride;
            uint16_t *dest_row = base_ptr + (size_t)(filled_rows + i) * fb_stride;
            memcpy(dest_row, src_row, row_bytes);
        }

        filled_rows += rows_to_copy;
    }
}

void rm690b0_fill_color_direct(rm690b0_rm690b0_obj_t *self, uint16_t color);
esp_err_t rm690b0_fill_rect_direct_fullwidth(rm690b0_rm690b0_obj_t *self,
    mp_int_t start_y, mp_int_t rows, uint16_t swapped_color);
bool rm690b0_on_color_trans_done(esp_lcd_panel_io_handle_t panel_io,
    esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
int16_t *rm690b0_acquire_span_cache(rm690b0_impl_t *impl, size_t needed_rows);
void rm690b0_span_update(rm690b0_span_accumulator_t *acc, mp_int_t row_y, mp_int_t x_val);
bool expand_even_region(mp_int_t *x, mp_int_t *y, mp_int_t *width, mp_int_t *height);

// ============================================================================
// rm690b0_finalize_draw — inline, depends on mark_dirty_region + flush_region
// ============================================================================

static inline esp_err_t rm690b0_finalize_draw(
    rm690b0_rm690b0_obj_t *self, rm690b0_impl_t *impl,
    mp_int_t phys_x, mp_int_t phys_y, mp_int_t phys_w, mp_int_t phys_h) {
    mark_dirty_region(impl, phys_x, phys_y, phys_w, phys_h);
    if (!impl->double_buffered) {
        return rm690b0_flush_region(self, phys_x, phys_y, phys_w, phys_h);
    }
    return ESP_OK;
}
