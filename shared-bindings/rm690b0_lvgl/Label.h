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
} rm690b0_lvgl_label_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_label_type;

void common_hal_rm690b0_lvgl_label_construct(rm690b0_lvgl_label_obj_t *self, const char *text);
void common_hal_rm690b0_lvgl_label_set_text(rm690b0_lvgl_label_obj_t *self, const char *text);
const char* common_hal_rm690b0_lvgl_label_get_text(rm690b0_lvgl_label_obj_t *self);
void common_hal_rm690b0_lvgl_label_set_text_color(rm690b0_lvgl_label_obj_t *self, uint32_t color);