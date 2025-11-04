// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Spinbox.h"
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

//| class Spinbox(Widget):
//|     """A numeric spinbox widget."""
//|
//|     def __init__(self) -> None:
//|         """Create a new Spinbox widget."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_spinbox_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    rm690b0_lvgl_spinbox_obj_t *self = mp_obj_malloc(rm690b0_lvgl_spinbox_obj_t, &rm690b0_lvgl_spinbox_type);
    self->on_change_handler = mp_const_none;
    common_hal_rm690b0_lvgl_spinbox_construct(self);
    return MP_OBJ_FROM_PTR(self);
}

//|     value: int
//|     """The current value of the spinbox."""
//|
static mp_obj_t rm690b0_lvgl_spinbox_get_value(mp_obj_t self_in) {
    rm690b0_lvgl_spinbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_spinbox_get_value(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_spinbox_get_value_obj, rm690b0_lvgl_spinbox_get_value);

static mp_obj_t rm690b0_lvgl_spinbox_set_value(mp_obj_t self_in, mp_obj_t value_obj) {
    rm690b0_lvgl_spinbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t value = mp_obj_get_int(value_obj);
    common_hal_rm690b0_lvgl_spinbox_set_value(self, value);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_spinbox_set_value_obj, rm690b0_lvgl_spinbox_set_value);

MP_PROPERTY_GETSET(rm690b0_lvgl_spinbox_value_obj,
    (mp_obj_t)&rm690b0_lvgl_spinbox_get_value_obj,
    (mp_obj_t)&rm690b0_lvgl_spinbox_set_value_obj);

//|     def set_range(self, min_value: int, max_value: int) -> None:
//|         """Set the range of the spinbox.
//|
//|         :param int min_value: The minimum value
//|         :param int max_value: The maximum value
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_spinbox_set_range(mp_obj_t self_in, mp_obj_t min_obj, mp_obj_t max_obj) {
    rm690b0_lvgl_spinbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t min_val = mp_obj_get_int(min_obj);
    mp_int_t max_val = mp_obj_get_int(max_obj);
    common_hal_rm690b0_lvgl_spinbox_set_range(self, min_val, max_val);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(rm690b0_lvgl_spinbox_set_range_obj, rm690b0_lvgl_spinbox_set_range);

//|     def increment(self) -> None:
//|         """Increment the spinbox value."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_spinbox_increment(mp_obj_t self_in) {
    rm690b0_lvgl_spinbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_spinbox_increment(self);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_spinbox_increment_obj, rm690b0_lvgl_spinbox_increment);

//|     def decrement(self) -> None:
//|         """Decrement the spinbox value."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_spinbox_decrement(mp_obj_t self_in) {
    rm690b0_lvgl_spinbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_spinbox_decrement(self);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_spinbox_decrement_obj, rm690b0_lvgl_spinbox_decrement);

//|     def set_digit_format(self, digit_count: int, separator_position: int) -> None:
//|         """Set the digit format.
//|
//|         :param int digit_count: Total number of digits
//|         :param int separator_position: Position of the decimal separator from the right
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_spinbox_set_digit_format(mp_obj_t self_in, mp_obj_t count_obj, mp_obj_t sep_obj) {
    rm690b0_lvgl_spinbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t count = mp_obj_get_int(count_obj);
    mp_int_t sep = mp_obj_get_int(sep_obj);
    common_hal_rm690b0_lvgl_spinbox_set_digit_format(self, (uint8_t)count, (uint8_t)sep);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(rm690b0_lvgl_spinbox_set_digit_format_obj, rm690b0_lvgl_spinbox_set_digit_format);

//|     def set_step(self, step: int) -> None:
//|         """Set the increment/decrement step.
//|
//|         :param int step: The step value
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_spinbox_set_step(mp_obj_t self_in, mp_obj_t step_obj) {
    rm690b0_lvgl_spinbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t step = mp_obj_get_int(step_obj);
    common_hal_rm690b0_lvgl_spinbox_set_step(self, (uint32_t)step);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_spinbox_set_step_obj, rm690b0_lvgl_spinbox_set_step);

//|     on_change: Optional[Callable[[Spinbox], None]]
//|     """The callback function to run when the value changes."""
//|
static mp_obj_t rm690b0_lvgl_spinbox_get_on_change(mp_obj_t self_in) {
    rm690b0_lvgl_spinbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return self->on_change_handler;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_spinbox_get_on_change_obj, rm690b0_lvgl_spinbox_get_on_change);

static mp_obj_t rm690b0_lvgl_spinbox_set_on_change(mp_obj_t self_in, mp_obj_t callback) {
    rm690b0_lvgl_spinbox_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (callback != mp_const_none && !mp_obj_is_callable(callback)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Callback must be callable or None"));
    }
    self->on_change_handler = callback;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_spinbox_set_on_change_obj, rm690b0_lvgl_spinbox_set_on_change);

MP_PROPERTY_GETSET(rm690b0_lvgl_spinbox_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_spinbox_get_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_spinbox_set_on_change_obj);

static const mp_rom_map_elem_t rm690b0_lvgl_spinbox_locals_dict_table[] = {
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
    
    // Spinbox-specific properties
    // Spinbox specific
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&rm690b0_lvgl_spinbox_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_range), MP_ROM_PTR(&rm690b0_lvgl_spinbox_set_range_obj) },
    { MP_ROM_QSTR(MP_QSTR_increment), MP_ROM_PTR(&rm690b0_lvgl_spinbox_increment_obj) },
    { MP_ROM_QSTR(MP_QSTR_decrement), MP_ROM_PTR(&rm690b0_lvgl_spinbox_decrement_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_digit_format), MP_ROM_PTR(&rm690b0_lvgl_spinbox_set_digit_format_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_step), MP_ROM_PTR(&rm690b0_lvgl_spinbox_set_step_obj) },
    { MP_ROM_QSTR(MP_QSTR_on_change), MP_ROM_PTR(&rm690b0_lvgl_spinbox_on_change_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_spinbox_locals_dict, rm690b0_lvgl_spinbox_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_spinbox_type,
    MP_QSTR_Spinbox,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_spinbox_make_new,
    locals_dict, &rm690b0_lvgl_spinbox_locals_dict,
    parent, &rm690b0_lvgl_widget_type
);