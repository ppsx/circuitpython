// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Textarea.h"
#include "common-hal/rm690b0_lvgl/Textarea.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

// Event callback for text changes
static void textarea_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    rm690b0_lvgl_textarea_obj_t *self = (rm690b0_lvgl_textarea_obj_t *)lv_event_get_user_data(e);

    if (self == NULL) {
        return;
    }

    if (code == LV_EVENT_VALUE_CHANGED && self->on_change_handler != mp_const_none) {
        mp_call_function_1(self->on_change_handler, MP_OBJ_FROM_PTR(self));
    } else if ((code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED) && self->on_focus_handler != mp_const_none) {
        mp_call_function_1(self->on_focus_handler, MP_OBJ_FROM_PTR(self));
    } else if (code == LV_EVENT_READY && self->on_submit_handler != mp_const_none) {
        mp_call_function_1(self->on_submit_handler, MP_OBJ_FROM_PTR(self));
    }
}

void common_hal_rm690b0_lvgl_textarea_construct(rm690b0_lvgl_textarea_obj_t *self) {
    lv_obj_t *ta = lv_textarea_create(lv_scr_act());
    if (ta == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL textarea"));
        return;
    }

    lv_obj_add_event_cb(ta, textarea_event_cb, LV_EVENT_ALL, self);

    self->base.native_obj = ta;
    self->base.callback = mp_const_none;
    self->on_change_handler = mp_const_none;
    self->on_focus_handler = mp_const_none;
    self->on_submit_handler = mp_const_none;

    // Sensible defaults
    lv_obj_set_size(ta, 200, 80);
    lv_textarea_set_one_line(ta, false);
}

void common_hal_rm690b0_lvgl_textarea_set_text(rm690b0_lvgl_textarea_obj_t *self, const char *text) {
    lv_obj_t *ta = (lv_obj_t *)self->base.native_obj;
    lv_textarea_set_text(ta, text);
}

const char *common_hal_rm690b0_lvgl_textarea_get_text(rm690b0_lvgl_textarea_obj_t *self) {
    lv_obj_t *ta = (lv_obj_t *)self->base.native_obj;
    return lv_textarea_get_text(ta);
}

void common_hal_rm690b0_lvgl_textarea_set_placeholder(rm690b0_lvgl_textarea_obj_t *self, const char *text) {
    lv_obj_t *ta = (lv_obj_t *)self->base.native_obj;
    lv_textarea_set_placeholder_text(ta, text);
}

const char *common_hal_rm690b0_lvgl_textarea_get_placeholder(rm690b0_lvgl_textarea_obj_t *self) {
    lv_obj_t *ta = (lv_obj_t *)self->base.native_obj;
    const char *txt = lv_textarea_get_placeholder_text(ta);
    return txt ? txt : "";
}

void common_hal_rm690b0_lvgl_textarea_set_password_mode(rm690b0_lvgl_textarea_obj_t *self, bool enabled) {
    lv_obj_t *ta = (lv_obj_t *)self->base.native_obj;
    lv_textarea_set_password_mode(ta, enabled);
}

bool common_hal_rm690b0_lvgl_textarea_get_password_mode(rm690b0_lvgl_textarea_obj_t *self) {
    lv_obj_t *ta = (lv_obj_t *)self->base.native_obj;
    return lv_textarea_get_password_mode(ta);
}

void common_hal_rm690b0_lvgl_textarea_set_one_line(rm690b0_lvgl_textarea_obj_t *self, bool enabled) {
    lv_obj_t *ta = (lv_obj_t *)self->base.native_obj;
    lv_textarea_set_one_line(ta, enabled);
}

bool common_hal_rm690b0_lvgl_textarea_get_one_line(rm690b0_lvgl_textarea_obj_t *self) {
    lv_obj_t *ta = (lv_obj_t *)self->base.native_obj;
    return lv_textarea_get_one_line(ta);
}

void common_hal_rm690b0_lvgl_textarea_set_max_length(rm690b0_lvgl_textarea_obj_t *self, uint32_t max_len) {
    lv_obj_t *ta = (lv_obj_t *)self->base.native_obj;
    lv_textarea_set_max_length(ta, max_len);
}

uint32_t common_hal_rm690b0_lvgl_textarea_get_max_length(rm690b0_lvgl_textarea_obj_t *self) {
    lv_obj_t *ta = (lv_obj_t *)self->base.native_obj;
    return lv_textarea_get_max_length(ta);
}
