// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"

typedef struct {
    mp_obj_base_t base;
    void *native_font; // lv_font_t*
    mp_obj_t source;   // Keep reference to source data/string to prevent GC
} rm690b0_lvgl_font_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_font_type;

void common_hal_rm690b0_lvgl_font_construct(rm690b0_lvgl_font_obj_t *self, mp_obj_t file_or_data, mp_int_t size);
void common_hal_rm690b0_lvgl_font_set_size(rm690b0_lvgl_font_obj_t *self, mp_int_t size);
void common_hal_rm690b0_lvgl_font_deinit(rm690b0_lvgl_font_obj_t *self);