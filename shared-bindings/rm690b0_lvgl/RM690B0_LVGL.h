// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "py/obj.h"

// Forward declaration for the rm690b0_lvgl object type
typedef struct {
    mp_obj_base_t base;
    bool display_initialized;
    bool touch_initialized;
    bool image_subsystem_initialized;  // Track if LVGL image subsystem is ready
    mp_int_t width;
    mp_int_t height;
    // Strong reference to touch I2C object to prevent GC from collecting it
    mp_obj_t touch_i2c_obj;
    // Port-specific implementation data will be added by common-hal
    void *impl;
} rm690b0_lvgl_rm690b0_lvgl_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_rm690b0_lvgl_type;

// Common HAL function declarations
void common_hal_rm690b0_lvgl_rm690b0_lvgl_construct(rm690b0_lvgl_rm690b0_lvgl_obj_t *self);
void common_hal_rm690b0_lvgl_rm690b0_lvgl_deinit(rm690b0_lvgl_rm690b0_lvgl_obj_t *self);
void common_hal_rm690b0_lvgl_init_display(rm690b0_lvgl_rm690b0_lvgl_obj_t *self);
void common_hal_rm690b0_lvgl_init_touch(rm690b0_lvgl_rm690b0_lvgl_obj_t *self, mp_obj_t i2c);
void common_hal_rm690b0_lvgl_task_handler(rm690b0_lvgl_rm690b0_lvgl_obj_t *self);
void common_hal_rm690b0_lvgl_test_draw(rm690b0_lvgl_rm690b0_lvgl_obj_t *self);
void common_hal_rm690b0_lvgl_deinit_all(void);
void common_hal_rm690b0_lvgl_scroll_screen(rm690b0_lvgl_rm690b0_lvgl_obj_t *self, mp_int_t x, mp_int_t y, bool animated);
mp_int_t common_hal_rm690b0_lvgl_get_scroll_y(rm690b0_lvgl_rm690b0_lvgl_obj_t *self);

// Get the display width and height
mp_int_t common_hal_rm690b0_lvgl_get_width(const rm690b0_lvgl_rm690b0_lvgl_obj_t *self);
mp_int_t common_hal_rm690b0_lvgl_get_height(const rm690b0_lvgl_rm690b0_lvgl_obj_t *self);

// Initialize rendering subsystem (workaround for touch race condition)
void common_hal_rm690b0_lvgl_init_rendering(rm690b0_lvgl_rm690b0_lvgl_obj_t *self);

// Set theme colors
void common_hal_rm690b0_lvgl_set_theme_color(rm690b0_lvgl_rm690b0_lvgl_obj_t *self, uint32_t primary, uint32_t secondary, bool dark);
