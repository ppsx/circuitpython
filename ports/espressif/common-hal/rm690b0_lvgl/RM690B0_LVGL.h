// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include "shared-bindings/rm690b0_lvgl/RM690B0_LVGL.h"
// #include "lvgl.h"  // TODO: Enable when LVGL is available
// #include "esp_lvgl_port.h"  // TODO: Enable when LVGL port is available
#include "esp-idf/components/esp_lcd/include/esp_lcd_panel_ops.h"
#include "esp_timer.h"

// Implementation structure for ESP32-specific LVGL integration
typedef struct {
    // LVGL display and input device drivers (stubbed for now)
    void *lvgl_disp;   // lv_disp_t when LVGL available
    void *lvgl_indev;  // lv_indev_t when LVGL available

    // RM690B0 panel handle (obtained from rm690b0 module)
    esp_lcd_panel_handle_t panel_handle;

    // LVGL port configuration (stubbed)
    void *port_cfg;  // lvgl_port_cfg_t when available

    // Display driver buffers (stubbed)
    void *disp_buf;  // lv_disp_draw_buf_t when available
    void *buf1;      // lv_color_t* when available
    void *buf2;      // lv_color_t* when available

    // Double buffered DMA to prevent data corruption during rapid updates
    void *dma_buffers[2];
    uint8_t dma_buf_idx;

    // Touch controller I2C handle
    void *touch_i2c_handle;

    // Tick timer
    esp_timer_handle_t tick_timer;
    bool tick_timer_started;

    // Display dimensions
    uint16_t hor_res;
    uint16_t ver_res;
} rm690b0_lvgl_impl_t;

// Additional common-hal function declarations (if needed)
void common_hal_rm690b0_lvgl_rm690b0_lvgl_construct(rm690b0_lvgl_rm690b0_lvgl_obj_t *self);
void common_hal_rm690b0_lvgl_rm690b0_lvgl_deinit(rm690b0_lvgl_rm690b0_lvgl_obj_t *self);
void common_hal_rm690b0_lvgl_init_display(rm690b0_lvgl_rm690b0_lvgl_obj_t *self);
void common_hal_rm690b0_lvgl_init_touch(rm690b0_lvgl_rm690b0_lvgl_obj_t *self, mp_obj_t i2c);
void common_hal_rm690b0_lvgl_task_handler(rm690b0_lvgl_rm690b0_lvgl_obj_t *self);
void common_hal_rm690b0_lvgl_deinit_all(void);
