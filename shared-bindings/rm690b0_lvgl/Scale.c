// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"

#include "shared-bindings/rm690b0_lvgl/Scale.h"
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

//| class Scale(Widget):
//|     """Gauge-like scale widget (backed by LVGL meter)."""
//|
//|     def __init__(self, min_value: int = 0, max_value: int = 100) -> None:
//|         """Create scale."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_scale_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_min_value, ARG_max_value };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_min_value, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_max_value, MP_ARG_INT, {.u_int = 100} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rm690b0_lvgl_scale_obj_t *self = mp_obj_malloc(rm690b0_lvgl_scale_obj_t, &rm690b0_lvgl_scale_type);
    self->base.callback = mp_const_none;
    common_hal_rm690b0_lvgl_scale_construct(self, args[ARG_min_value].u_int, args[ARG_max_value].u_int);
    return MP_OBJ_FROM_PTR(self);
}

//|     value: int
//|     """Current indicator value."""
//|
static mp_obj_t rm690b0_lvgl_scale_get_value(mp_obj_t self_in) {
    rm690b0_lvgl_scale_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_scale_get_value(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_scale_get_value_obj, rm690b0_lvgl_scale_get_value);

static mp_obj_t rm690b0_lvgl_scale_set_value(mp_obj_t self_in, mp_obj_t value_obj) {
    rm690b0_lvgl_scale_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_scale_set_value(self, mp_obj_get_int(value_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_scale_set_value_obj, rm690b0_lvgl_scale_set_value);

MP_PROPERTY_GETSET(rm690b0_lvgl_scale_value_obj,
    (mp_obj_t)&rm690b0_lvgl_scale_get_value_obj,
    (mp_obj_t)&rm690b0_lvgl_scale_set_value_obj);

//|     def set_range(self, min_value: int, max_value: int) -> None:
//|         """Set numeric range."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_scale_set_range(mp_obj_t self_in, mp_obj_t min_obj, mp_obj_t max_obj) {
    rm690b0_lvgl_scale_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_scale_set_range(self, mp_obj_get_int(min_obj), mp_obj_get_int(max_obj));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(rm690b0_lvgl_scale_set_range_obj, rm690b0_lvgl_scale_set_range);

//|     def set_angles(self, angle_range: int, rotation: int) -> None:
//|         """Configure sweep and rotation."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_scale_set_angles(mp_obj_t self_in, mp_obj_t range_obj, mp_obj_t rotation_obj) {
    rm690b0_lvgl_scale_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_scale_set_angles(self, mp_obj_get_int(range_obj), mp_obj_get_int(rotation_obj));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(rm690b0_lvgl_scale_set_angles_obj, rm690b0_lvgl_scale_set_angles);

//|     def set_ticks(self, count: int = 41, width: int = 2, length: int = 8, color: int = 0x888888) -> None:
//|         """Set minor tick appearance."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_scale_set_ticks(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_count, ARG_width, ARG_length, ARG_color };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_count, MP_ARG_INT, {.u_int = 41} },
        { MP_QSTR_width, MP_ARG_INT, {.u_int = 2} },
        { MP_QSTR_length, MP_ARG_INT, {.u_int = 8} },
        { MP_QSTR_color, MP_ARG_INT, {.u_int = 0x888888} },
    };
    rm690b0_lvgl_scale_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    common_hal_rm690b0_lvgl_scale_set_ticks(
        self,
        args[ARG_count].u_int,
        args[ARG_width].u_int,
        args[ARG_length].u_int,
        args[ARG_color].u_int);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(rm690b0_lvgl_scale_set_ticks_obj, 1, rm690b0_lvgl_scale_set_ticks);

//|     def set_major_ticks(
//|         self,
//|         every: int = 5,
//|         width: int = 4,
//|         length: int = 16,
//|         color: int = 0xFFFFFF,
//|         label_gap: int = 12,
//|     ) -> None:
//|         """Configure major tick marks and labels."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_scale_set_major_ticks(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_every, ARG_width, ARG_length, ARG_color, ARG_label_gap };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_every, MP_ARG_INT, {.u_int = 5} },
        { MP_QSTR_width, MP_ARG_INT, {.u_int = 4} },
        { MP_QSTR_length, MP_ARG_INT, {.u_int = 16} },
        { MP_QSTR_color, MP_ARG_INT, {.u_int = 0xffffff} },
        { MP_QSTR_label_gap, MP_ARG_INT, {.u_int = 12} },
    };
    rm690b0_lvgl_scale_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    common_hal_rm690b0_lvgl_scale_set_major_ticks(
        self,
        args[ARG_every].u_int,
        args[ARG_width].u_int,
        args[ARG_length].u_int,
        args[ARG_color].u_int,
        args[ARG_label_gap].u_int);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(rm690b0_lvgl_scale_set_major_ticks_obj, 1, rm690b0_lvgl_scale_set_major_ticks);

static mp_obj_t rm690b0_lvgl_scale_delete(mp_obj_t self_in) {
    rm690b0_lvgl_scale_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_scale_deinit(self);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_scale_delete_obj, rm690b0_lvgl_scale_delete);

static const mp_rom_map_elem_t rm690b0_lvgl_scale_locals_dict_table[] = {
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

    // Scale specific
    // Scale API
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&rm690b0_lvgl_scale_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_range), MP_ROM_PTR(&rm690b0_lvgl_scale_set_range_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_angles), MP_ROM_PTR(&rm690b0_lvgl_scale_set_angles_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_ticks), MP_ROM_PTR(&rm690b0_lvgl_scale_set_ticks_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_major_ticks), MP_ROM_PTR(&rm690b0_lvgl_scale_set_major_ticks_obj) },
    { MP_ROM_QSTR(MP_QSTR_delete), MP_ROM_PTR(&rm690b0_lvgl_scale_delete_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_scale_locals_dict, rm690b0_lvgl_scale_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_scale_type,
    MP_QSTR_Scale,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_scale_make_new,
    locals_dict, &rm690b0_lvgl_scale_locals_dict,
    parent, &rm690b0_lvgl_widget_type
    );
