// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Roller.h"
#include "common-hal/rm690b0_lvgl/Roller.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

static void roller_event_handler(lv_event_t *e) {
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) {
        return;
    }

    rm690b0_lvgl_roller_obj_t *self = (rm690b0_lvgl_roller_obj_t *)lv_event_get_user_data(e);
    if (self == NULL) {
        return;
    }

    common_hal_rm690b0_lvgl_widget_invoke_callback((rm690b0_lvgl_widget_obj_t *)self, MP_OBJ_FROM_PTR(self));
}

void common_hal_rm690b0_lvgl_roller_construct(rm690b0_lvgl_roller_obj_t *self) {
    // Create LVGL roller on the active screen
    lv_obj_t *roller = lv_roller_create(lv_scr_act());
    if (roller == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL roller"));
    }

    // Set default properties
    self->base.native_obj = roller;
    self->base.callback = mp_const_none;

    lv_obj_add_event_cb(roller, roller_event_handler, LV_EVENT_VALUE_CHANGED, self);

    // Default options (example)
    lv_roller_set_options(roller, "Option 1\nOption 2\nOption 3", LV_ROLLER_MODE_NORMAL);
}

void common_hal_rm690b0_lvgl_roller_set_options(rm690b0_lvgl_roller_obj_t *self, const char *options, mp_int_t mode) {
    lv_obj_t *roller = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    // mode 0: Normal, 1: Infinite
    lv_roller_mode_t lv_mode = (mode == 1) ? LV_ROLLER_MODE_INFINITE : LV_ROLLER_MODE_NORMAL;
    lv_roller_set_options(roller, options, lv_mode);
}

void common_hal_rm690b0_lvgl_roller_set_selected(rm690b0_lvgl_roller_obj_t *self, mp_int_t index, bool anim) {
    lv_obj_t *roller = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    // Animation mode: LV_ANIM_ON / LV_ANIM_OFF
    lv_anim_enable_t anim_en = anim ? LV_ANIM_ON : LV_ANIM_OFF;
    lv_roller_set_selected(roller, (uint16_t)index, anim_en);

    common_hal_rm690b0_lvgl_widget_invoke_callback((rm690b0_lvgl_widget_obj_t *)self, MP_OBJ_FROM_PTR(self));
}

mp_int_t common_hal_rm690b0_lvgl_roller_get_selected(rm690b0_lvgl_roller_obj_t *self) {
    lv_obj_t *roller = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    return (mp_int_t)lv_roller_get_selected(roller);
}

void common_hal_rm690b0_lvgl_roller_set_visible_row_count(rm690b0_lvgl_roller_obj_t *self, mp_int_t count) {
    lv_obj_t *roller = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    lv_roller_set_visible_row_count(roller, (uint8_t)count);
}

const char *common_hal_rm690b0_lvgl_roller_get_selected_str(rm690b0_lvgl_roller_obj_t *self) {
    lv_obj_t *roller = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);

    // We need a static buffer or GC-allocated buffer to return string
    // LVGL has a function to copy selected string to a buffer
    // Let's use a static buffer for now, similar to how other string getters might work
    // Or better: allocate a small buffer on stack, but return value needs to persist?
    // Actually, common_hal calls are used by shared-bindings which creates a Python string object immediately
    // So we can use a static buffer that is valid until the Python string is created.

    static char buf[256]; // Max option length
    lv_roller_get_selected_str(roller, buf, sizeof(buf));
    return buf;
}
