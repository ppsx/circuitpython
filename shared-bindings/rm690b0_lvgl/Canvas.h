// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "py/obj.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"

typedef struct {
    rm690b0_lvgl_widget_obj_t base;
    uint8_t *buffer;
    size_t buffer_size;
    mp_int_t color_format;
    mp_int_t buf_width;
    mp_int_t buf_height;
} rm690b0_lvgl_canvas_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_canvas_type;

void common_hal_rm690b0_lvgl_canvas_construct(rm690b0_lvgl_canvas_obj_t *self, mp_int_t width, mp_int_t height, mp_int_t color_format);
void common_hal_rm690b0_lvgl_canvas_deinit(rm690b0_lvgl_canvas_obj_t *self);

void common_hal_rm690b0_lvgl_canvas_fill_bg(rm690b0_lvgl_canvas_obj_t *self, uint32_t color, mp_int_t opacity);
void common_hal_rm690b0_lvgl_canvas_set_px(rm690b0_lvgl_canvas_obj_t *self, mp_int_t x, mp_int_t y, uint32_t color, mp_int_t opacity);
void common_hal_rm690b0_lvgl_canvas_draw_line(rm690b0_lvgl_canvas_obj_t *self, const mp_int_t *points, size_t point_count, uint32_t color, mp_int_t width);
