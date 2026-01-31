// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"

// Switch widget object
typedef struct {
    mp_obj_base_t base;
    void *native_obj; // lv_obj_t* handle (switch)
    mp_obj_t on_change_handler; // Python callback for state changes
} rm690b0_lvgl_switch_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_switch_type;

void common_hal_rm690b0_lvgl_switch_construct(rm690b0_lvgl_switch_obj_t *self);
void common_hal_rm690b0_lvgl_switch_deinit(rm690b0_lvgl_switch_obj_t *self);

bool common_hal_rm690b0_lvgl_switch_get_checked(rm690b0_lvgl_switch_obj_t *self);
void common_hal_rm690b0_lvgl_switch_set_checked(rm690b0_lvgl_switch_obj_t *self, bool checked);
