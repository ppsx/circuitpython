// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include <stdbool.h>

#include "py/obj.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"

// Keyboard widget object
typedef struct {
    rm690b0_lvgl_widget_obj_t base;
    mp_obj_t on_change_handler; // Python callback for key presses/value changes
    bool popovers_enabled;
} rm690b0_lvgl_keyboard_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_keyboard_type;

void common_hal_rm690b0_lvgl_keyboard_construct(rm690b0_lvgl_keyboard_obj_t *self);
void common_hal_rm690b0_lvgl_keyboard_set_textarea(rm690b0_lvgl_keyboard_obj_t *self, mp_obj_t textarea_obj);
void common_hal_rm690b0_lvgl_keyboard_set_mode(rm690b0_lvgl_keyboard_obj_t *self, uint8_t mode);
uint8_t common_hal_rm690b0_lvgl_keyboard_get_mode(rm690b0_lvgl_keyboard_obj_t *self);
void common_hal_rm690b0_lvgl_keyboard_set_popovers(rm690b0_lvgl_keyboard_obj_t *self, bool enabled);
bool common_hal_rm690b0_lvgl_keyboard_get_popovers(rm690b0_lvgl_keyboard_obj_t *self);
