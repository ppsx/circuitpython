// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"

// Checkbox widget object
typedef struct {
    mp_obj_base_t base;
    void *native_obj; // lv_obj_t* handle (checkbox)
    mp_obj_t on_change_handler; // Python callback for state changes
} rm690b0_lvgl_checkbox_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_checkbox_type;

void common_hal_rm690b0_lvgl_checkbox_construct(rm690b0_lvgl_checkbox_obj_t *self, const char *text);
void common_hal_rm690b0_lvgl_checkbox_deinit(rm690b0_lvgl_checkbox_obj_t *self);

bool common_hal_rm690b0_lvgl_checkbox_get_checked(rm690b0_lvgl_checkbox_obj_t *self);
void common_hal_rm690b0_lvgl_checkbox_set_checked(rm690b0_lvgl_checkbox_obj_t *self, bool checked);

const char *common_hal_rm690b0_lvgl_checkbox_get_text(rm690b0_lvgl_checkbox_obj_t *self);
void common_hal_rm690b0_lvgl_checkbox_set_text(rm690b0_lvgl_checkbox_obj_t *self, const char *text);
