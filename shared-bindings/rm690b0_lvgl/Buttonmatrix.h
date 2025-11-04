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
    mp_obj_t on_click_handler;
    mp_obj_t buttons_list;
    const char **btn_map;
} rm690b0_lvgl_buttonmatrix_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_buttonmatrix_type;

void common_hal_rm690b0_lvgl_buttonmatrix_construct(rm690b0_lvgl_buttonmatrix_obj_t *self, mp_obj_t buttons);
void common_hal_rm690b0_lvgl_buttonmatrix_set_map(rm690b0_lvgl_buttonmatrix_obj_t *self, mp_obj_t buttons);
uint16_t common_hal_rm690b0_lvgl_buttonmatrix_get_selected_btn(rm690b0_lvgl_buttonmatrix_obj_t *self);
void common_hal_rm690b0_lvgl_buttonmatrix_set_selected_btn(rm690b0_lvgl_buttonmatrix_obj_t *self, uint16_t btn_id);
