// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>

#include "py/obj.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"

typedef struct {
    rm690b0_lvgl_widget_obj_t base;
    void *scale;      // lv_meter_scale_t *
    void *indicator;  // lv_meter_indicator_t *
    mp_int_t min_value;
    mp_int_t max_value;
    mp_int_t angle_range;
    mp_int_t rotation;
} rm690b0_lvgl_scale_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_scale_type;

void common_hal_rm690b0_lvgl_scale_construct(rm690b0_lvgl_scale_obj_t *self, mp_int_t min_value, mp_int_t max_value);
void common_hal_rm690b0_lvgl_scale_deinit(rm690b0_lvgl_scale_obj_t *self);

void common_hal_rm690b0_lvgl_scale_set_range(rm690b0_lvgl_scale_obj_t *self, mp_int_t min_value, mp_int_t max_value);
void common_hal_rm690b0_lvgl_scale_set_angles(rm690b0_lvgl_scale_obj_t *self, mp_int_t angle_range, mp_int_t rotation);

void common_hal_rm690b0_lvgl_scale_set_value(rm690b0_lvgl_scale_obj_t *self, mp_int_t value);
mp_int_t common_hal_rm690b0_lvgl_scale_get_value(rm690b0_lvgl_scale_obj_t *self);

void common_hal_rm690b0_lvgl_scale_set_ticks(rm690b0_lvgl_scale_obj_t *self,
    mp_int_t tick_count,
    mp_int_t tick_width,
    mp_int_t tick_length,
    uint32_t tick_color);

void common_hal_rm690b0_lvgl_scale_set_major_ticks(rm690b0_lvgl_scale_obj_t *self,
    mp_int_t nth,
    mp_int_t width,
    mp_int_t length,
    uint32_t color,
    mp_int_t label_gap);
