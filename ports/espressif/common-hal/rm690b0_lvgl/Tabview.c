// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Tabview.h"
#include "shared-bindings/rm690b0_lvgl/Container.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

static void tabview_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    rm690b0_lvgl_tabview_obj_t *self = (rm690b0_lvgl_tabview_obj_t *)lv_event_get_user_data(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        if (self->on_change_handler != mp_const_none) {
            mp_call_function_1(self->on_change_handler, MP_OBJ_FROM_PTR(self));
        }
    }
}

void common_hal_rm690b0_lvgl_tabview_construct(rm690b0_lvgl_tabview_obj_t *self, mp_int_t tab_pos, mp_int_t tab_size) {
    lv_obj_t *tv = lv_tabview_create(lv_scr_act(), (lv_dir_t)tab_pos, (lv_coord_t)tab_size);
    if (tv == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL tabview"));
        return;
    }

    lv_obj_add_event_cb(tv, tabview_event_handler, LV_EVENT_VALUE_CHANGED, self);

    self->base.native_obj = tv;
    self->base.callback = mp_const_none;
}

mp_obj_t common_hal_rm690b0_lvgl_tabview_add_tab(rm690b0_lvgl_tabview_obj_t *self, const char *name) {
    lv_obj_t *tv = (lv_obj_t *)self->base.native_obj;
    lv_obj_t *tab = lv_tabview_add_tab(tv, name);

    if (tab == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create tab"));
        return mp_const_none;
    }

    // Create Python Container wrapper
    rm690b0_lvgl_container_obj_t *py_cont = mp_obj_malloc(rm690b0_lvgl_container_obj_t, &rm690b0_lvgl_container_type);

    // Initialize Widget base
    py_cont->base.native_obj = tab;
    py_cont->base.callback = mp_const_none;

    return MP_OBJ_FROM_PTR(py_cont);
}

uint16_t common_hal_rm690b0_lvgl_tabview_get_active_tab(rm690b0_lvgl_tabview_obj_t *self) {
    lv_obj_t *tv = (lv_obj_t *)self->base.native_obj;
    return lv_tabview_get_tab_act(tv);
}

void common_hal_rm690b0_lvgl_tabview_set_active_tab(rm690b0_lvgl_tabview_obj_t *self, uint16_t idx) {
    lv_obj_t *tv = (lv_obj_t *)self->base.native_obj;
    lv_tabview_set_act(tv, idx, LV_ANIM_OFF);
}
