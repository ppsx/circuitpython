// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Spinbox.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

static void spinbox_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    rm690b0_lvgl_spinbox_obj_t *self = (rm690b0_lvgl_spinbox_obj_t *)lv_event_get_user_data(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        if (self->on_change_handler != mp_const_none) {
            mp_call_function_1(self->on_change_handler, MP_OBJ_FROM_PTR(self));
        }
    }
}

void common_hal_rm690b0_lvgl_spinbox_construct(rm690b0_lvgl_spinbox_obj_t *self) {
    lv_obj_t *sb = lv_spinbox_create(lv_scr_act());
    if (sb == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL spinbox"));
        return;
    }
    
    // Default setup
    lv_spinbox_set_rollover(sb, true);
    lv_spinbox_set_digit_format(sb, 4, 0); // 4 digits, no separator
    lv_spinbox_set_range(sb, 0, 9999);
    
    lv_obj_add_event_cb(sb, spinbox_event_handler, LV_EVENT_VALUE_CHANGED, self);
    
    self->base.native_obj = sb;
    self->base.callback = mp_const_none;
}

void common_hal_rm690b0_lvgl_spinbox_set_value(rm690b0_lvgl_spinbox_obj_t *self, mp_int_t value) {
    lv_obj_t *sb = (lv_obj_t *)self->base.native_obj;
    lv_spinbox_set_value(sb, (int32_t)value);
}

mp_int_t common_hal_rm690b0_lvgl_spinbox_get_value(rm690b0_lvgl_spinbox_obj_t *self) {
    lv_obj_t *sb = (lv_obj_t *)self->base.native_obj;
    return (mp_int_t)lv_spinbox_get_value(sb);
}

void common_hal_rm690b0_lvgl_spinbox_set_range(rm690b0_lvgl_spinbox_obj_t *self, mp_int_t min_value, mp_int_t max_value) {
    lv_obj_t *sb = (lv_obj_t *)self->base.native_obj;
    lv_spinbox_set_range(sb, (int32_t)min_value, (int32_t)max_value);
}

void common_hal_rm690b0_lvgl_spinbox_increment(rm690b0_lvgl_spinbox_obj_t *self) {
    lv_obj_t *sb = (lv_obj_t *)self->base.native_obj;
    lv_spinbox_increment(sb);
}

void common_hal_rm690b0_lvgl_spinbox_decrement(rm690b0_lvgl_spinbox_obj_t *self) {
    lv_obj_t *sb = (lv_obj_t *)self->base.native_obj;
    lv_spinbox_decrement(sb);
}

void common_hal_rm690b0_lvgl_spinbox_set_digit_format(rm690b0_lvgl_spinbox_obj_t *self, uint8_t digit_count, uint8_t separator_position) {
    lv_obj_t *sb = (lv_obj_t *)self->base.native_obj;
    lv_spinbox_set_digit_format(sb, (uint8_t)digit_count, (uint8_t)separator_position);
}

void common_hal_rm690b0_lvgl_spinbox_set_step(rm690b0_lvgl_spinbox_obj_t *self, uint32_t step) {
    lv_obj_t *sb = (lv_obj_t *)self->base.native_obj;
    lv_spinbox_set_step(sb, (uint32_t)step);
}