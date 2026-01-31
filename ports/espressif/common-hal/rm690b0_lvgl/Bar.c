// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Bar.h"
#include "common-hal/rm690b0_lvgl/Bar.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

void common_hal_rm690b0_lvgl_bar_construct(rm690b0_lvgl_bar_obj_t *self, mp_int_t min_value, mp_int_t max_value) {
    // Create LVGL bar on the active screen
    lv_obj_t *bar = lv_bar_create(lv_scr_act());
    if (bar == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL bar"));
    }

    self->native_obj = bar;

    // Set range
    lv_bar_set_range(bar, (int32_t)min_value, (int32_t)max_value);

    // Set initial value to minimum
    lv_bar_set_value(bar, (int32_t)min_value, LV_ANIM_OFF);

    // Set default size
    lv_obj_set_width(bar, 200);
    lv_obj_set_height(bar, 20);
}

void common_hal_rm690b0_lvgl_bar_deinit(rm690b0_lvgl_bar_obj_t *self) {
    if (self->native_obj != NULL) {
        lv_obj_del((lv_obj_t *)self->native_obj);
        self->native_obj = NULL;
    }
}

mp_int_t common_hal_rm690b0_lvgl_bar_get_value(rm690b0_lvgl_bar_obj_t *self) {
    lv_obj_t *bar = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    return (mp_int_t)lv_bar_get_value(bar);
}

void common_hal_rm690b0_lvgl_bar_set_value(rm690b0_lvgl_bar_obj_t *self, mp_int_t value) {
    lv_obj_t *bar = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    lv_bar_set_value(bar, (int32_t)value, LV_ANIM_OFF);
}

mp_int_t common_hal_rm690b0_lvgl_bar_get_min_value(rm690b0_lvgl_bar_obj_t *self) {
    lv_obj_t *bar = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    return (mp_int_t)lv_bar_get_min_value(bar);
}

void common_hal_rm690b0_lvgl_bar_set_min_value(rm690b0_lvgl_bar_obj_t *self, mp_int_t min_value) {
    lv_obj_t *bar = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    int32_t max_value = lv_bar_get_max_value(bar);
    lv_bar_set_range(bar, (int32_t)min_value, max_value);
}

mp_int_t common_hal_rm690b0_lvgl_bar_get_max_value(rm690b0_lvgl_bar_obj_t *self) {
    lv_obj_t *bar = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    return (mp_int_t)lv_bar_get_max_value(bar);
}

void common_hal_rm690b0_lvgl_bar_set_max_value(rm690b0_lvgl_bar_obj_t *self, mp_int_t max_value) {
    lv_obj_t *bar = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    int32_t min_value = lv_bar_get_min_value(bar);
    lv_bar_set_range(bar, min_value, (int32_t)max_value);
}

void common_hal_rm690b0_lvgl_bar_set_range(rm690b0_lvgl_bar_obj_t *self, mp_int_t min_value, mp_int_t max_value) {
    lv_obj_t *bar = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    lv_bar_set_range(bar, (int32_t)min_value, (int32_t)max_value);
}
