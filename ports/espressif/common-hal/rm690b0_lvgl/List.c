// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/List.h"
#include "shared-bindings/rm690b0_lvgl/Button.h"
#include "shared-bindings/rm690b0_lvgl/Label.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

static void list_btn_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    rm690b0_lvgl_button_obj_t *btn_obj = (rm690b0_lvgl_button_obj_t *)lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        if (btn_obj->on_click_handler != mp_const_none) {
            mp_call_function_1(btn_obj->on_click_handler, MP_OBJ_FROM_PTR(btn_obj));
        }
    }
}

void common_hal_rm690b0_lvgl_list_construct(rm690b0_lvgl_list_obj_t *self) {
    lv_obj_t *list = lv_list_create(lv_scr_act());
    if (list == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL list"));
        return;
    }
    
    self->base.native_obj = list;
    self->base.callback = mp_const_none;
}

mp_obj_t common_hal_rm690b0_lvgl_list_add_btn(rm690b0_lvgl_list_obj_t *self, const char *icon, const char *text) {
    lv_obj_t *list = (lv_obj_t *)self->base.native_obj;
    lv_obj_t *btn = lv_list_add_btn(list, icon, text);
    
    if (btn == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create list button"));
        return mp_const_none;
    }

    // Create Python Button wrapper
    rm690b0_lvgl_button_obj_t *py_btn = mp_obj_malloc(rm690b0_lvgl_button_obj_t, &rm690b0_lvgl_button_type);
    
    // Initialize Widget base
    py_btn->base.native_obj = btn;
    py_btn->base.callback = mp_const_none;
    
    // Initialize Button specific
    py_btn->on_click_handler = mp_const_none;
    
    // Attach event handler
    lv_obj_add_event_cb(btn, list_btn_event_handler, LV_EVENT_ALL, py_btn);
    
    return MP_OBJ_FROM_PTR(py_btn);
}

mp_obj_t common_hal_rm690b0_lvgl_list_add_text(rm690b0_lvgl_list_obj_t *self, const char *text) {
    lv_obj_t *list = (lv_obj_t *)self->base.native_obj;
    lv_obj_t *lbl = lv_list_add_text(list, text);
    
    if (lbl == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create list text"));
        return mp_const_none;
    }

    // Create Python Label wrapper
    rm690b0_lvgl_label_obj_t *py_lbl = mp_obj_malloc(rm690b0_lvgl_label_obj_t, &rm690b0_lvgl_label_type);
    
    // Initialize Widget base
    py_lbl->base.native_obj = lbl;
    py_lbl->base.callback = mp_const_none;
    
    return MP_OBJ_FROM_PTR(py_lbl);
}