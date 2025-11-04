// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Msgbox.h"
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

//| class Msgbox(Widget):
//|     """A modal message box widget."""
//|
//|     def __init__(self, title: str, text: str, buttons: List[str], close_btn: bool = False) -> None:
//|         """Create a new Msgbox widget.
//|
//|         :param str title: The title of the message box
//|         :param str text: The text of the message box
//|         :param List[str] buttons: List of button texts, must end with empty string ""
//|         :param bool close_btn: Whether to add a close button
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_msgbox_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_title, ARG_text, ARG_buttons, ARG_close_btn };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_title, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_text, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_buttons, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_close_btn, MP_ARG_BOOL, {.u_bool = false} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rm690b0_lvgl_msgbox_obj_t *self = mp_obj_malloc(rm690b0_lvgl_msgbox_obj_t, &rm690b0_lvgl_msgbox_type);
    
    // Initialize handler to None
    self->on_click_handler = mp_const_none;
    self->buttons_list = args[ARG_buttons].u_obj;
    self->btn_map = NULL;

    const char *title = mp_obj_str_get_str(args[ARG_title].u_obj);
    const char *text = mp_obj_str_get_str(args[ARG_text].u_obj);
    mp_obj_t buttons = args[ARG_buttons].u_obj;
    bool close_btn = args[ARG_close_btn].u_bool;
    
    common_hal_rm690b0_lvgl_msgbox_construct(self, title, text, buttons, close_btn);
    return MP_OBJ_FROM_PTR(self);
}

//|     def close(self) -> None:
//|         """Close and delete the message box."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_msgbox_close(mp_obj_t self_in) {
    rm690b0_lvgl_msgbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_msgbox_close(self);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_msgbox_close_obj, rm690b0_lvgl_msgbox_close);

//|     on_click: Optional[Callable[[int], None]]
//|     """The callback function to run when a button is clicked.
//|     The callback receives the button index as an argument."""
//|
static mp_obj_t rm690b0_lvgl_msgbox_get_on_click(mp_obj_t self_in) {
    rm690b0_lvgl_msgbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return self->on_click_handler;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_msgbox_get_on_click_obj, rm690b0_lvgl_msgbox_get_on_click);

static mp_obj_t rm690b0_lvgl_msgbox_set_on_click(mp_obj_t self_in, mp_obj_t callback) {
    rm690b0_lvgl_msgbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (callback != mp_const_none && !mp_obj_is_callable(callback)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Callback must be callable or None"));
    }
    self->on_click_handler = callback;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_msgbox_set_on_click_obj, rm690b0_lvgl_msgbox_set_on_click);

MP_PROPERTY_GETSET(rm690b0_lvgl_msgbox_on_click_obj,
    (mp_obj_t)&rm690b0_lvgl_msgbox_get_on_click_obj,
    (mp_obj_t)&rm690b0_lvgl_msgbox_set_on_click_obj);

static const mp_rom_map_elem_t rm690b0_lvgl_msgbox_locals_dict_table[] = {
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

    // Msgbox-specific properties/methods
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&rm690b0_lvgl_msgbox_close_obj) },
    { MP_ROM_QSTR(MP_QSTR_on_click), MP_ROM_PTR(&rm690b0_lvgl_msgbox_on_click_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_msgbox_locals_dict, rm690b0_lvgl_msgbox_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_msgbox_type,
    MP_QSTR_Msgbox,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_msgbox_make_new,
    locals_dict, &rm690b0_lvgl_msgbox_locals_dict,
    parent, &rm690b0_lvgl_widget_type
);