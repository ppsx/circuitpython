// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Checkbox.h"
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

//| class Checkbox(Widget):
//|     """A checkbox widget for toggling options on/off."""
//|
//|     def __init__(self, text: str = "Checkbox") -> None:
//|         """Create a new Checkbox widget.
//|
//|         :param str text: The text label for the checkbox (default: "Checkbox")
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_checkbox_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_text };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_text, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rm690b0_lvgl_checkbox_obj_t *self = mp_obj_malloc(rm690b0_lvgl_checkbox_obj_t, &rm690b0_lvgl_checkbox_type);

    // Initialize callback to None
    self->on_change_handler = mp_const_none;

    const char *text = "Checkbox";
    if (args[ARG_text].u_obj != MP_OBJ_NULL) {
        text = mp_obj_str_get_str(args[ARG_text].u_obj);
    }

    common_hal_rm690b0_lvgl_checkbox_construct(self, text);
    return MP_OBJ_FROM_PTR(self);
}

//|     checked: bool
//|     """The checked state of the checkbox (True/False)."""
//|
static mp_obj_t rm690b0_lvgl_checkbox_get_checked(mp_obj_t self_in) {
    rm690b0_lvgl_checkbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(common_hal_rm690b0_lvgl_checkbox_get_checked(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_checkbox_get_checked_obj, rm690b0_lvgl_checkbox_get_checked);

static mp_obj_t rm690b0_lvgl_checkbox_set_checked(mp_obj_t self_in, mp_obj_t checked_obj) {
    rm690b0_lvgl_checkbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    bool checked = mp_obj_is_true(checked_obj);
    common_hal_rm690b0_lvgl_checkbox_set_checked(self, checked);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_checkbox_set_checked_obj, rm690b0_lvgl_checkbox_set_checked);

MP_PROPERTY_GETSET(rm690b0_lvgl_checkbox_checked_obj,
    (mp_obj_t)&rm690b0_lvgl_checkbox_get_checked_obj,
    (mp_obj_t)&rm690b0_lvgl_checkbox_set_checked_obj);

//|     text: str
//|     """The text label displayed next to the checkbox."""
//|
static mp_obj_t rm690b0_lvgl_checkbox_get_text(mp_obj_t self_in) {
    rm690b0_lvgl_checkbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *text = common_hal_rm690b0_lvgl_checkbox_get_text(self);
    return mp_obj_new_str(text, strlen(text));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_checkbox_get_text_obj, rm690b0_lvgl_checkbox_get_text);

static mp_obj_t rm690b0_lvgl_checkbox_set_text(mp_obj_t self_in, mp_obj_t text_obj) {
    rm690b0_lvgl_checkbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *text = mp_obj_str_get_str(text_obj);
    common_hal_rm690b0_lvgl_checkbox_set_text(self, text);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_checkbox_set_text_obj, rm690b0_lvgl_checkbox_set_text);

MP_PROPERTY_GETSET(rm690b0_lvgl_checkbox_text_obj,
    (mp_obj_t)&rm690b0_lvgl_checkbox_get_text_obj,
    (mp_obj_t)&rm690b0_lvgl_checkbox_set_text_obj);

//|     on_change: Optional[Callable[[Checkbox], None]]
//|     """The callback function to run when the checkbox state changes.
//|     The callback receives the checkbox instance as an argument."""
//|
static mp_obj_t rm690b0_lvgl_checkbox_get_on_change(mp_obj_t self_in) {
    rm690b0_lvgl_checkbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return self->on_change_handler;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_checkbox_get_on_change_obj, rm690b0_lvgl_checkbox_get_on_change);

static mp_obj_t rm690b0_lvgl_checkbox_set_on_change(mp_obj_t self_in, mp_obj_t callback) {
    rm690b0_lvgl_checkbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (callback != mp_const_none && !mp_obj_is_callable(callback)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Callback must be callable or None"));
    }
    self->on_change_handler = callback;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_checkbox_set_on_change_obj, rm690b0_lvgl_checkbox_set_on_change);

MP_PROPERTY_GETSET(rm690b0_lvgl_checkbox_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_checkbox_get_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_checkbox_set_on_change_obj);

//|     def toggle(self) -> None:
//|         """Toggle the checkbox state (checked <-> unchecked)."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_checkbox_toggle(mp_obj_t self_in) {
    rm690b0_lvgl_checkbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    bool current = common_hal_rm690b0_lvgl_checkbox_get_checked(self);
    common_hal_rm690b0_lvgl_checkbox_set_checked(self, !current);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_checkbox_toggle_obj, rm690b0_lvgl_checkbox_toggle);

static const mp_rom_map_elem_t rm690b0_lvgl_checkbox_locals_dict_table[] = {
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

    // Checkbox-specific properties
    { MP_ROM_QSTR(MP_QSTR_checked), MP_ROM_PTR(&rm690b0_lvgl_checkbox_checked_obj) },
    { MP_ROM_QSTR(MP_QSTR_text), MP_ROM_PTR(&rm690b0_lvgl_checkbox_text_obj) },
    { MP_ROM_QSTR(MP_QSTR_on_change), MP_ROM_PTR(&rm690b0_lvgl_checkbox_on_change_obj) },

    // Checkbox-specific methods
    { MP_ROM_QSTR(MP_QSTR_toggle), MP_ROM_PTR(&rm690b0_lvgl_checkbox_toggle_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_checkbox_locals_dict, rm690b0_lvgl_checkbox_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_checkbox_type,
    MP_QSTR_Checkbox,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_checkbox_make_new,
    locals_dict, &rm690b0_lvgl_checkbox_locals_dict,
    parent, &rm690b0_lvgl_widget_type
);
