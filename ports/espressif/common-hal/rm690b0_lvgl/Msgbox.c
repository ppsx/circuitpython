// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Msgbox.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"
#include <string.h>

static void msgbox_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    rm690b0_lvgl_msgbox_obj_t *self = (rm690b0_lvgl_msgbox_obj_t *)lv_event_get_user_data(e);
    lv_obj_t *mbox = (lv_obj_t *)self->base.native_obj;

    if (code == LV_EVENT_VALUE_CHANGED) {
        // A button was clicked
        if (self->on_click_handler != mp_const_none) {
            uint16_t idx = lv_msgbox_get_active_btn(mbox);
            mp_call_function_1(self->on_click_handler, MP_OBJ_NEW_SMALL_INT(idx));
        }
    } else if (code == LV_EVENT_DELETE) {
        // Msgbox is being deleted (e.g. close button clicked)
        self->base.native_obj = NULL;
        if (self->on_click_handler != mp_const_none) {
            mp_call_function_1(self->on_click_handler, MP_OBJ_NEW_SMALL_INT(-1));
        }
    }
}

void common_hal_rm690b0_lvgl_msgbox_construct(rm690b0_lvgl_msgbox_obj_t *self, const char *title, const char *text, mp_obj_t buttons, bool close_btn) {
    size_t btn_count;
    mp_obj_t *btn_items;
    mp_obj_get_array(buttons, &btn_count, &btn_items);

    // Allocate array for pointers. +1 for terminator.
    self->btn_map = m_malloc((btn_count + 1) * sizeof(char *));
    if (self->btn_map == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to allocate memory for msgbox buttons"));
        return;
    }

    for (size_t i = 0; i < btn_count; i++) {
        self->btn_map[i] = mp_obj_str_get_str(btn_items[i]);
    }
    // Ensure null termination (empty string for LVGL btnmatrix map)
    self->btn_map[btn_count] = "";

    // Create msgbox on the top layer (parent=NULL)
    lv_obj_t *mbox = lv_msgbox_create(NULL, title, text, (const char **)self->btn_map, close_btn);
    if (mbox == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL msgbox"));
        return;
    }

    lv_obj_center(mbox);

    lv_obj_add_event_cb(mbox, msgbox_event_handler, LV_EVENT_ALL, self);

    self->base.native_obj = mbox;
    self->base.callback = mp_const_none;
}

void common_hal_rm690b0_lvgl_msgbox_close(rm690b0_lvgl_msgbox_obj_t *self) {
    if (self->base.native_obj != NULL) {
        lv_obj_t *obj = (lv_obj_t *)self->base.native_obj;
        self->base.native_obj = NULL;
        lv_msgbox_close(obj);
    }
}
