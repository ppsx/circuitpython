// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Dropdown.h"
#include "common-hal/rm690b0_lvgl/Dropdown.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"
#include <string.h>

// Event callback for dropdown selection changes
static void dropdown_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *dropdown = lv_event_get_target(e);
    
    if (code == LV_EVENT_VALUE_CHANGED) {
        // Get the dropdown object from user_data
        rm690b0_lvgl_dropdown_obj_t *self = (rm690b0_lvgl_dropdown_obj_t *)lv_obj_get_user_data(dropdown);
        
        if (self != NULL && self->on_change_handler != mp_const_none) {
            // Call the Python callback
            mp_call_function_1(self->on_change_handler, MP_OBJ_FROM_PTR(self));
        }
    }
}

void common_hal_rm690b0_lvgl_dropdown_construct(rm690b0_lvgl_dropdown_obj_t *self, const char *options) {
    // Create LVGL dropdown on the active screen
    lv_obj_t *dropdown = lv_dropdown_create(lv_scr_act());
    if (dropdown == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL dropdown"));
    }
    
    self->native_obj = dropdown;
    
    // Set options
    lv_dropdown_set_options(dropdown, options);
    
    // Store reference to self in user_data for event callbacks
    lv_obj_set_user_data(dropdown, self);
    
    // Add event callback for value changes
    lv_obj_add_event_cb(dropdown, dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Set default size
    lv_obj_set_width(dropdown, 150);
}

void common_hal_rm690b0_lvgl_dropdown_deinit(rm690b0_lvgl_dropdown_obj_t *self) {
    if (self->native_obj != NULL) {
        lv_obj_del((lv_obj_t *)self->native_obj);
        self->native_obj = NULL;
    }
}

mp_int_t common_hal_rm690b0_lvgl_dropdown_get_selected(rm690b0_lvgl_dropdown_obj_t *self) {
    lv_obj_t *dropdown = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    return (mp_int_t)lv_dropdown_get_selected(dropdown);
}

void common_hal_rm690b0_lvgl_dropdown_set_selected(rm690b0_lvgl_dropdown_obj_t *self, mp_int_t index) {
    lv_obj_t *dropdown = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    
    // Check if selection is actually changing
    mp_int_t current_index = common_hal_rm690b0_lvgl_dropdown_get_selected(self);
    if (current_index == index) {
        return;  // No change needed
    }
    
    lv_dropdown_set_selected(dropdown, (uint16_t)index);
    
    // Manually trigger the callback when set programmatically
    if (self->on_change_handler != mp_const_none) {
        mp_call_function_1(self->on_change_handler, MP_OBJ_FROM_PTR(self));
    }
}

void common_hal_rm690b0_lvgl_dropdown_get_text(rm690b0_lvgl_dropdown_obj_t *self, char *buf, size_t buf_size) {
    lv_obj_t *dropdown = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    lv_dropdown_get_selected_str(dropdown, buf, buf_size);
}

void common_hal_rm690b0_lvgl_dropdown_set_options(rm690b0_lvgl_dropdown_obj_t *self, const char *options) {
    lv_obj_t *dropdown = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    lv_dropdown_set_options(dropdown, options);
}

void common_hal_rm690b0_lvgl_dropdown_clear_options(rm690b0_lvgl_dropdown_obj_t *self) {
    lv_obj_t *dropdown = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    lv_dropdown_clear_options(dropdown);
}

void common_hal_rm690b0_lvgl_dropdown_add_option(rm690b0_lvgl_dropdown_obj_t *self, const char *option, mp_int_t pos) {
    lv_obj_t *dropdown = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    lv_dropdown_add_option(dropdown, option, (uint32_t)pos);
}