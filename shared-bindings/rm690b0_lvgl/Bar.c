// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Bar.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"

// Import Widget property objects for inheritance
extern const mp_obj_property_t rm690b0_lvgl_widget_x_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_y_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_width_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_height_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_color_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_opa_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_parent_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_delete_obj;

//| class Bar(Widget):
//|     """A progress bar widget for displaying progress or levels."""
//|
//|     def __init__(self, min_value: int = 0, max_value: int = 100) -> None:
//|         """Create a new Bar widget.
//|
//|         :param int min_value: The minimum value (default: 0)
//|         :param int max_value: The maximum value (default: 100)
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_bar_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_min_value, ARG_max_value };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_min_value, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_max_value, MP_ARG_INT, {.u_int = 100} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rm690b0_lvgl_bar_obj_t *self = mp_obj_malloc(rm690b0_lvgl_bar_obj_t, &rm690b0_lvgl_bar_type);

    mp_int_t min_value = args[ARG_min_value].u_int;
    mp_int_t max_value = args[ARG_max_value].u_int;

    common_hal_rm690b0_lvgl_bar_construct(self, min_value, max_value);
    return MP_OBJ_FROM_PTR(self);
}

//|     value: int
//|     """The current value of the bar."""
//|
static mp_obj_t rm690b0_lvgl_bar_get_value(mp_obj_t self_in) {
    rm690b0_lvgl_bar_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(common_hal_rm690b0_lvgl_bar_get_value(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_bar_get_value_obj, rm690b0_lvgl_bar_get_value);

static mp_obj_t rm690b0_lvgl_bar_set_value(mp_obj_t self_in, mp_obj_t value_obj) {
    rm690b0_lvgl_bar_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t value = mp_obj_get_int(value_obj);
    common_hal_rm690b0_lvgl_bar_set_value(self, value);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_bar_set_value_obj, rm690b0_lvgl_bar_set_value);

MP_PROPERTY_GETSET(rm690b0_lvgl_bar_value_obj,
    (mp_obj_t)&rm690b0_lvgl_bar_get_value_obj,
    (mp_obj_t)&rm690b0_lvgl_bar_set_value_obj);

//|     min_value: int
//|     """The minimum value of the bar."""
//|
static mp_obj_t rm690b0_lvgl_bar_get_min_value(mp_obj_t self_in) {
    rm690b0_lvgl_bar_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(common_hal_rm690b0_lvgl_bar_get_min_value(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_bar_get_min_value_obj, rm690b0_lvgl_bar_get_min_value);

static mp_obj_t rm690b0_lvgl_bar_set_min_value(mp_obj_t self_in, mp_obj_t min_value_obj) {
    rm690b0_lvgl_bar_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t min_value = mp_obj_get_int(min_value_obj);
    common_hal_rm690b0_lvgl_bar_set_min_value(self, min_value);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_bar_set_min_value_obj, rm690b0_lvgl_bar_set_min_value);

MP_PROPERTY_GETSET(rm690b0_lvgl_bar_min_value_obj,
    (mp_obj_t)&rm690b0_lvgl_bar_get_min_value_obj,
    (mp_obj_t)&rm690b0_lvgl_bar_set_min_value_obj);

//|     max_value: int
//|     """The maximum value of the bar."""
//|
static mp_obj_t rm690b0_lvgl_bar_get_max_value(mp_obj_t self_in) {
    rm690b0_lvgl_bar_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(common_hal_rm690b0_lvgl_bar_get_max_value(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_bar_get_max_value_obj, rm690b0_lvgl_bar_get_max_value);

static mp_obj_t rm690b0_lvgl_bar_set_max_value(mp_obj_t self_in, mp_obj_t max_value_obj) {
    rm690b0_lvgl_bar_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t max_value = mp_obj_get_int(max_value_obj);
    common_hal_rm690b0_lvgl_bar_set_max_value(self, max_value);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_bar_set_max_value_obj, rm690b0_lvgl_bar_set_max_value);

MP_PROPERTY_GETSET(rm690b0_lvgl_bar_max_value_obj,
    (mp_obj_t)&rm690b0_lvgl_bar_get_max_value_obj,
    (mp_obj_t)&rm690b0_lvgl_bar_set_max_value_obj);

//|     def set_range(self, min_value: int, max_value: int) -> None:
//|         """Set both minimum and maximum values at once.
//|
//|         :param int min_value: The new minimum value
//|         :param int max_value: The new maximum value
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_bar_set_range(mp_obj_t self_in, mp_obj_t min_value_obj, mp_obj_t max_value_obj) {
    rm690b0_lvgl_bar_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t min_value = mp_obj_get_int(min_value_obj);
    mp_int_t max_value = mp_obj_get_int(max_value_obj);
    common_hal_rm690b0_lvgl_bar_set_range(self, min_value, max_value);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(rm690b0_lvgl_bar_set_range_obj, rm690b0_lvgl_bar_set_range);

static const mp_rom_map_elem_t rm690b0_lvgl_bar_locals_dict_table[] = {
    // Inherited Widget properties
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&rm690b0_lvgl_widget_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&rm690b0_lvgl_widget_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&rm690b0_lvgl_widget_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&rm690b0_lvgl_widget_height_obj) },

    // Inherited Widget methods
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_color), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_color_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_opa), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_opa_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_parent), MP_ROM_PTR(&rm690b0_lvgl_widget_set_parent_obj) },
    { MP_ROM_QSTR(MP_QSTR_delete), MP_ROM_PTR(&rm690b0_lvgl_widget_delete_obj) },

    // Bar-specific properties
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&rm690b0_lvgl_bar_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_min_value), MP_ROM_PTR(&rm690b0_lvgl_bar_min_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_max_value), MP_ROM_PTR(&rm690b0_lvgl_bar_max_value_obj) },

    // Bar-specific methods
    { MP_ROM_QSTR(MP_QSTR_set_range), MP_ROM_PTR(&rm690b0_lvgl_bar_set_range_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_bar_locals_dict, rm690b0_lvgl_bar_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_bar_type,
    MP_QSTR_Bar,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_bar_make_new,
    locals_dict, &rm690b0_lvgl_bar_locals_dict,
    parent, &rm690b0_lvgl_widget_type
    );
