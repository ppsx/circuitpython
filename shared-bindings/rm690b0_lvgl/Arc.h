// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"

// Arc widget object
typedef struct {
    mp_obj_base_t base;
    void *native_obj; // lv_obj_t* handle (arc)
    mp_obj_t on_change_handler; // Python callback for value changes
} rm690b0_lvgl_arc_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_arc_type;

void common_hal_rm690b0_lvgl_arc_construct(rm690b0_lvgl_arc_obj_t *self, mp_int_t min_value, mp_int_t max_value);
void common_hal_rm690b0_lvgl_arc_deinit(rm690b0_lvgl_arc_obj_t *self);

mp_int_t common_hal_rm690b0_lvgl_arc_get_value(rm690b0_lvgl_arc_obj_t *self);
void common_hal_rm690b0_lvgl_arc_set_value(rm690b0_lvgl_arc_obj_t *self, mp_int_t value);

mp_int_t common_hal_rm690b0_lvgl_arc_get_min_value(rm690b0_lvgl_arc_obj_t *self);
void common_hal_rm690b0_lvgl_arc_set_min_value(rm690b0_lvgl_arc_obj_t *self, mp_int_t min_value);

mp_int_t common_hal_rm690b0_lvgl_arc_get_max_value(rm690b0_lvgl_arc_obj_t *self);
void common_hal_rm690b0_lvgl_arc_set_max_value(rm690b0_lvgl_arc_obj_t *self, mp_int_t max_value);

void common_hal_rm690b0_lvgl_arc_set_range(rm690b0_lvgl_arc_obj_t *self, mp_int_t min_value, mp_int_t max_value);