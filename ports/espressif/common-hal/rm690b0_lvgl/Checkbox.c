// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Checkbox.h"
#include "common-hal/rm690b0_lvgl/Checkbox.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

// Event callback for checkbox state changes
static void checkbox_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *checkbox = lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        // Get the checkbox object from user_data
        rm690b0_lvgl_checkbox_obj_t *self = (rm690b0_lvgl_checkbox_obj_t *)lv_obj_get_user_data(checkbox);

        if (self != NULL && self->on_change_handler != mp_const_none) {
            // Call the Python callback
            mp_call_function_1(self->on_change_handler, MP_OBJ_FROM_PTR(self));
        }
    }
}

void common_hal_rm690b0_lvgl_checkbox_construct(rm690b0_lvgl_checkbox_obj_t *self, const char *text) {
    // Create LVGL checkbox on the active screen
    lv_obj_t *checkbox = lv_checkbox_create(lv_scr_act());
    if (checkbox == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL checkbox"));
    }

    self->native_obj = checkbox;

    // Set text label
    lv_checkbox_set_text(checkbox, text);

    // Store reference to self in user_data for event callbacks
    lv_obj_set_user_data(checkbox, self);

    // Add event callback for value changes
    lv_obj_add_event_cb(checkbox, checkbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

void common_hal_rm690b0_lvgl_checkbox_deinit(rm690b0_lvgl_checkbox_obj_t *self) {
    if (self->native_obj != NULL) {
        lv_obj_del((lv_obj_t *)self->native_obj);
        self->native_obj = NULL;
    }
}

bool common_hal_rm690b0_lvgl_checkbox_get_checked(rm690b0_lvgl_checkbox_obj_t *self) {
    lv_obj_t *checkbox = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    uint32_t state = lv_obj_get_state(checkbox);
    return (state & LV_STATE_CHECKED) != 0;
}

void common_hal_rm690b0_lvgl_checkbox_set_checked(rm690b0_lvgl_checkbox_obj_t *self, bool checked) {
    lv_obj_t *checkbox = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);

    // Check if state is actually changing
    bool current_state = common_hal_rm690b0_lvgl_checkbox_get_checked(self);
    if (current_state == checked) {
        return;  // No change needed
    }

    if (checked) {
        lv_obj_add_state(checkbox, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(checkbox, LV_STATE_CHECKED);
    }

    // Manually trigger the callback when set programmatically
    if (self->on_change_handler != mp_const_none) {
        mp_call_function_1(self->on_change_handler, MP_OBJ_FROM_PTR(self));
    }
}

const char *common_hal_rm690b0_lvgl_checkbox_get_text(rm690b0_lvgl_checkbox_obj_t *self) {
    lv_obj_t *checkbox = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    return lv_checkbox_get_text(checkbox);
}

void common_hal_rm690b0_lvgl_checkbox_set_text(rm690b0_lvgl_checkbox_obj_t *self, const char *text) {
    lv_obj_t *checkbox = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    lv_checkbox_set_text(checkbox, text);
}
