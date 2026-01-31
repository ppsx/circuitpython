// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Label.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

void common_hal_rm690b0_lvgl_label_construct(rm690b0_lvgl_label_obj_t *self, const char *text) {
    lv_obj_t *label = lv_label_create(lv_scr_act());
    if (label == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL label"));
        return;
    }
    lv_label_set_text(label, text);
    self->base.native_obj = label;
    self->base.callback = mp_const_none;
}

void common_hal_rm690b0_lvgl_label_set_text(rm690b0_lvgl_label_obj_t *self, const char *text) {
    lv_obj_t *label = (lv_obj_t *)self->base.native_obj;
    lv_label_set_text(label, text);
}

const char *common_hal_rm690b0_lvgl_label_get_text(rm690b0_lvgl_label_obj_t *self) {
    lv_obj_t *label = (lv_obj_t *)self->base.native_obj;
    return lv_label_get_text(label);
}

void common_hal_rm690b0_lvgl_label_set_text_color(rm690b0_lvgl_label_obj_t *self, uint32_t color) {
    lv_obj_t *label = (lv_obj_t *)self->base.native_obj;
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
}
