// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"

// Roller object structure
typedef struct {
    rm690b0_lvgl_widget_obj_t base;
} rm690b0_lvgl_roller_obj_t;

// Type object declaration
extern const mp_obj_type_t rm690b0_lvgl_roller_type;

// Function declarations
void common_hal_rm690b0_lvgl_roller_construct(rm690b0_lvgl_roller_obj_t *self);
void common_hal_rm690b0_lvgl_roller_set_options(rm690b0_lvgl_roller_obj_t *self, const char *options, mp_int_t mode);
void common_hal_rm690b0_lvgl_roller_set_selected(rm690b0_lvgl_roller_obj_t *self, mp_int_t index, bool anim);
mp_int_t common_hal_rm690b0_lvgl_roller_get_selected(rm690b0_lvgl_roller_obj_t *self);
void common_hal_rm690b0_lvgl_roller_set_visible_row_count(rm690b0_lvgl_roller_obj_t *self, mp_int_t count);
const char *common_hal_rm690b0_lvgl_roller_get_selected_str(rm690b0_lvgl_roller_obj_t *self);