// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Container.h"
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

//| class Container(Widget):
//|     """Container widget with layout capabilities."""
//|
//|     def __init__(self) -> None:
//|         """Create a Container widget."""
//|         ...
static mp_obj_t rm690b0_lvgl_container_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    rm690b0_lvgl_container_obj_t *self = mp_obj_malloc(rm690b0_lvgl_container_obj_t, &rm690b0_lvgl_container_type);
    common_hal_rm690b0_lvgl_container_construct(self);

    return MP_OBJ_FROM_PTR(self);
}

//|     def set_flex_flow(self, flow: int) -> None:
//|         """Set the flex layout flow direction.
//|
//|         :param int flow: Layout direction (ROW, COLUMN, ROW_WRAP, COLUMN_WRAP)
//|         """
//|         ...
static mp_obj_t rm690b0_lvgl_container_set_flex_flow(mp_obj_t self_in, mp_obj_t flow_obj) {
    rm690b0_lvgl_container_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_container_set_flex_flow(self, mp_obj_get_int(flow_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_container_set_flex_flow_obj, rm690b0_lvgl_container_set_flex_flow);

//|     def set_flex_align(self, main_place: int, cross_place: int, track_cross_place: int) -> None:
//|         """Set the alignment of children within the container.
//|
//|         :param int main_place: Alignment on main axis
//|         :param int cross_place: Alignment on cross axis
//|         :param int track_cross_place: Alignment of tracks (if wrapping)
//|         """
//|         ...
static mp_obj_t rm690b0_lvgl_container_set_flex_align(size_t n_args, const mp_obj_t *args) {
    rm690b0_lvgl_container_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    common_hal_rm690b0_lvgl_container_set_flex_align(self, mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rm690b0_lvgl_container_set_flex_align_obj, 4, 4, rm690b0_lvgl_container_set_flex_align);

//|     def set_padding(self, padding: int) -> None:
//|         """Set padding on all sides in pixels.
//|
//|         :param int padding: Padding in pixels
//|         """
//|         ...
static mp_obj_t rm690b0_lvgl_container_set_padding(mp_obj_t self_in, mp_obj_t padding_obj) {
    rm690b0_lvgl_container_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_container_set_padding(self, mp_obj_get_int(padding_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_container_set_padding_obj, rm690b0_lvgl_container_set_padding);

static const mp_rom_map_elem_t rm690b0_lvgl_container_locals_dict_table[] = {
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

    // Container-specific methods
    // Container specific
    { MP_ROM_QSTR(MP_QSTR_set_flex_flow), MP_ROM_PTR(&rm690b0_lvgl_container_set_flex_flow_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_flex_align), MP_ROM_PTR(&rm690b0_lvgl_container_set_flex_align_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_padding), MP_ROM_PTR(&rm690b0_lvgl_container_set_padding_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_container_locals_dict, rm690b0_lvgl_container_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_container_type,
    MP_QSTR_Container,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_container_make_new,
    locals_dict, &rm690b0_lvgl_container_locals_dict,
    parent, &rm690b0_lvgl_widget_type
    );
