// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "shared-bindings/rm690b0_lvgl/Font.h"
#include "lvgl.h"

void common_hal_rm690b0_lvgl_widget_set_style_text_font(rm690b0_lvgl_widget_obj_t *self, mp_obj_t font_obj) {
    lv_obj_t *widget = common_hal_rm690b0_lvgl_widget_get_native_obj(self);
    if (widget == NULL) {
        return;
    }

    if (font_obj == mp_const_none) {
        lv_obj_set_style_text_font(widget, LV_FONT_DEFAULT, 0);
        return;
    }

    if (!mp_obj_is_type(font_obj, &rm690b0_lvgl_font_type)) {
        mp_raise_TypeError(MP_ERROR_TEXT("Expected a Font object"));
    }

    rm690b0_lvgl_font_obj_t *font = MP_OBJ_TO_PTR(font_obj);
    if (font->native_font != NULL) {
        lv_obj_set_style_text_font(widget, (lv_font_t *)font->native_font, 0);
    }
}

void common_hal_rm690b0_lvgl_widget_construct(rm690b0_lvgl_widget_obj_t *self) {
    // Default to creating a base object on the active screen
    // Subclasses (like Label) will handle their own construction and set native_obj
    // This construct is called if Widget() is instantiated directly
    self->callback = mp_const_none;
    if (self->native_obj == NULL) {
        self->native_obj = lv_obj_create(lv_scr_act());
        if (self->native_obj == NULL) {
            mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL widget"));
        }
    }
}

void common_hal_rm690b0_lvgl_widget_deinit(rm690b0_lvgl_widget_obj_t *self) {
    if (self->native_obj != NULL) {
        lv_obj_del((lv_obj_t *)self->native_obj);
        self->native_obj = NULL;
    }
}

void common_hal_rm690b0_lvgl_widget_set_parent(rm690b0_lvgl_widget_obj_t *self, rm690b0_lvgl_widget_obj_t *parent) {
    lv_obj_t *obj = common_hal_rm690b0_lvgl_widget_get_native_obj(self);
    lv_obj_t *parent_obj = common_hal_rm690b0_lvgl_widget_get_native_obj(parent);
    lv_obj_set_parent(obj, parent_obj);
}

mp_obj_t common_hal_rm690b0_lvgl_widget_get_callback(mp_obj_t widget) {
    rm690b0_lvgl_widget_obj_t *self = MP_OBJ_TO_PTR(widget);
    if (self->callback == NULL) {
        return mp_const_none;
    }
    return self->callback;
}

void common_hal_rm690b0_lvgl_widget_set_callback(mp_obj_t widget, mp_obj_t callback) {
    if (callback != mp_const_none && !mp_obj_is_callable(callback)) {
        mp_raise_ValueError(MP_ERROR_TEXT("callback must be callable or None"));
    }
    rm690b0_lvgl_widget_obj_t *self = MP_OBJ_TO_PTR(widget);
    self->callback = callback;
}

mp_int_t common_hal_rm690b0_lvgl_widget_get_x(rm690b0_lvgl_widget_obj_t *self) {
    lv_obj_t *obj = common_hal_rm690b0_lvgl_widget_get_native_obj(self);
    return lv_obj_get_x(obj);
}

void common_hal_rm690b0_lvgl_widget_set_x(rm690b0_lvgl_widget_obj_t *self, mp_int_t x) {
    lv_obj_t *obj = common_hal_rm690b0_lvgl_widget_get_native_obj(self);
    lv_obj_set_x(obj, (lv_coord_t)x);
}

mp_int_t common_hal_rm690b0_lvgl_widget_get_y(rm690b0_lvgl_widget_obj_t *self) {
    lv_obj_t *obj = common_hal_rm690b0_lvgl_widget_get_native_obj(self);
    return lv_obj_get_y(obj);
}

void common_hal_rm690b0_lvgl_widget_set_y(rm690b0_lvgl_widget_obj_t *self, mp_int_t y) {
    lv_obj_t *obj = common_hal_rm690b0_lvgl_widget_get_native_obj(self);
    lv_obj_set_y(obj, (lv_coord_t)y);
}

mp_int_t common_hal_rm690b0_lvgl_widget_get_width(rm690b0_lvgl_widget_obj_t *self) {
    lv_obj_t *obj = common_hal_rm690b0_lvgl_widget_get_native_obj(self);
    return lv_obj_get_width(obj);
}

void common_hal_rm690b0_lvgl_widget_set_width(rm690b0_lvgl_widget_obj_t *self, mp_int_t width) {
    lv_obj_t *obj = common_hal_rm690b0_lvgl_widget_get_native_obj(self);
    lv_obj_set_width(obj, (lv_coord_t)width);
}

mp_int_t common_hal_rm690b0_lvgl_widget_get_height(rm690b0_lvgl_widget_obj_t *self) {
    lv_obj_t *obj = common_hal_rm690b0_lvgl_widget_get_native_obj(self);
    return lv_obj_get_height(obj);
}

void common_hal_rm690b0_lvgl_widget_set_height(rm690b0_lvgl_widget_obj_t *self, mp_int_t height) {
    lv_obj_t *obj = common_hal_rm690b0_lvgl_widget_get_native_obj(self);
    lv_obj_set_height(obj, (lv_coord_t)height);
}

void common_hal_rm690b0_lvgl_widget_set_style_bg_color(rm690b0_lvgl_widget_obj_t *self, uint32_t color) {
    lv_obj_t *obj = common_hal_rm690b0_lvgl_widget_get_native_obj(self);
    lv_obj_set_style_bg_color(obj, lv_color_hex(color), 0);
}

void common_hal_rm690b0_lvgl_widget_set_style_bg_opa(rm690b0_lvgl_widget_obj_t *self, uint8_t opa) {
    lv_obj_t *obj = common_hal_rm690b0_lvgl_widget_get_native_obj(self);
    lv_obj_set_style_bg_opa(obj, (lv_opa_t)opa, 0);
}

void common_hal_rm690b0_lvgl_widget_set_style_text_color(rm690b0_lvgl_widget_obj_t *self, uint32_t color) {
    lv_obj_t *obj = common_hal_rm690b0_lvgl_widget_get_native_obj(self);
    lv_obj_set_style_text_color(obj, lv_color_hex(color), 0);
}