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
} rm690b0_lvgl_msgbox_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_msgbox_type;

void common_hal_rm690b0_lvgl_msgbox_construct(rm690b0_lvgl_msgbox_obj_t *self, const char *title, const char *text, mp_obj_t buttons, bool close_btn);
void common_hal_rm690b0_lvgl_msgbox_close(rm690b0_lvgl_msgbox_obj_t *self);