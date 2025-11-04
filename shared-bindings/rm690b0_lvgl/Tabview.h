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
} rm690b0_lvgl_tabview_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_tabview_type;

void common_hal_rm690b0_lvgl_tabview_construct(rm690b0_lvgl_tabview_obj_t *self, mp_int_t tab_pos, mp_int_t tab_size);
mp_obj_t common_hal_rm690b0_lvgl_tabview_add_tab(rm690b0_lvgl_tabview_obj_t *self, const char *name);
uint16_t common_hal_rm690b0_lvgl_tabview_get_active_tab(rm690b0_lvgl_tabview_obj_t *self);
void common_hal_rm690b0_lvgl_tabview_set_active_tab(rm690b0_lvgl_tabview_obj_t *self, uint16_t idx);
