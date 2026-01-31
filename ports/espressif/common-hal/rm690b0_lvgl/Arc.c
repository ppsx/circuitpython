// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Arc.h"
#include "common-hal/rm690b0_lvgl/Arc.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

// Event callback for arc value changes
static void arc_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *arc = lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        // Get the arc object from user_data
        rm690b0_lvgl_arc_obj_t *self = (rm690b0_lvgl_arc_obj_t *)lv_obj_get_user_data(arc);

        if (self != NULL && self->on_change_handler != mp_const_none) {
            // Call the Python callback
            mp_call_function_1(self->on_change_handler, MP_OBJ_FROM_PTR(self));
        }
    }
}

void common_hal_rm690b0_lvgl_arc_construct(rm690b0_lvgl_arc_obj_t *self, mp_int_t min_value, mp_int_t max_value) {
    // Create LVGL arc on the active screen
    lv_obj_t *arc = lv_arc_create(lv_scr_act());
    if (arc == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL arc"));
    }

    self->native_obj = arc;

    // Set range
    lv_arc_set_range(arc, (int32_t)min_value, (int32_t)max_value);

    // Set initial value to minimum
    lv_arc_set_value(arc, (int32_t)min_value);

    // Store reference to self in user_data for event callbacks
    lv_obj_set_user_data(arc, self);

    // Add event callback for value changes
    lv_obj_add_event_cb(arc, arc_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Set default size (arcs are typically square)
    lv_obj_set_width(arc, 100);
    lv_obj_set_height(arc, 100);
}

void common_hal_rm690b0_lvgl_arc_deinit(rm690b0_lvgl_arc_obj_t *self) {
    if (self->native_obj != NULL) {
        lv_obj_del((lv_obj_t *)self->native_obj);
        self->native_obj = NULL;
    }
}

mp_int_t common_hal_rm690b0_lvgl_arc_get_value(rm690b0_lvgl_arc_obj_t *self) {
    lv_obj_t *arc = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    return (mp_int_t)lv_arc_get_value(arc);
}

void common_hal_rm690b0_lvgl_arc_set_value(rm690b0_lvgl_arc_obj_t *self, mp_int_t value) {
    lv_obj_t *arc = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);

    // Check if value is actually changing
    mp_int_t current_value = common_hal_rm690b0_lvgl_arc_get_value(self);
    if (current_value == value) {
        return;  // No change needed
    }

    lv_arc_set_value(arc, (int32_t)value);

    // Manually trigger the callback when set programmatically
    if (self->on_change_handler != mp_const_none) {
        mp_call_function_1(self->on_change_handler, MP_OBJ_FROM_PTR(self));
    }
}

mp_int_t common_hal_rm690b0_lvgl_arc_get_min_value(rm690b0_lvgl_arc_obj_t *self) {
    lv_obj_t *arc = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    return (mp_int_t)lv_arc_get_min_value(arc);
}

void common_hal_rm690b0_lvgl_arc_set_min_value(rm690b0_lvgl_arc_obj_t *self, mp_int_t min_value) {
    lv_obj_t *arc = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    int32_t max_value = lv_arc_get_max_value(arc);
    lv_arc_set_range(arc, (int32_t)min_value, max_value);
}

mp_int_t common_hal_rm690b0_lvgl_arc_get_max_value(rm690b0_lvgl_arc_obj_t *self) {
    lv_obj_t *arc = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    return (mp_int_t)lv_arc_get_max_value(arc);
}

void common_hal_rm690b0_lvgl_arc_set_max_value(rm690b0_lvgl_arc_obj_t *self, mp_int_t max_value) {
    lv_obj_t *arc = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    int32_t min_value = lv_arc_get_min_value(arc);
    lv_arc_set_range(arc, min_value, (int32_t)max_value);
}

void common_hal_rm690b0_lvgl_arc_set_range(rm690b0_lvgl_arc_obj_t *self, mp_int_t min_value, mp_int_t max_value) {
    lv_obj_t *arc = common_hal_rm690b0_lvgl_widget_get_native_obj((rm690b0_lvgl_widget_obj_t *)self);
    lv_arc_set_range(arc, (int32_t)min_value, (int32_t)max_value);
}
