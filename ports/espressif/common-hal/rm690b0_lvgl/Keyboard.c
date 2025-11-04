// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Keyboard.h"
#include "shared-bindings/rm690b0_lvgl/Textarea.h"
#include "common-hal/rm690b0_lvgl/Keyboard.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

// Event callback for keyboard events
static void keyboard_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    rm690b0_lvgl_keyboard_obj_t *self = (rm690b0_lvgl_keyboard_obj_t *)lv_event_get_user_data(e);
    if (self == NULL) {
        return;
    }

    if (code == LV_EVENT_VALUE_CHANGED) {
        if (self->on_change_handler != mp_const_none) {
            mp_call_function_1(self->on_change_handler, MP_OBJ_FROM_PTR(self));
        }
    }
}

void common_hal_rm690b0_lvgl_keyboard_construct(rm690b0_lvgl_keyboard_obj_t *self) {
    lv_obj_t *kb = lv_keyboard_create(lv_scr_act());
    if (kb == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL keyboard"));
        return;
    }

    lv_obj_add_event_cb(kb, keyboard_event_cb, LV_EVENT_VALUE_CHANGED, self);

    self->base.native_obj = kb;
    self->base.callback = mp_const_none;
    self->on_change_handler = mp_const_none;
    self->popovers_enabled = false;

    // Default size
    lv_obj_set_width(kb, 320);
    lv_obj_set_height(kb, 140);
}

void common_hal_rm690b0_lvgl_keyboard_set_textarea(rm690b0_lvgl_keyboard_obj_t *self, mp_obj_t textarea_obj) {
    lv_obj_t *kb = (lv_obj_t *)self->base.native_obj;

    if (textarea_obj == mp_const_none) {
        lv_keyboard_set_textarea(kb, NULL);
        return;
    }

    const mp_obj_type_t *type = mp_obj_get_type(textarea_obj);
    if (!mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(type), MP_OBJ_FROM_PTR(&rm690b0_lvgl_textarea_type))) {
        mp_raise_TypeError(MP_ERROR_TEXT("Expected a Textarea or None"));
    }
    rm690b0_lvgl_textarea_obj_t *ta = MP_OBJ_TO_PTR(textarea_obj);
    lv_keyboard_set_textarea(kb, (lv_obj_t *)ta->base.native_obj);
}

void common_hal_rm690b0_lvgl_keyboard_set_mode(rm690b0_lvgl_keyboard_obj_t *self, uint8_t mode) {
    lv_obj_t *kb = (lv_obj_t *)self->base.native_obj;
    lv_keyboard_set_mode(kb, mode);
}

uint8_t common_hal_rm690b0_lvgl_keyboard_get_mode(rm690b0_lvgl_keyboard_obj_t *self) {
    lv_obj_t *kb = (lv_obj_t *)self->base.native_obj;
    return lv_keyboard_get_mode(kb);
}

void common_hal_rm690b0_lvgl_keyboard_set_popovers(rm690b0_lvgl_keyboard_obj_t *self, bool enabled) {
    lv_obj_t *kb = (lv_obj_t *)self->base.native_obj;
    lv_keyboard_set_popovers(kb, enabled);
    self->popovers_enabled = enabled;
}

bool common_hal_rm690b0_lvgl_keyboard_get_popovers(rm690b0_lvgl_keyboard_obj_t *self) {
    return self->popovers_enabled;
}
