// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "shared-bindings/rm690b0_lvgl/RM690B0_LVGL.h"
#include "common-hal/rm690b0_lvgl/RM690B0_LVGL.h"
#include "shared-bindings/rm690b0/RM690B0.h"
#include "py/runtime.h"
#include "py/mperrno.h"

#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "components/driver/i2c/include/driver/i2c.h"
#include "common-hal/busio/I2C.h"
#include "shared-bindings/busio/I2C.h"
#include "lvgl.h"

#include <string.h>
#include <stdlib.h>

// Display configuration from board settings
#ifndef CIRCUITPY_RM690B0_WIDTH
#define CIRCUITPY_RM690B0_WIDTH (600)
#endif

#ifndef CIRCUITPY_RM690B0_HEIGHT
#define CIRCUITPY_RM690B0_HEIGHT (450)
#endif

// Touch controller I2C configuration
#ifndef CIRCUITPY_TOUCH_I2C_SDA
#define CIRCUITPY_TOUCH_I2C_SDA (6)
#endif

#ifndef CIRCUITPY_TOUCH_I2C_SCL
#define CIRCUITPY_TOUCH_I2C_SCL (7)
#endif

#ifndef CIRCUITPY_TOUCH_I2C_FREQ
#define CIRCUITPY_TOUCH_I2C_FREQ (400000)
#endif

#define LVGL_TICK_PERIOD_MS     2
#define LVGL_BUFFER_ROWS        10  // Reduced to 10 rows to fit in internal RAM (10 * 600 = 6000 pixels)

// RM690B0 color swap macro (synced with rm690b0 driver)
#define RGB565_SWAP_GB(c) (__builtin_bswap16(c))

static const char *TAG = "rm690b0_lvgl";

// Singleton instance for cleanup
static rm690b0_lvgl_rm690b0_lvgl_obj_t *rm690b0_lvgl_singleton = NULL;

// External reference to RM690B0 display instance (from rm690b0 module)
extern esp_lcd_panel_handle_t rm690b0_get_panel_handle(void);

// LVGL-specific static data
static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
static bool lvgl_initialized = false;

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
    rm690b0_lvgl_rm690b0_lvgl_obj_t *self = (rm690b0_lvgl_rm690b0_lvgl_obj_t *)drv->user_data;
    rm690b0_lvgl_impl_t *impl = (rm690b0_lvgl_impl_t *)self->impl;

    // Expand coordinates to satisfy even-alignment requirements
    int32_t x1 = area->x1;
    int32_t x2 = area->x2;
    int32_t y1 = area->y1;
    int32_t y2 = area->y2;

    if (x1 % 2 != 0) x1--;
    if ((x2 + 1) % 2 != 0) x2++;
    if (y1 % 2 != 0) y1--;
    if ((y2 + 1) % 2 != 0) y2++;

    // Clamp to display bounds
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= impl->hor_res) x2 = impl->hor_res - 1;
    if (y2 >= impl->ver_res) y2 = impl->ver_res - 1;

    int32_t w = x2 - x1 + 1;
    int32_t h = y2 - y1 + 1;
    int32_t src_w = area->x2 - area->x1 + 1;
    int32_t src_h = area->y2 - area->y1 + 1;

    // Use current DMA buffer (cycling between 2 buffers)
    uint16_t *dest_buf = (uint16_t *)impl->dma_buffers[impl->dma_buf_idx];
    uint16_t *src_buf = (uint16_t *)color_p;

    // Advance to next buffer for next flush
    impl->dma_buf_idx = (impl->dma_buf_idx + 1) % 2;

    if (dest_buf == NULL) {
        // Fallback if DMA buffer allocation failed (should not happen if init succeeded)
        size_t pixel_count = src_w * src_h;
        for (size_t i = 0; i < pixel_count; i++) {
            src_buf[i] = RGB565_SWAP_GB(src_buf[i]);
        }
        esp_lcd_panel_draw_bitmap(impl->panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_p);
        lv_disp_flush_ready(drv);
        return;
    }

    // Copy to DMA buffer with padding and color conversion
    for (int32_t r = 0; r < h; r++) {
        int32_t cur_y = y1 + r;
        // Map to source Y (clamped)
        int32_t src_r = cur_y - area->y1;
        if (src_r < 0) src_r = 0;
        if (src_r >= src_h) src_r = src_h - 1;

        uint16_t *dest_row = dest_buf + (r * w);
        uint16_t *src_row = src_buf + (src_r * src_w);

        for (int32_t c = 0; c < w; c++) {
            int32_t cur_x = x1 + c;
            // Map to source X (clamped)
            int32_t src_c = cur_x - area->x1;
            if (src_c < 0) src_c = 0;
            if (src_c >= src_w) src_c = src_w - 1;

            dest_row[c] = RGB565_SWAP_GB(src_row[src_c]);
        }
    }

    esp_lcd_panel_draw_bitmap(impl->panel_handle, x1, y1, x2 + 1, y2 + 1, dest_buf);
    lv_disp_flush_ready(drv);
}

static void lvgl_tick_task(void *arg) {
    (void)arg;
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

// FT6336U Touch Controller Defines
#define FT6336U_I2C_ADDRESS         0x38
#define FT6336U_REG_TD_STATUS       0x02
#define FT6336U_REG_P1_XH           0x03
#define FT6336U_REG_P1_XL           0x04
#define FT6336U_REG_P1_YH           0x05
#define FT6336U_REG_P1_YL           0x06

// LVGL-specific static data for touch
static lv_indev_drv_t indev_drv;

typedef struct {
    mp_obj_t i2c_bus_obj;
} touch_user_data_t;

static void lvgl_touch_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    touch_user_data_t *user_data = (touch_user_data_t *)drv->user_data;
    busio_i2c_obj_t *i2c = MP_OBJ_TO_PTR(user_data->i2c_bus_obj);

    uint8_t read_buf[5]; // TD_STATUS, P1_XH, P1_XL, P1_YH, P1_YL
    uint8_t reg_addr = FT6336U_REG_TD_STATUS;
    data->state = LV_INDEV_STATE_REL;

    // CRITICAL FIX: Early exit if image subsystem not initialized
    // Prevents race condition during first image render when LVGL's
    // image cache is being initialized. Touch interrupts during this
    // initialization can cause memory corruption and board resets.
    if (rm690b0_lvgl_singleton != NULL &&
        !rm690b0_lvgl_singleton->image_subsystem_initialized) {
        return;  // Defer touch processing until image subsystem is ready
    }

    if (!common_hal_busio_i2c_try_lock(i2c)) {
        return;
    }

    // Set the register address to read from, then read the data
    uint8_t write_result = common_hal_busio_i2c_write(i2c, FT6336U_I2C_ADDRESS, &reg_addr, 1);
    uint8_t read_result = 0xFF;
    if (write_result == 0) {
        read_result = common_hal_busio_i2c_read(i2c, FT6336U_I2C_ADDRESS, read_buf, 5);
    }
    common_hal_busio_i2c_unlock(i2c);

    if (write_result == 0 && read_result == 0) {
        uint8_t touch_count = read_buf[0];
        if (touch_count > 0 && touch_count < 3) {
            // Raw touch coordinates (portrait orientation: 0-449 x, 0-599 y)
            uint16_t touch_x = ((read_buf[1] & 0x0F) << 8) | read_buf[2];
            uint16_t touch_y = ((read_buf[3] & 0x0F) << 8) | read_buf[4];

            // Transform from portrait touch (450x600) to landscape display (600x450)
            // Display rotation 0 (landscape): touch is rotated 90° counter-clockwise
            // Transform: display_x = (599 - touch_y), display_y = touch_x
            data->point.x = 599 - touch_y;
            data->point.y = touch_x;
            data->state = LV_INDEV_STATE_PR;
        }
    }
}

void common_hal_rm690b0_lvgl_rm690b0_lvgl_construct(rm690b0_lvgl_rm690b0_lvgl_obj_t *self) {
    self->display_initialized = false;
    self->touch_initialized = false;
    self->image_subsystem_initialized = false;  // Not initialized until first image render
    self->width = CIRCUITPY_RM690B0_WIDTH;
    self->height = CIRCUITPY_RM690B0_HEIGHT;
    // Ensure touch I2C reference starts as None so GC tracking is correct
    self->touch_i2c_obj = mp_const_none;

    // Allocate implementation structure
    self->impl = m_malloc(sizeof(rm690b0_lvgl_impl_t));
    rm690b0_lvgl_impl_t *impl = (rm690b0_lvgl_impl_t *)self->impl;

    memset(impl, 0, sizeof(rm690b0_lvgl_impl_t));
    impl->hor_res = CIRCUITPY_RM690B0_WIDTH;
    impl->ver_res = CIRCUITPY_RM690B0_HEIGHT;

    // Store singleton reference
    rm690b0_lvgl_singleton = self;

    ESP_LOGI(TAG, "RM690B0_LVGL constructed (LVGL stub - full implementation pending)");
}

void common_hal_rm690b0_lvgl_rm690b0_lvgl_deinit(rm690b0_lvgl_rm690b0_lvgl_obj_t *self) {
    if (self->impl == NULL) {
        return;
    }

    rm690b0_lvgl_impl_t *impl = (rm690b0_lvgl_impl_t *)self->impl;

    // Stop and delete tick timer
    if (impl->tick_timer_started && impl->tick_timer != NULL) {
        esp_timer_stop(impl->tick_timer);
        esp_timer_delete(impl->tick_timer);
        impl->tick_timer = NULL;
        impl->tick_timer_started = false;
    }

    // Free display buffers
    if (impl->buf1 != NULL) {
        heap_caps_free(impl->buf1);
        impl->buf1 = NULL;
    }

    if (impl->buf2 != NULL) {
        heap_caps_free(impl->buf2);
        impl->buf2 = NULL;
    }

    if (impl->dma_buffers[0] != NULL) {
        heap_caps_free(impl->dma_buffers[0]);
        impl->dma_buffers[0] = NULL;
    }
    if (impl->dma_buffers[1] != NULL) {
        heap_caps_free(impl->dma_buffers[1]);
        impl->dma_buffers[1] = NULL;
    }

    if (impl->lvgl_indev != NULL) {
        lv_indev_t *indev = (lv_indev_t *)impl->lvgl_indev;
        if (indev->driver && indev->driver->user_data) {
            m_free(indev->driver->user_data);
            indev->driver->user_data = NULL;
        }
    }

    // Free implementation structure
    m_free(self->impl);
    self->impl = NULL;

    self->display_initialized = false;
    self->touch_initialized = false;
    // Release strong reference to touch I2C object so it can be GC'ed safely
    self->touch_i2c_obj = mp_const_none;

    if (rm690b0_lvgl_singleton == self) {
        if (lvgl_initialized) {
            lv_deinit();
            lvgl_initialized = false;
        }
        rm690b0_lvgl_singleton = NULL;
    }

    ESP_LOGI(TAG, "RM690B0_LVGL deinitialized");
}

void common_hal_rm690b0_lvgl_init_display(rm690b0_lvgl_rm690b0_lvgl_obj_t *self) {
    if (self->display_initialized) {
        ESP_LOGW(TAG, "Display already initialized");
        return;
    }

    rm690b0_lvgl_impl_t *impl = (rm690b0_lvgl_impl_t *)self->impl;

    if (!lvgl_initialized) {
        lv_init();
        lvgl_initialized = true;
    }

    // Get panel handle from RM690B0 module
    impl->panel_handle = rm690b0_get_panel_handle();
    if (impl->panel_handle == NULL) {
        mp_raise_msg(&mp_type_RuntimeError,
            MP_ERROR_TEXT("RM690B0 display not initialized. Initialize rm690b0.RM690B0 first."));
        return;
    }

    // Allocate LVGL draw buffers
    size_t buf_size = impl->hor_res * LVGL_BUFFER_ROWS;
    impl->buf1 = heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (impl->buf1 == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to allocate LVGL buffer 1"));
        return;
    }

    impl->buf2 = heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (impl->buf2 == NULL) {
        heap_caps_free(impl->buf1);
        impl->buf1 = NULL;
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to allocate LVGL buffer 2"));
        return;
    }
    ESP_LOGI(TAG, "Allocated LVGL draw buffers: 2 x %zu bytes in PSRAM", buf_size * sizeof(lv_color_t));

    // Allocate DMA buffers (internal RAM) for double-buffered transfers
    // Ensure size covers alignment padding (max 2px extra width/height)
    size_t dma_buf_pixels = (impl->hor_res + 4) * (LVGL_BUFFER_ROWS + 2);

    // Allocate in internal RAM (MALLOC_CAP_DMA) for stability
    // We reduced buffer size so 2 buffers (approx 26KB) easily fit in SRAM
    impl->dma_buffers[0] = heap_caps_malloc(dma_buf_pixels * sizeof(uint16_t), MALLOC_CAP_DMA);
    impl->dma_buffers[1] = heap_caps_malloc(dma_buf_pixels * sizeof(uint16_t), MALLOC_CAP_DMA);

    if (impl->dma_buffers[0] == NULL || impl->dma_buffers[1] == NULL) {
        if (impl->dma_buffers[0]) heap_caps_free(impl->dma_buffers[0]);
        if (impl->dma_buffers[1]) heap_caps_free(impl->dma_buffers[1]);
        heap_caps_free(impl->buf1);
        heap_caps_free(impl->buf2);
        impl->buf1 = NULL;
        impl->buf2 = NULL;
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to allocate DMA buffers"));
        return;
    }
    impl->dma_buf_idx = 0;
    ESP_LOGI(TAG, "Allocated 2 DMA transfer buffers: %zu bytes each", dma_buf_pixels * sizeof(uint16_t));

    // Initialize LVGL draw buffer and display driver
    lv_disp_draw_buf_init(&disp_buf, impl->buf1, impl->buf2, buf_size);

    ESP_LOGI(TAG, "Initializing LVGL display driver");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = impl->hor_res;
    disp_drv.ver_res = impl->ver_res;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = self;
    impl->lvgl_disp = lv_disp_drv_register(&disp_drv);

    // Setup tick timer
    const esp_timer_create_args_t tick_timer_args = {
        .callback = &lvgl_tick_task,
        .name = "lvgl_tick"
    };
    esp_timer_create(&tick_timer_args, &impl->tick_timer);
    esp_timer_start_periodic(impl->tick_timer, LVGL_TICK_PERIOD_MS * 1000);
    impl->tick_timer_started = true;
    ESP_LOGI(TAG, "LVGL tick timer started");

    self->display_initialized = true;
    ESP_LOGI(TAG, "LVGL display initialization complete");

    // Automatically initialize rendering subsystem to prevent touch race condition
    // This creates a temporary off-screen image and renders it to fully initialize
    // LVGL's image cache and rendering pipeline before touch callbacks can fire
    ESP_LOGI(TAG, "Initializing rendering subsystem for touch safety...");
    common_hal_rm690b0_lvgl_init_rendering(self);
}

void common_hal_rm690b0_lvgl_init_touch(rm690b0_lvgl_rm690b0_lvgl_obj_t *self, mp_obj_t i2c_obj) {
    if (!self->display_initialized) {
        mp_raise_msg(&mp_type_RuntimeError,
            MP_ERROR_TEXT("Display must be initialized before touch. Call init_display() first."));
        return;
    }

    // Verify rendering subsystem was initialized by init_display()
    // This should always be true since init_display() now calls init_rendering() automatically
    if (!self->image_subsystem_initialized) {
        ESP_LOGW(TAG, "Rendering subsystem not initialized - this should not happen!");
        ESP_LOGW(TAG, "Attempting to initialize now...");
        common_hal_rm690b0_lvgl_init_rendering(self);
    }
    if (self->touch_initialized) {
        ESP_LOGW(TAG, "Touch already initialized");
        return;
    }
    rm690b0_lvgl_impl_t *impl = (rm690b0_lvgl_impl_t *)self->impl;

    // Keep a strong reference to the I2C object on the Python-side LVGL object
    // so that the Micropython GC will not collect it while touch is active.
    self->touch_i2c_obj = i2c_obj;

    touch_user_data_t *user_data = m_malloc(sizeof(touch_user_data_t));
    // Use the stashed reference to ensure we always point to the GC-rooted object
    user_data->i2c_bus_obj = self->touch_i2c_obj;

    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_touch_read_cb;
    indev_drv.user_data = user_data;
    impl->lvgl_indev = lv_indev_drv_register(&indev_drv);

    self->touch_initialized = true;
    ESP_LOGI(TAG, "LVGL touch initialization complete");
}

void common_hal_rm690b0_lvgl_task_handler(rm690b0_lvgl_rm690b0_lvgl_obj_t *self) {
    if (!self->display_initialized) {
        return;
    }

    lv_task_handler();
}

void common_hal_rm690b0_lvgl_deinit_all(void) {
    if (rm690b0_lvgl_singleton != NULL) {
        common_hal_rm690b0_lvgl_rm690b0_lvgl_deinit(rm690b0_lvgl_singleton);
    }
}

// Event handler for button clicks
static void button_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_PRESSED) {
        // Button pressed - make it darker
        lv_obj_set_style_bg_opa(btn, 200, 0);
        ESP_LOGI(TAG, "Button pressed");
    } else if (code == LV_EVENT_RELEASED) {
        // Button released - restore opacity
        lv_obj_set_style_bg_opa(btn, 255, 0);
        ESP_LOGI(TAG, "Button released");
    } else if (code == LV_EVENT_CLICKED) {
        // Button clicked - log it
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        ESP_LOGI(TAG, "Button clicked: %s", lv_label_get_text(label));
    }
}

void common_hal_rm690b0_lvgl_test_draw(rm690b0_lvgl_rm690b0_lvgl_obj_t *self) {
    if (!self->display_initialized) {
        mp_raise_msg(&mp_type_RuntimeError,
            MP_ERROR_TEXT("Display must be initialized. Call init_display() first."));
        return;
    }

    ESP_LOGI(TAG, "Drawing LVGL interactive test pattern");

    // Set screen background to light gray
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xE0E0E0), 0);

    // Create title label
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Touch Test - Tap Buttons");
    lv_obj_set_style_text_color(title, lv_color_black(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Create RED button (top-left)
    lv_obj_t *btn1 = lv_btn_create(scr);
    lv_obj_set_size(btn1, 180, 120);
    lv_obj_set_pos(btn1, 20, 50);
    lv_obj_set_style_bg_color(btn1, lv_color_hex(0xFF0000), 0);
    lv_obj_add_event_cb(btn1, button_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *label1 = lv_label_create(btn1);
    lv_label_set_text(label1, "RED\nBUTTON");
    lv_obj_set_style_text_color(label1, lv_color_white(), 0);
    lv_obj_center(label1);

    // Create GREEN button (top-right)
    lv_obj_t *btn2 = lv_btn_create(scr);
    lv_obj_set_size(btn2, 180, 120);
    lv_obj_set_pos(btn2, 400, 50);
    lv_obj_set_style_bg_color(btn2, lv_color_hex(0x00FF00), 0);
    lv_obj_add_event_cb(btn2, button_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *label2 = lv_label_create(btn2);
    lv_label_set_text(label2, "GREEN\nBUTTON");
    lv_obj_set_style_text_color(label2, lv_color_black(), 0);
    lv_obj_center(label2);

    // Create BLUE button (bottom-left)
    lv_obj_t *btn3 = lv_btn_create(scr);
    lv_obj_set_size(btn3, 180, 120);
    lv_obj_set_pos(btn3, 20, 280);
    lv_obj_set_style_bg_color(btn3, lv_color_hex(0x0000FF), 0);
    lv_obj_add_event_cb(btn3, button_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *label3 = lv_label_create(btn3);
    lv_label_set_text(label3, "BLUE\nBUTTON");
    lv_obj_set_style_text_color(label3, lv_color_white(), 0);
    lv_obj_center(label3);

    // Create YELLOW button (bottom-right)
    lv_obj_t *btn4 = lv_btn_create(scr);
    lv_obj_set_size(btn4, 180, 120);
    lv_obj_set_pos(btn4, 400, 280);
    lv_obj_set_style_bg_color(btn4, lv_color_hex(0xFFFF00), 0);
    lv_obj_add_event_cb(btn4, button_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *label4 = lv_label_create(btn4);
    lv_label_set_text(label4, "YELLOW\nBUTTON");
    lv_obj_set_style_text_color(label4, lv_color_black(), 0);
    lv_obj_center(label4);

    // Create instruction label
    lv_obj_t *info = lv_label_create(scr);
    lv_label_set_text(info, "Watch console for touch events");
    lv_obj_set_style_text_color(info, lv_color_hex(0x404040), 0);
    lv_obj_align(info, LV_ALIGN_BOTTOM_MID, 0, -10);

    // Force a redraw
    lv_refr_now(NULL);

    ESP_LOGI(TAG, "Interactive test pattern drawn - touch buttons to test");
}

mp_int_t common_hal_rm690b0_lvgl_get_width(const rm690b0_lvgl_rm690b0_lvgl_obj_t *self) {
    return self->width;
}

mp_int_t common_hal_rm690b0_lvgl_get_height(const rm690b0_lvgl_rm690b0_lvgl_obj_t *self) {
    return self->height;
}

void common_hal_rm690b0_lvgl_init_rendering(rm690b0_lvgl_rm690b0_lvgl_obj_t *self) {
    // Helper function to initialize LVGL rendering subsystem
    // before touch initialization. This prevents the race condition
    // where touch callbacks fire during first-time image cache setup.
    //
    // Usage:
    //   lvgl = rm690b0_lvgl.RM690B0_LVGL()
    //   lvgl.init_display()
    //   lvgl.init_rendering()  # Call this before init_touch()
    //   lvgl.init_touch(i2c)

    if (!self->display_initialized) {
        mp_raise_msg(&mp_type_RuntimeError,
            MP_ERROR_TEXT("Display must be initialized first. Call init_display() before init_rendering()."));
        return;
    }

    if (self->image_subsystem_initialized) {
        ESP_LOGW(TAG, "Image subsystem already initialized");
        return;
    }

    ESP_LOGI(TAG, "Preparing image subsystem for touch initialization...");

    // CRITICAL: Set flag BEFORE any lv_task_handler() calls
    // This prevents touch callbacks from firing during initialization
    self->image_subsystem_initialized = true;

    // Create a temporary off-screen image to initialize LVGL's image cache
    lv_obj_t *dummy_img = lv_img_create(lv_scr_act());
    if (dummy_img == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create dummy image"));
        return;
    }

    // Position off-screen so it's not visible
    lv_obj_set_pos(dummy_img, -1000, -1000);
    lv_obj_set_size(dummy_img, 1, 1);

    // Create minimal 1x1 image descriptor
    static lv_img_dsc_t dummy_dsc;
    static const uint8_t dummy_data[2] = {0x00, 0x00}; // 1 pixel RGB565

    dummy_dsc.header.always_zero = 0;
    dummy_dsc.header.w = 1;
    dummy_dsc.header.h = 1;
    dummy_dsc.data_size = 2;
    dummy_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
    dummy_dsc.data = dummy_data;

    // Set the dummy image source
    lv_img_set_src(dummy_img, &dummy_dsc);

    // Render 3 times to fully initialize image subsystem
    // First render: Allocates image cache structures
    // Second render: Completes DMA buffer setup
    // Third render: Confirms system stability
    for (int i = 0; i < 3; i++) {
        lv_task_handler();
        ESP_LOGD(TAG, "Image subsystem initialization render %d/3", i + 1);
    }

    // Clean up dummy image
    lv_obj_del(dummy_img);

    // One final render to process the deletion
    lv_task_handler();

    ESP_LOGI(TAG, "Image subsystem ready - safe to initialize touch");
}

void common_hal_rm690b0_lvgl_scroll_screen(rm690b0_lvgl_rm690b0_lvgl_obj_t *self, mp_int_t x, mp_int_t y, bool animated) {
    (void)self;
    lv_obj_t *scr = lv_scr_act();
    lv_obj_scroll_to(scr, (lv_coord_t)x, (lv_coord_t)y, animated ? LV_ANIM_ON : LV_ANIM_OFF);
}

mp_int_t common_hal_rm690b0_lvgl_get_scroll_y(rm690b0_lvgl_rm690b0_lvgl_obj_t *self) {
    (void)self;
    return lv_obj_get_scroll_y(lv_scr_act());
}

void common_hal_rm690b0_lvgl_set_theme_color(rm690b0_lvgl_rm690b0_lvgl_obj_t *self, uint32_t primary, uint32_t secondary, bool dark) {
    if (!self->display_initialized) {
        mp_raise_msg(&mp_type_RuntimeError,
            MP_ERROR_TEXT("Display must be initialized first. Call init_display() before set_theme_color()."));
        return;
    }

    rm690b0_lvgl_impl_t *impl = (rm690b0_lvgl_impl_t *)self->impl;
    lv_disp_t *disp = (lv_disp_t *)impl->lvgl_disp;

    // Reinitialize theme with new colors
    lv_theme_t *theme = lv_theme_default_init(
        disp,
        lv_color_hex(primary),
        lv_color_hex(secondary),
        dark,
        LV_FONT_DEFAULT
    );

    lv_disp_set_theme(disp, theme);

    // Force refresh to apply new theme
    lv_obj_invalidate(lv_scr_act());

    ESP_LOGI(TAG, "Theme updated: primary=0x%06lX, secondary=0x%06lX, dark=%d",
             (unsigned long)primary, (unsigned long)secondary, dark);
}
