// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Button.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

static void button_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    rm690b0_lvgl_button_obj_t *self = (rm690b0_lvgl_button_obj_t *)lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        if (self->on_click_handler != mp_const_none) {
            mp_call_function_1(self->on_click_handler, MP_OBJ_FROM_PTR(self));
        }
    }
}

void common_hal_rm690b0_lvgl_button_construct(rm690b0_lvgl_button_obj_t *self, const char *text) {
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    if (btn == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL button"));
        return;
    }
    
    lv_obj_t *label = lv_label_create(btn);
    if (label == NULL) {
        lv_obj_del(btn);
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL button label"));
        return;
    }
    lv_label_set_text(label, text);
    lv_obj_center(label);
    
    lv_obj_add_event_cb(btn, button_event_handler, LV_EVENT_ALL, self);
    
    self->base.native_obj = btn;
    self->base.callback = mp_const_none;
}

void common_hal_rm690b0_lvgl_button_set_text(rm690b0_lvgl_button_obj_t *self, const char *text) {
    lv_obj_t *btn = (lv_obj_t *)self->base.native_obj;
    // Get the label (first child)
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    if (label) {
        lv_label_set_text(label, text);
    }
}

const char* common_hal_rm690b0_lvgl_button_get_text(rm690b0_lvgl_button_obj_t *self) {
    lv_obj_t *btn = (lv_obj_t *)self->base.native_obj;
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    if (label) {
        return lv_label_get_text(label);
    }
    return "";
}