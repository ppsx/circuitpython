// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"

// Dropdown widget object
typedef struct {
    mp_obj_base_t base;
    void *native_obj; // lv_obj_t* handle (dropdown)
    mp_obj_t on_change_handler; // Python callback for selection changes
} rm690b0_lvgl_dropdown_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_dropdown_type;

void common_hal_rm690b0_lvgl_dropdown_construct(rm690b0_lvgl_dropdown_obj_t *self, const char *options);
void common_hal_rm690b0_lvgl_dropdown_deinit(rm690b0_lvgl_dropdown_obj_t *self);

mp_int_t common_hal_rm690b0_lvgl_dropdown_get_selected(rm690b0_lvgl_dropdown_obj_t *self);
void common_hal_rm690b0_lvgl_dropdown_set_selected(rm690b0_lvgl_dropdown_obj_t *self, mp_int_t index);

void common_hal_rm690b0_lvgl_dropdown_get_text(rm690b0_lvgl_dropdown_obj_t *self, char *buf, size_t buf_size);
void common_hal_rm690b0_lvgl_dropdown_set_options(rm690b0_lvgl_dropdown_obj_t *self, const char *options);

void common_hal_rm690b0_lvgl_dropdown_clear_options(rm690b0_lvgl_dropdown_obj_t *self);
void common_hal_rm690b0_lvgl_dropdown_add_option(rm690b0_lvgl_dropdown_obj_t *self, const char *option, mp_int_t pos);
