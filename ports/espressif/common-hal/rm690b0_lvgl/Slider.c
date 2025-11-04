// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Slider.h"
#include "common-hal/rm690b0_lvgl/Slider.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

// Event callback for slider value changes
static void slider_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *slider = lv_event_get_target(e);
    
    if (code == LV_EVENT_VALUE_CHANGED) {
        // Get the slider object from user_data
        rm690b0_lvgl_slider_obj_t *self = (rm690b0_lvgl_slider_obj_t *)lv_obj_get_user_data(slider);
        
        if (self != NULL && self->on_change_handler != mp_const_none) {
            // Call the Python callback
            mp_call_function_1(self->on_change_handler, MP_OBJ_FROM_PTR(self));
        }
    }
}

void common_hal_rm690b0_lvgl_slider_construct(rm690b0_lvgl_slider_obj_t *self, mp_int_t min_value, mp_int_t max_value) {
    // Create LVGL slider on the active screen
    lv_obj_t *slider = lv_slider_create(lv_scr_act());
    if (slider == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL slider"));
    }
    
    self->native_obj = slider;
    
    // Set range
    lv_slider_set_range(slider, (int32_t)min_value, (int32_t)max_value);
    
    // Set initial value to minimum
    lv_slider_set_value(slider, (int32_t)min_value, LV_ANIM_OFF);
    
    // Store reference to self in user_data for event callbacks
    lv_obj_set_user_data(slider, self);
    
    // Add event callback for value changes
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Set default size
    lv_obj_set_width(slider, 200);
    lv_obj_set_height(slider, 10);
}

void common_hal_rm690b0_lvgl_slider_deinit(rm690b0_lvgl_slider_obj_t *self) {
    if (self->native_obj != NULL) {
        lv_obj_del((lv_obj_t *)self->native_obj);
        self->native_obj = NULL;
    }
}

mp_int_t common_hal_rm690b0_lvgl_slider_get_value(rm690b0_lvgl_slider_obj_t *self) {
    lv_obj_t *slider = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    return (mp_int_t)lv_slider_get_value(slider);
}

void common_hal_rm690b0_lvgl_slider_set_value(rm690b0_lvgl_slider_obj_t *self, mp_int_t value) {
    lv_obj_t *slider = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    
    // Check if value is actually changing
    mp_int_t current_value = common_hal_rm690b0_lvgl_slider_get_value(self);
    if (current_value == value) {
        return;  // No change needed
    }
    
    lv_slider_set_value(slider, (int32_t)value, LV_ANIM_OFF);
    
    // Manually trigger the callback when set programmatically
    if (self->on_change_handler != mp_const_none) {
        mp_call_function_1(self->on_change_handler, MP_OBJ_FROM_PTR(self));
    }
}

mp_int_t common_hal_rm690b0_lvgl_slider_get_min_value(rm690b0_lvgl_slider_obj_t *self) {
    lv_obj_t *slider = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    return (mp_int_t)lv_slider_get_min_value(slider);
}

void common_hal_rm690b0_lvgl_slider_set_min_value(rm690b0_lvgl_slider_obj_t *self, mp_int_t min_value) {
    lv_obj_t *slider = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    int32_t max_value = lv_slider_get_max_value(slider);
    lv_slider_set_range(slider, (int32_t)min_value, max_value);
}

mp_int_t common_hal_rm690b0_lvgl_slider_get_max_value(rm690b0_lvgl_slider_obj_t *self) {
    lv_obj_t *slider = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    return (mp_int_t)lv_slider_get_max_value(slider);
}

void common_hal_rm690b0_lvgl_slider_set_max_value(rm690b0_lvgl_slider_obj_t *self, mp_int_t max_value) {
    lv_obj_t *slider = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    int32_t min_value = lv_slider_get_min_value(slider);
    lv_slider_set_range(slider, min_value, (int32_t)max_value);
}

void common_hal_rm690b0_lvgl_slider_set_range(rm690b0_lvgl_slider_obj_t *self, mp_int_t min_value, mp_int_t max_value) {
    lv_obj_t *slider = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    lv_slider_set_range(slider, (int32_t)min_value, (int32_t)max_value);
}