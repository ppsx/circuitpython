// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"

//| class Widget:
//|     """Base class for LVGL widgets."""
//|
//|     def __init__(self) -> None:
//|         """Create a new Widget. This is usually not called directly,
//|         but through subclasses like Label or Button."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_widget_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    // Allow creating base widgets for containers/panels
    rm690b0_lvgl_widget_obj_t *self = mp_obj_malloc(rm690b0_lvgl_widget_obj_t, &rm690b0_lvgl_widget_type);
    self->callback = mp_const_none;
    common_hal_rm690b0_lvgl_widget_construct(self);
    return MP_OBJ_FROM_PTR(self);
}

//|     x: int
//|     """The x-coordinate of the widget relative to its parent."""
//|
static mp_obj_t rm690b0_lvgl_widget_get_x(mp_obj_t self_in) {
    rm690b0_lvgl_widget_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_widget_get_x(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_widget_get_x_obj, rm690b0_lvgl_widget_get_x);

static mp_obj_t rm690b0_lvgl_widget_set_x(mp_obj_t self_in, mp_obj_t x_obj) {
    rm690b0_lvgl_widget_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_widget_set_x(self, mp_obj_get_int(x_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_widget_set_x_obj, rm690b0_lvgl_widget_set_x);

MP_PROPERTY_GETSET(rm690b0_lvgl_widget_x_obj,
    (mp_obj_t)&rm690b0_lvgl_widget_get_x_obj,
    (mp_obj_t)&rm690b0_lvgl_widget_set_x_obj);

//|     y: int
//|     """The y-coordinate of the widget relative to its parent."""
//|
static mp_obj_t rm690b0_lvgl_widget_get_y(mp_obj_t self_in) {
    rm690b0_lvgl_widget_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_widget_get_y(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_widget_get_y_obj, rm690b0_lvgl_widget_get_y);

static mp_obj_t rm690b0_lvgl_widget_set_y(mp_obj_t self_in, mp_obj_t y_obj) {
    rm690b0_lvgl_widget_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_widget_set_y(self, mp_obj_get_int(y_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_widget_set_y_obj, rm690b0_lvgl_widget_set_y);

MP_PROPERTY_GETSET(rm690b0_lvgl_widget_y_obj,
    (mp_obj_t)&rm690b0_lvgl_widget_get_y_obj,
    (mp_obj_t)&rm690b0_lvgl_widget_set_y_obj);

//|     width: int
//|     """The width of the widget in pixels."""
//|
static mp_obj_t rm690b0_lvgl_widget_get_width(mp_obj_t self_in) {
    rm690b0_lvgl_widget_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_widget_get_width(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_widget_get_width_obj, rm690b0_lvgl_widget_get_width);

static mp_obj_t rm690b0_lvgl_widget_set_width(mp_obj_t self_in, mp_obj_t width_obj) {
    rm690b0_lvgl_widget_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_widget_set_width(self, mp_obj_get_int(width_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_widget_set_width_obj, rm690b0_lvgl_widget_set_width);

MP_PROPERTY_GETSET(rm690b0_lvgl_widget_width_obj,
    (mp_obj_t)&rm690b0_lvgl_widget_get_width_obj,
    (mp_obj_t)&rm690b0_lvgl_widget_set_width_obj);

//|     height: int
//|     """The height of the widget in pixels."""
//|
static mp_obj_t rm690b0_lvgl_widget_get_height(mp_obj_t self_in) {
    rm690b0_lvgl_widget_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_widget_get_height(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_widget_get_height_obj, rm690b0_lvgl_widget_get_height);

static mp_obj_t rm690b0_lvgl_widget_set_height(mp_obj_t self_in, mp_obj_t height_obj) {
    rm690b0_lvgl_widget_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_widget_set_height(self, mp_obj_get_int(height_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_widget_set_height_obj, rm690b0_lvgl_widget_set_height);

MP_PROPERTY_GETSET(rm690b0_lvgl_widget_height_obj,
    (mp_obj_t)&rm690b0_lvgl_widget_get_height_obj,
    (mp_obj_t)&rm690b0_lvgl_widget_set_height_obj);

//|     def set_style_bg_color(self, color: int) -> None:
//|         """Set the background color of the widget (RGB888 integer)."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_widget_set_style_bg_color(mp_obj_t self_in, mp_obj_t color_obj) {
    rm690b0_lvgl_widget_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_widget_set_style_bg_color(self, mp_obj_get_int(color_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_widget_set_style_bg_color_obj, rm690b0_lvgl_widget_set_style_bg_color);

static mp_obj_t rm690b0_lvgl_widget_set_style_text_color(mp_obj_t self_in, mp_obj_t color_obj) {
    rm690b0_lvgl_widget_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_widget_set_style_text_color(self, mp_obj_get_int(color_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_widget_set_style_text_color_obj, rm690b0_lvgl_widget_set_style_text_color);

//|     def set_style_text_font(self, font: Font) -> None:
//|         """Set the text font of the widget."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_widget_set_style_text_font(mp_obj_t self_in, mp_obj_t font_obj) {
    rm690b0_lvgl_widget_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_widget_set_style_text_font(self, font_obj);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_widget_set_style_text_font_obj, rm690b0_lvgl_widget_set_style_text_font);

//|     def set_style_bg_opa(self, opa: int) -> None:
//|         """Set the background opacity of the widget (0-255)."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_widget_set_style_bg_opa(mp_obj_t self_in, mp_obj_t opa_obj) {
    rm690b0_lvgl_widget_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_widget_set_style_bg_opa(self, mp_obj_get_int(opa_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_widget_set_style_bg_opa_obj, rm690b0_lvgl_widget_set_style_bg_opa);

//|     def set_parent(self, parent: Widget) -> None:
//|         """Set the parent of the widget."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_widget_set_parent(mp_obj_t self_in, mp_obj_t parent_obj) {
    rm690b0_lvgl_widget_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const mp_obj_type_t *parent_type = mp_obj_get_type(parent_obj);
    if (!mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(parent_type), MP_OBJ_FROM_PTR(&rm690b0_lvgl_widget_type))) {
        mp_raise_TypeError_varg(MP_ERROR_TEXT("Expected a %q"), rm690b0_lvgl_widget_type.name);
    }
    rm690b0_lvgl_widget_obj_t *parent = MP_OBJ_TO_PTR(parent_obj);
    common_hal_rm690b0_lvgl_widget_set_parent(self, parent);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_widget_set_parent_obj, rm690b0_lvgl_widget_set_parent);

static mp_obj_t rm690b0_lvgl_widget_delete(mp_obj_t self_in) {
    rm690b0_lvgl_widget_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_widget_deinit(self);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_widget_delete_obj, rm690b0_lvgl_widget_delete);

static const mp_rom_map_elem_t rm690b0_lvgl_widget_locals_dict_table[] = {
    // Properties
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&rm690b0_lvgl_widget_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&rm690b0_lvgl_widget_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&rm690b0_lvgl_widget_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&rm690b0_lvgl_widget_height_obj) },
    
    // Methods
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_color), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_color_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_text_color), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_text_color_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_text_font), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_text_font_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_opa), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_opa_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_parent), MP_ROM_PTR(&rm690b0_lvgl_widget_set_parent_obj) },
    { MP_ROM_QSTR(MP_QSTR_delete), MP_ROM_PTR(&rm690b0_lvgl_widget_delete_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_widget_locals_dict, rm690b0_lvgl_widget_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_widget_type,
    MP_QSTR_Widget,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_widget_make_new,
    locals_dict, &rm690b0_lvgl_widget_locals_dict
);
