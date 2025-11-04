// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Slider.h"
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

//| class Slider(Widget):
//|     """A slider widget for adjusting numeric values."""
//|
//|     def __init__(self, min_value: int = 0, max_value: int = 100) -> None:
//|         """Create a new Slider widget.
//|
//|         :param int min_value: The minimum value (default: 0)
//|         :param int max_value: The maximum value (default: 100)
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_slider_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_min_value, ARG_max_value };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_min_value, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_max_value, MP_ARG_INT, {.u_int = 100} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rm690b0_lvgl_slider_obj_t *self = mp_obj_malloc(rm690b0_lvgl_slider_obj_t, &rm690b0_lvgl_slider_type);
    
    // Initialize callback to None
    self->on_change_handler = mp_const_none;
    
    mp_int_t min_value = args[ARG_min_value].u_int;
    mp_int_t max_value = args[ARG_max_value].u_int;
    
    common_hal_rm690b0_lvgl_slider_construct(self, min_value, max_value);
    return MP_OBJ_FROM_PTR(self);
}

//|     value: int
//|     """The current value of the slider."""
//|
static mp_obj_t rm690b0_lvgl_slider_get_value(mp_obj_t self_in) {
    rm690b0_lvgl_slider_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_slider_get_value(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_slider_get_value_obj, rm690b0_lvgl_slider_get_value);

static mp_obj_t rm690b0_lvgl_slider_set_value(mp_obj_t self_in, mp_obj_t value_obj) {
    rm690b0_lvgl_slider_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_slider_set_value(self, mp_obj_get_int(value_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_slider_set_value_obj, rm690b0_lvgl_slider_set_value);

MP_PROPERTY_GETSET(rm690b0_lvgl_slider_value_obj,
    (mp_obj_t)&rm690b0_lvgl_slider_get_value_obj,
    (mp_obj_t)&rm690b0_lvgl_slider_set_value_obj);

//|     min_value: int
//|     """The minimum value of the slider."""
//|
static mp_obj_t rm690b0_lvgl_slider_get_min_value(mp_obj_t self_in) {
    rm690b0_lvgl_slider_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_slider_get_min_value(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_slider_get_min_value_obj, rm690b0_lvgl_slider_get_min_value);

static mp_obj_t rm690b0_lvgl_slider_set_min_value(mp_obj_t self_in, mp_obj_t min_value_obj) {
    rm690b0_lvgl_slider_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_slider_set_min_value(self, mp_obj_get_int(min_value_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_slider_set_min_value_obj, rm690b0_lvgl_slider_set_min_value);

MP_PROPERTY_GETSET(rm690b0_lvgl_slider_min_value_obj,
    (mp_obj_t)&rm690b0_lvgl_slider_get_min_value_obj,
    (mp_obj_t)&rm690b0_lvgl_slider_set_min_value_obj);

//|     max_value: int
//|     """The maximum value of the slider."""
//|
static mp_obj_t rm690b0_lvgl_slider_get_max_value(mp_obj_t self_in) {
    rm690b0_lvgl_slider_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_slider_get_max_value(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_slider_get_max_value_obj, rm690b0_lvgl_slider_get_max_value);

static mp_obj_t rm690b0_lvgl_slider_set_max_value(mp_obj_t self_in, mp_obj_t max_value_obj) {
    rm690b0_lvgl_slider_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_slider_set_max_value(self, mp_obj_get_int(max_value_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_slider_set_max_value_obj, rm690b0_lvgl_slider_set_max_value);

MP_PROPERTY_GETSET(rm690b0_lvgl_slider_max_value_obj,
    (mp_obj_t)&rm690b0_lvgl_slider_get_max_value_obj,
    (mp_obj_t)&rm690b0_lvgl_slider_set_max_value_obj);

//|     on_change: Optional[Callable[[Slider], None]]
//|     """The callback function to run when the slider value changes.
//|     The callback receives the slider instance as an argument."""
//|
static mp_obj_t rm690b0_lvgl_slider_get_on_change(mp_obj_t self_in) {
    rm690b0_lvgl_slider_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return self->on_change_handler;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_slider_get_on_change_obj, rm690b0_lvgl_slider_get_on_change);

static mp_obj_t rm690b0_lvgl_slider_set_on_change(mp_obj_t self_in, mp_obj_t callback) {
    rm690b0_lvgl_slider_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (callback != mp_const_none && !mp_obj_is_callable(callback)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Callback must be callable or None"));
    }
    self->on_change_handler = callback;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_slider_set_on_change_obj, rm690b0_lvgl_slider_set_on_change);

MP_PROPERTY_GETSET(rm690b0_lvgl_slider_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_slider_get_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_slider_set_on_change_obj);

//|     def set_range(self, min_value: int, max_value: int) -> None:
//|         """Set both minimum and maximum values at once.
//|
//|         :param int min_value: The minimum value
//|         :param int max_value: The maximum value
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_slider_set_range(mp_obj_t self_in, mp_obj_t min_value_obj, mp_obj_t max_value_obj) {
    rm690b0_lvgl_slider_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t min_value = mp_obj_get_int(min_value_obj);
    mp_int_t max_value = mp_obj_get_int(max_value_obj);
    common_hal_rm690b0_lvgl_slider_set_range(self, min_value, max_value);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(rm690b0_lvgl_slider_set_range_obj, rm690b0_lvgl_slider_set_range);

static const mp_rom_map_elem_t rm690b0_lvgl_slider_locals_dict_table[] = {
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
    
    // Slider-specific properties
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&rm690b0_lvgl_slider_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_min_value), MP_ROM_PTR(&rm690b0_lvgl_slider_min_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_max_value), MP_ROM_PTR(&rm690b0_lvgl_slider_max_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_on_change), MP_ROM_PTR(&rm690b0_lvgl_slider_on_change_obj) },
    
    // Slider-specific methods
    { MP_ROM_QSTR(MP_QSTR_set_range), MP_ROM_PTR(&rm690b0_lvgl_slider_set_range_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_slider_locals_dict, rm690b0_lvgl_slider_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_slider_type,
    MP_QSTR_Slider,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_slider_make_new,
    locals_dict, &rm690b0_lvgl_slider_locals_dict,
    parent, &rm690b0_lvgl_widget_type
);