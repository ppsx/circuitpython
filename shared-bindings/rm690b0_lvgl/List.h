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
} rm690b0_lvgl_list_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_list_type;

void common_hal_rm690b0_lvgl_list_construct(rm690b0_lvgl_list_obj_t *self);
mp_obj_t common_hal_rm690b0_lvgl_list_add_btn(rm690b0_lvgl_list_obj_t *self, const char *icon, const char *text);
mp_obj_t common_hal_rm690b0_lvgl_list_add_text(rm690b0_lvgl_list_obj_t *self, const char *text);