// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "py/obj.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"

typedef struct {
    rm690b0_lvgl_widget_obj_t base;
    void *points; // lv_point_t *
    size_t point_count;
} rm690b0_lvgl_line_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_line_type;

void common_hal_rm690b0_lvgl_line_construct(rm690b0_lvgl_line_obj_t *self);
void common_hal_rm690b0_lvgl_line_deinit(rm690b0_lvgl_line_obj_t *self);

void common_hal_rm690b0_lvgl_line_set_points(rm690b0_lvgl_line_obj_t *self, const mp_int_t *coords, size_t coord_count);
void common_hal_rm690b0_lvgl_line_set_y_invert(rm690b0_lvgl_line_obj_t *self, bool invert);
bool common_hal_rm690b0_lvgl_line_get_y_invert(rm690b0_lvgl_line_obj_t *self);

void common_hal_rm690b0_lvgl_line_set_line_width(rm690b0_lvgl_line_obj_t *self, mp_int_t width);
mp_int_t common_hal_rm690b0_lvgl_line_get_line_width(rm690b0_lvgl_line_obj_t *self);

void common_hal_rm690b0_lvgl_line_set_line_color(rm690b0_lvgl_line_obj_t *self, uint32_t color);
uint32_t common_hal_rm690b0_lvgl_line_get_line_color(rm690b0_lvgl_line_obj_t *self);

void common_hal_rm690b0_lvgl_line_set_rounded(rm690b0_lvgl_line_obj_t *self, bool rounded);
bool common_hal_rm690b0_lvgl_line_get_rounded(rm690b0_lvgl_line_obj_t *self);
