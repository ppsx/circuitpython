// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"

typedef struct {
    rm690b0_lvgl_widget_obj_t base;
} rm690b0_lvgl_chart_obj_t;

typedef struct {
    mp_obj_base_t base;
    rm690b0_lvgl_chart_obj_t *chart;
    void *series; // lv_chart_series_t *
} rm690b0_lvgl_chart_series_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_chart_type;
extern const mp_obj_type_t rm690b0_lvgl_chart_series_type;

void common_hal_rm690b0_lvgl_chart_construct(rm690b0_lvgl_chart_obj_t *self, mp_int_t chart_type);
void common_hal_rm690b0_lvgl_chart_set_type(rm690b0_lvgl_chart_obj_t *self, mp_int_t chart_type);
mp_int_t common_hal_rm690b0_lvgl_chart_get_type(rm690b0_lvgl_chart_obj_t *self);

void common_hal_rm690b0_lvgl_chart_set_point_count(rm690b0_lvgl_chart_obj_t *self, mp_int_t count);
mp_int_t common_hal_rm690b0_lvgl_chart_get_point_count(rm690b0_lvgl_chart_obj_t *self);

void common_hal_rm690b0_lvgl_chart_set_range(rm690b0_lvgl_chart_obj_t *self, mp_int_t axis, mp_int_t min_value, mp_int_t max_value);

mp_obj_t common_hal_rm690b0_lvgl_chart_add_series(rm690b0_lvgl_chart_obj_t *self, uint32_t color, mp_int_t axis);
void common_hal_rm690b0_lvgl_chart_refresh(rm690b0_lvgl_chart_obj_t *self);

void common_hal_rm690b0_lvgl_chart_series_set_points(rm690b0_lvgl_chart_series_obj_t *series, size_t value_count, const mp_int_t *values);
void common_hal_rm690b0_lvgl_chart_series_set_point(rm690b0_lvgl_chart_series_obj_t *series, mp_int_t index, mp_int_t value);
void common_hal_rm690b0_lvgl_chart_series_append(rm690b0_lvgl_chart_series_obj_t *series, mp_int_t value);
void common_hal_rm690b0_lvgl_chart_series_set_color(rm690b0_lvgl_chart_series_obj_t *series, uint32_t color);
