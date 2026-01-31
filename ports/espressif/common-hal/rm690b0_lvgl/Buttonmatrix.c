// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Buttonmatrix.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

static void buttonmatrix_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    rm690b0_lvgl_buttonmatrix_obj_t *self = (rm690b0_lvgl_buttonmatrix_obj_t *)lv_event_get_user_data(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        if (self->on_click_handler != mp_const_none) {
            mp_call_function_1(self->on_click_handler, MP_OBJ_FROM_PTR(self));
        }
    }
}

void common_hal_rm690b0_lvgl_buttonmatrix_construct(rm690b0_lvgl_buttonmatrix_obj_t *self, mp_obj_t buttons) {
    lv_obj_t *btnm = lv_btnmatrix_create(lv_scr_act());
    if (btnm == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL buttonmatrix"));
        return;
    }

    lv_obj_add_event_cb(btnm, buttonmatrix_event_handler, LV_EVENT_VALUE_CHANGED, self);

    self->base.native_obj = btnm;
    self->base.callback = mp_const_none;

    if (buttons != mp_const_none) {
        common_hal_rm690b0_lvgl_buttonmatrix_set_map(self, buttons);
    }
}

void common_hal_rm690b0_lvgl_buttonmatrix_set_map(rm690b0_lvgl_buttonmatrix_obj_t *self, mp_obj_t buttons) {
    size_t btn_count;
    mp_obj_t *btn_items;
    mp_obj_get_array(buttons, &btn_count, &btn_items);

    // Free previous map if it exists? We rely on Python GC, but btn_map pointer is managed by us?
    // Wait, if we use m_malloc, we should free it.
    // However, if we store the new map pointer, we should free the old one if it was allocated by us.
    // Assuming self->btn_map was initialized to NULL or points to an array we own.
    // The previous implementation for Msgbox used m_malloc but relied on object destruction to free?
    // Actually, Msgbox keeps it until closed.
    // For Buttonmatrix, map can change. So we should realloc or free/alloc.
    // But `rm690b0_lvgl_buttonmatrix_obj_t` struct layout is in shared bindings.

    if (self->btn_map != NULL) {
        // m_free(self->btn_map); // If we track allocation. m_malloc is GC heap?
        // Actually m_malloc returns GC managed memory. If we overwrite the pointer, the old block becomes unreachable (unless LVGL holds it).
        // LVGL DOES NOT copy the map. It holds the pointer.
        // So we must ensure the old map is NOT freed while LVGL uses it.
        // But we are setting a new map. LVGL will use the new map pointer.
        // So the old map is no longer used by LVGL.
        // And if we drop the pointer, GC will reclaim it.
    }

    self->buttons_list = buttons; // Keep Python objects alive

    // Allocate array for pointers. +1 for terminator.
    self->btn_map = m_malloc((btn_count + 1) * sizeof(char *));

    for (size_t i = 0; i < btn_count; i++) {
        const char *s = mp_obj_str_get_str(btn_items[i]);
        // Handle "\n" in string? LVGL uses "\n" in the array to break lines?
        // Actually LVGL expects the string itself to be "\n" to break line.
        // So if user passed "\n" string in list, it works.
        self->btn_map[i] = s;
    }
    // Ensure null termination (empty string for LVGL btnmatrix map)
    self->btn_map[btn_count] = "";

    lv_btnmatrix_set_map((lv_obj_t *)self->base.native_obj, (const char **)self->btn_map);
}

uint16_t common_hal_rm690b0_lvgl_buttonmatrix_get_selected_btn(rm690b0_lvgl_buttonmatrix_obj_t *self) {
    lv_obj_t *btnm = (lv_obj_t *)self->base.native_obj;
    return lv_btnmatrix_get_selected_btn(btnm);
}

void common_hal_rm690b0_lvgl_buttonmatrix_set_selected_btn(rm690b0_lvgl_buttonmatrix_obj_t *self, uint16_t btn_id) {
    lv_obj_t *btnm = (lv_obj_t *)self->base.native_obj;
    lv_btnmatrix_set_selected_btn(btnm, btn_id);
}
