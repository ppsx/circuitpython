// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/List.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"

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

//| class List(Widget):
//|     """A scrollable list widget."""
//|
//|     def __init__(self) -> None:
//|         """Create a new List widget."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_list_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    rm690b0_lvgl_list_obj_t *self = mp_obj_malloc(rm690b0_lvgl_list_obj_t, &rm690b0_lvgl_list_type);
    common_hal_rm690b0_lvgl_list_construct(self);
    return MP_OBJ_FROM_PTR(self);
}

//|     def add_btn(self, text: str, icon: Optional[str] = None) -> Button:
//|         """Add a button to the list.
//|
//|         :param str text: The text to display on the button
//|         :param str icon: Optional icon symbol (e.g. lvgl.SYMBOL.OK)
//|         :return: The created Button object
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_list_add_btn(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_text, ARG_icon };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_text, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_icon, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    rm690b0_lvgl_list_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    const char *text = mp_obj_str_get_str(args[ARG_text].u_obj);
    const char *icon = NULL;
    if (args[ARG_icon].u_obj != mp_const_none) {
        icon = mp_obj_str_get_str(args[ARG_icon].u_obj);
    }

    return common_hal_rm690b0_lvgl_list_add_btn(self, icon, text);
}
static MP_DEFINE_CONST_FUN_OBJ_KW(rm690b0_lvgl_list_add_btn_obj, 1, rm690b0_lvgl_list_add_btn);

//|     def add_text(self, text: str) -> Label:
//|         """Add a text label to the list.
//|
//|         :param str text: The text to display
//|         :return: The created Label object
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_list_add_text(mp_obj_t self_in, mp_obj_t text_obj) {
    rm690b0_lvgl_list_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *text = mp_obj_str_get_str(text_obj);
    return common_hal_rm690b0_lvgl_list_add_text(self, text);
}
static MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_list_add_text_obj, rm690b0_lvgl_list_add_text);

static const mp_rom_map_elem_t rm690b0_lvgl_list_locals_dict_table[] = {
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

    // List methods
    { MP_ROM_QSTR(MP_QSTR_add_btn), MP_ROM_PTR(&rm690b0_lvgl_list_add_btn_obj) },
    { MP_ROM_QSTR(MP_QSTR_add_text), MP_ROM_PTR(&rm690b0_lvgl_list_add_text_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_list_locals_dict, rm690b0_lvgl_list_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_list_type,
    MP_QSTR_List,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_list_make_new,
    locals_dict, &rm690b0_lvgl_list_locals_dict,
    parent, &rm690b0_lvgl_widget_type
);
