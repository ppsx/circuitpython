// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Spinner.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"
#include "shared-bindings/util.h"

extern const mp_obj_property_t rm690b0_lvgl_widget_x_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_y_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_width_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_height_obj;

extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_color_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_opa_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_parent_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_delete_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_parent_obj;

//| class Spinner(Widget):
//|     """Spinner widget for indicating activity or loading state."""
//|
//|     def __init__(self, time: int = 1000, arc_length: int = 60) -> None:
//|         """Create a Spinner widget.
//|
//|         :param int time: The time for one revolution in milliseconds (default 1000)
//|         :param int arc_length: The length of the spinning arc in degrees (default 60)
//|         """
//|         ...
static mp_obj_t rm690b0_lvgl_spinner_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_time, ARG_arc_length };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_time, MP_ARG_INT, {.u_int = 1000} },
        { MP_QSTR_arc_length, MP_ARG_INT, {.u_int = 60} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rm690b0_lvgl_spinner_obj_t *self = mp_obj_malloc(rm690b0_lvgl_spinner_obj_t, &rm690b0_lvgl_spinner_type);
    common_hal_rm690b0_lvgl_spinner_construct(self, args[ARG_time].u_int, args[ARG_arc_length].u_int);

    return MP_OBJ_FROM_PTR(self);
}

static const mp_rom_map_elem_t rm690b0_lvgl_spinner_locals_dict_table[] = {
    // Inherited from Widget
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&rm690b0_lvgl_widget_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&rm690b0_lvgl_widget_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&rm690b0_lvgl_widget_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&rm690b0_lvgl_widget_height_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_color), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_color_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_color), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_color_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_opa), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_opa_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_parent), MP_ROM_PTR(&rm690b0_lvgl_widget_set_parent_obj) },
    { MP_ROM_QSTR(MP_QSTR_delete), MP_ROM_PTR(&rm690b0_lvgl_widget_delete_obj) },
    // Spinner specific methods could be added here if needed (e.g. changing speed/arc later)
    // but LVGL primarily configures them at creation.
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_spinner_locals_dict, rm690b0_lvgl_spinner_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_spinner_type,
    MP_QSTR_Spinner,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_spinner_make_new,
    locals_dict, &rm690b0_lvgl_spinner_locals_dict,
    parent, &rm690b0_lvgl_widget_type
    );