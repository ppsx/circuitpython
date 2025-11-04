// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Switch.h"
#include "common-hal/rm690b0_lvgl/Switch.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

// Event callback for switch state changes
static void switch_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *sw = lv_event_get_target(e);
    
    if (code == LV_EVENT_VALUE_CHANGED) {
        // Get the switch object from user_data
        rm690b0_lvgl_switch_obj_t *self = (rm690b0_lvgl_switch_obj_t *)lv_obj_get_user_data(sw);
        
        if (self != NULL && self->on_change_handler != mp_const_none) {
            // Call the Python callback
            mp_call_function_1(self->on_change_handler, MP_OBJ_FROM_PTR(self));
        }
    }
}

void common_hal_rm690b0_lvgl_switch_construct(rm690b0_lvgl_switch_obj_t *self) {
    // Create LVGL switch on the active screen
    lv_obj_t *sw = lv_switch_create(lv_scr_act());
    if (sw == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL switch"));
    }
    
    self->native_obj = sw;
    
    // Store reference to self in user_data for event callbacks
    lv_obj_set_user_data(sw, self);
    
    // Add event callback for value changes
    lv_obj_add_event_cb(sw, switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

void common_hal_rm690b0_lvgl_switch_deinit(rm690b0_lvgl_switch_obj_t *self) {
    if (self->native_obj != NULL) {
        lv_obj_del((lv_obj_t *)self->native_obj);
        self->native_obj = NULL;
    }
}

bool common_hal_rm690b0_lvgl_switch_get_checked(rm690b0_lvgl_switch_obj_t *self) {
    lv_obj_t *sw = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    uint32_t state = lv_obj_get_state(sw);
    return (state & LV_STATE_CHECKED) != 0;
}

void common_hal_rm690b0_lvgl_switch_set_checked(rm690b0_lvgl_switch_obj_t *self, bool checked) {
    lv_obj_t *sw = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    
    // Check if state is actually changing
    bool current_state = common_hal_rm690b0_lvgl_switch_get_checked(self);
    if (current_state == checked) {
        return;  // No change needed
    }
    
    if (checked) {
        lv_obj_add_state(sw, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(sw, LV_STATE_CHECKED);
    }
    
    // Manually trigger the callback when set programmatically
    if (self->on_change_handler != mp_const_none) {
        mp_call_function_1(self->on_change_handler, MP_OBJ_FROM_PTR(self));
    }
}