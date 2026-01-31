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
    mp_obj_t on_change_handler;
} rm690b0_lvgl_spinbox_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_spinbox_type;

void common_hal_rm690b0_lvgl_spinbox_construct(rm690b0_lvgl_spinbox_obj_t *self);
void common_hal_rm690b0_lvgl_spinbox_set_value(rm690b0_lvgl_spinbox_obj_t *self, mp_int_t value);
mp_int_t common_hal_rm690b0_lvgl_spinbox_get_value(rm690b0_lvgl_spinbox_obj_t *self);
void common_hal_rm690b0_lvgl_spinbox_set_range(rm690b0_lvgl_spinbox_obj_t *self, mp_int_t min_value, mp_int_t max_value);
void common_hal_rm690b0_lvgl_spinbox_increment(rm690b0_lvgl_spinbox_obj_t *self);
void common_hal_rm690b0_lvgl_spinbox_decrement(rm690b0_lvgl_spinbox_obj_t *self);
void common_hal_rm690b0_lvgl_spinbox_set_digit_format(rm690b0_lvgl_spinbox_obj_t *self, uint8_t digit_count, uint8_t separator_position);
void common_hal_rm690b0_lvgl_spinbox_set_step(rm690b0_lvgl_spinbox_obj_t *self, uint32_t step);
