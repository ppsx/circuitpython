// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Container.h"
#include "common-hal/rm690b0_lvgl/Container.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

void common_hal_rm690b0_lvgl_container_construct(rm690b0_lvgl_container_obj_t *self) {
    // Create generic LVGL object (container) on the active screen
    lv_obj_t *container = lv_obj_create(lv_scr_act());
    if (container == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL container"));
    }

    // Set default properties
    self->base.native_obj = container;
    self->base.callback = mp_const_none;
}

void common_hal_rm690b0_lvgl_container_set_flex_flow(rm690b0_lvgl_container_obj_t *self, mp_int_t flow) {
    lv_obj_t *native_obj = common_hal_rm690b0_lvgl_widget_get_native_obj(&self->base);
    lv_obj_set_flex_flow(native_obj, (lv_flex_flow_t)flow);
}

void common_hal_rm690b0_lvgl_container_set_flex_align(rm690b0_lvgl_container_obj_t *self, mp_int_t main_place, mp_int_t cross_place, mp_int_t track_cross_place) {
    lv_obj_t *native_obj = common_hal_rm690b0_lvgl_widget_get_native_obj(&self->base);
    lv_obj_set_flex_align(native_obj, (lv_flex_align_t)main_place, (lv_flex_align_t)cross_place, (lv_flex_align_t)track_cross_place);
}

void common_hal_rm690b0_lvgl_container_set_padding(rm690b0_lvgl_container_obj_t *self, mp_int_t padding) {
    lv_obj_t *native_obj = common_hal_rm690b0_lvgl_widget_get_native_obj(&self->base);
    lv_obj_set_style_pad_all(native_obj, (lv_coord_t)padding, 0);
}
