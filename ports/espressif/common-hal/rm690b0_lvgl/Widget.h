// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

// Helper to retrieve the native LVGL object from the CircuitPython wrapper
static inline lv_obj_t* common_hal_rm690b0_lvgl_widget_get_native_obj(rm690b0_lvgl_widget_obj_t *self) {
    return (lv_obj_t*)self->native_obj;
}

static inline void common_hal_rm690b0_lvgl_widget_invoke_callback(rm690b0_lvgl_widget_obj_t *self, mp_obj_t arg) {
    if (self == NULL || self->callback == NULL || self->callback == mp_const_none) {
        return;
    }
    mp_call_function_1(self->callback, arg);
}