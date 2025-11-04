// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Label.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"
#include <string.h>

// Import Widget property objects for inheritance
extern const mp_obj_property_t rm690b0_lvgl_widget_x_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_y_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_width_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_height_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_color_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_text_font_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_opa_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_parent_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_delete_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_parent_obj;

//| class Label(Widget):
//|     """A widget that displays text."""
//|
//|     def __init__(self, text: str = "Label") -> None:
//|         """Create a new Label widget.
//|
//|         :param str text: The initial text to display
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_label_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_text };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_text, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rm690b0_lvgl_label_obj_t *self = mp_obj_malloc(rm690b0_lvgl_label_obj_t, &rm690b0_lvgl_label_type);
    
    const char *text = "Label";
    if (args[ARG_text].u_obj != MP_OBJ_NULL) {
        text = mp_obj_str_get_str(args[ARG_text].u_obj);
    }
    
    common_hal_rm690b0_lvgl_label_construct(self, text);
    return MP_OBJ_FROM_PTR(self);
}

//|     text: str
//|     """The text displayed by the label."""
//|
static mp_obj_t rm690b0_lvgl_label_get_text(mp_obj_t self_in) {
    rm690b0_lvgl_label_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *text = common_hal_rm690b0_lvgl_label_get_text(self);
    return mp_obj_new_str(text, strlen(text));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_label_get_text_obj, rm690b0_lvgl_label_get_text);

static mp_obj_t rm690b0_lvgl_label_set_text(mp_obj_t self_in, mp_obj_t text_obj) {
    rm690b0_lvgl_label_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *text = mp_obj_str_get_str(text_obj);
    common_hal_rm690b0_lvgl_label_set_text(self, text);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_label_set_text_obj, rm690b0_lvgl_label_set_text);

MP_PROPERTY_GETSET(rm690b0_lvgl_label_text_obj,
    (mp_obj_t)&rm690b0_lvgl_label_get_text_obj,
    (mp_obj_t)&rm690b0_lvgl_label_set_text_obj);

//|     def set_text_color(self, color: int) -> None:
//|         """Set the text color (RGB888 integer)."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_label_set_text_color(mp_obj_t self_in, mp_obj_t color_obj) {
    rm690b0_lvgl_label_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_label_set_text_color(self, mp_obj_get_int(color_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_label_set_text_color_obj, rm690b0_lvgl_label_set_text_color);

static const mp_rom_map_elem_t rm690b0_lvgl_label_locals_dict_table[] = {
    // Inherited Widget properties
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&rm690b0_lvgl_widget_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&rm690b0_lvgl_widget_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&rm690b0_lvgl_widget_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&rm690b0_lvgl_widget_height_obj) },
    
    // Inherited Widget methods
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_color), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_color_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_text_font), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_text_font_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_opa), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_opa_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_parent), MP_ROM_PTR(&rm690b0_lvgl_widget_set_parent_obj) },
    { MP_ROM_QSTR(MP_QSTR_delete), MP_ROM_PTR(&rm690b0_lvgl_widget_delete_obj) },

    // Label-specific properties
    { MP_ROM_QSTR(MP_QSTR_text), MP_ROM_PTR(&rm690b0_lvgl_label_text_obj) },
    
    // Label-specific methods
    { MP_ROM_QSTR(MP_QSTR_set_text_color), MP_ROM_PTR(&rm690b0_lvgl_label_set_text_color_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_label_locals_dict, rm690b0_lvgl_label_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_label_type,
    MP_QSTR_Label,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_label_make_new,
    locals_dict, &rm690b0_lvgl_label_locals_dict,
    parent, &rm690b0_lvgl_widget_type
);