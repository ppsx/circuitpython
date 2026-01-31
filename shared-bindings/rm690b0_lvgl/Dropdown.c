// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Dropdown.h"
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

//| class Dropdown(Widget):
//|     """A dropdown menu widget for selecting options from a list."""
//|
//|     def __init__(self, options: str = "Option 1\nOption 2\nOption 3") -> None:
//|         """Create a new Dropdown widget.
//|
//|         :param str options: Newline-separated list of options (default: "Option 1\nOption 2\nOption 3")
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_dropdown_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_options };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_options, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rm690b0_lvgl_dropdown_obj_t *self = mp_obj_malloc(rm690b0_lvgl_dropdown_obj_t, &rm690b0_lvgl_dropdown_type);

    // Initialize callback to None
    self->on_change_handler = mp_const_none;

    const char *options = "Option 1\nOption 2\nOption 3";
    if (args[ARG_options].u_obj != MP_OBJ_NULL) {
        options = mp_obj_str_get_str(args[ARG_options].u_obj);
    }

    common_hal_rm690b0_lvgl_dropdown_construct(self, options);
    return MP_OBJ_FROM_PTR(self);
}

//|     selected: int
//|     """The index of the currently selected option (0-based)."""
//|
static mp_obj_t rm690b0_lvgl_dropdown_get_selected(mp_obj_t self_in) {
    rm690b0_lvgl_dropdown_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(common_hal_rm690b0_lvgl_dropdown_get_selected(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_dropdown_get_selected_obj, rm690b0_lvgl_dropdown_get_selected);

static mp_obj_t rm690b0_lvgl_dropdown_set_selected(mp_obj_t self_in, mp_obj_t index_obj) {
    rm690b0_lvgl_dropdown_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t index = mp_obj_get_int(index_obj);
    common_hal_rm690b0_lvgl_dropdown_set_selected(self, index);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_dropdown_set_selected_obj, rm690b0_lvgl_dropdown_set_selected);

MP_PROPERTY_GETSET(rm690b0_lvgl_dropdown_selected_obj,
    (mp_obj_t)&rm690b0_lvgl_dropdown_get_selected_obj,
    (mp_obj_t)&rm690b0_lvgl_dropdown_set_selected_obj);

//|     text: str
//|     """The text of the currently selected option (read-only)."""
//|
static mp_obj_t rm690b0_lvgl_dropdown_get_text(mp_obj_t self_in) {
    rm690b0_lvgl_dropdown_obj_t *self = MP_OBJ_TO_PTR(self_in);
    char buf[256];
    common_hal_rm690b0_lvgl_dropdown_get_text(self, buf, sizeof(buf));
    return mp_obj_new_str(buf, strlen(buf));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_dropdown_get_text_obj, rm690b0_lvgl_dropdown_get_text);

MP_PROPERTY_GETTER(rm690b0_lvgl_dropdown_text_obj,
    (mp_obj_t)&rm690b0_lvgl_dropdown_get_text_obj);

//|     on_change: Optional[Callable[[Dropdown], None]]
//|     """The callback function to run when the selection changes.
//|     The callback receives the dropdown instance as an argument."""
//|
static mp_obj_t rm690b0_lvgl_dropdown_get_on_change(mp_obj_t self_in) {
    rm690b0_lvgl_dropdown_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return self->on_change_handler;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_dropdown_get_on_change_obj, rm690b0_lvgl_dropdown_get_on_change);

static mp_obj_t rm690b0_lvgl_dropdown_set_on_change(mp_obj_t self_in, mp_obj_t callback) {
    rm690b0_lvgl_dropdown_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (callback != mp_const_none && !mp_obj_is_callable(callback)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Callback must be callable or None"));
    }
    self->on_change_handler = callback;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_dropdown_set_on_change_obj, rm690b0_lvgl_dropdown_set_on_change);

MP_PROPERTY_GETSET(rm690b0_lvgl_dropdown_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_dropdown_get_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_dropdown_set_on_change_obj);

//|     def set_options(self, options: str) -> None:
//|         """Set the dropdown options as a newline-separated string.
//|
//|         :param str options: Newline-separated list of options (e.g., "Red\nGreen\nBlue")
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_dropdown_set_options(mp_obj_t self_in, mp_obj_t options_obj) {
    rm690b0_lvgl_dropdown_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *options = mp_obj_str_get_str(options_obj);
    common_hal_rm690b0_lvgl_dropdown_set_options(self, options);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_dropdown_set_options_obj, rm690b0_lvgl_dropdown_set_options);

//|     def clear_options(self) -> None:
//|         """Clear all options from the dropdown."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_dropdown_clear_options(mp_obj_t self_in) {
    rm690b0_lvgl_dropdown_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_dropdown_clear_options(self);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_dropdown_clear_options_obj, rm690b0_lvgl_dropdown_clear_options);

//|     def add_option(self, option: str, pos: int = -1) -> None:
//|         """Add a single option to the dropdown.
//|
//|         :param str option: The option text to add
//|         :param int pos: Position to insert (-1 = end, default: -1)
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_dropdown_add_option(size_t n_args, const mp_obj_t *args) {
    rm690b0_lvgl_dropdown_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    const char *option = mp_obj_str_get_str(args[1]);
    mp_int_t pos = (n_args > 2) ? mp_obj_get_int(args[2]) : -1;
    common_hal_rm690b0_lvgl_dropdown_add_option(self, option, pos);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rm690b0_lvgl_dropdown_add_option_obj, 2, 3, rm690b0_lvgl_dropdown_add_option);

static const mp_rom_map_elem_t rm690b0_lvgl_dropdown_locals_dict_table[] = {
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

    // Dropdown-specific properties
    { MP_ROM_QSTR(MP_QSTR_selected), MP_ROM_PTR(&rm690b0_lvgl_dropdown_selected_obj) },
    { MP_ROM_QSTR(MP_QSTR_text), MP_ROM_PTR(&rm690b0_lvgl_dropdown_text_obj) },
    { MP_ROM_QSTR(MP_QSTR_on_change), MP_ROM_PTR(&rm690b0_lvgl_dropdown_on_change_obj) },

    // Dropdown-specific methods
    { MP_ROM_QSTR(MP_QSTR_set_options), MP_ROM_PTR(&rm690b0_lvgl_dropdown_set_options_obj) },
    { MP_ROM_QSTR(MP_QSTR_clear_options), MP_ROM_PTR(&rm690b0_lvgl_dropdown_clear_options_obj) },
    { MP_ROM_QSTR(MP_QSTR_add_option), MP_ROM_PTR(&rm690b0_lvgl_dropdown_add_option_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_dropdown_locals_dict, rm690b0_lvgl_dropdown_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_dropdown_type,
    MP_QSTR_Dropdown,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_dropdown_make_new,
    locals_dict, &rm690b0_lvgl_dropdown_locals_dict,
    parent, &rm690b0_lvgl_widget_type
    );
