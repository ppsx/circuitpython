// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"

#include "shared-bindings/rm690b0_lvgl/Line.h"
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

static void line_parse_points(mp_obj_t seq_obj, mp_int_t **out_coords, size_t *out_coord_count) {
    size_t len;
    mp_obj_t *items;
    mp_obj_get_array(seq_obj, &len, &items);
    if (len < 2) {
        mp_raise_ValueError(MP_ERROR_TEXT("At least two points required"));
    }
    mp_int_t *coords = m_new(mp_int_t, len * 2);
    size_t idx = 0;
    for (size_t i = 0; i < len; i++) {
        size_t tuple_len;
        mp_obj_t *tuple_items;
        mp_obj_get_array(items[i], &tuple_len, &tuple_items);
        if (tuple_len != 2) {
            m_del(mp_int_t, coords, len * 2);
            mp_raise_ValueError(MP_ERROR_TEXT("Points must be (x, y)"));
        }
        coords[idx++] = mp_obj_get_int(tuple_items[0]);
        coords[idx++] = mp_obj_get_int(tuple_items[1]);
    }
    *out_coords = coords;
    *out_coord_count = len * 2;
}

//| class Line(Widget):
//|     """Simple polyline widget."""
//|
//|     def __init__(self) -> None:
//|         """Create a new line."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_line_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    (void)all_args;
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    rm690b0_lvgl_line_obj_t *self = mp_obj_malloc(rm690b0_lvgl_line_obj_t, &rm690b0_lvgl_line_type);
    self->base.callback = mp_const_none;
    self->points = NULL;
    self->point_count = 0;
    common_hal_rm690b0_lvgl_line_construct(self);
    return MP_OBJ_FROM_PTR(self);
}

//|     def set_points(self, points: Sequence[tuple[int, int]]) -> None:
//|         """Set line vertices."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_line_set_points(mp_obj_t self_in, mp_obj_t seq_obj) {
    rm690b0_lvgl_line_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t *coords;
    size_t coord_count;
    line_parse_points(seq_obj, &coords, &coord_count);
    common_hal_rm690b0_lvgl_line_set_points(self, coords, coord_count);
    m_del(mp_int_t, coords, coord_count);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_line_set_points_obj, rm690b0_lvgl_line_set_points);

//|     y_invert: bool
//|     """Flip vertical axis."""
//|
static mp_obj_t rm690b0_lvgl_line_get_y_invert(mp_obj_t self_in) {
    rm690b0_lvgl_line_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(common_hal_rm690b0_lvgl_line_get_y_invert(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_line_get_y_invert_obj, rm690b0_lvgl_line_get_y_invert);

static mp_obj_t rm690b0_lvgl_line_set_y_invert(mp_obj_t self_in, mp_obj_t val_obj) {
    rm690b0_lvgl_line_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_line_set_y_invert(self, mp_obj_is_true(val_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_line_set_y_invert_obj, rm690b0_lvgl_line_set_y_invert);

MP_PROPERTY_GETSET(rm690b0_lvgl_line_y_invert_obj,
    (mp_obj_t)&rm690b0_lvgl_line_get_y_invert_obj,
    (mp_obj_t)&rm690b0_lvgl_line_set_y_invert_obj);

//|     line_width: int
//|     """Stroke width."""
//|
static mp_obj_t rm690b0_lvgl_line_get_line_width(mp_obj_t self_in) {
    rm690b0_lvgl_line_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_line_get_line_width(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_line_get_line_width_obj, rm690b0_lvgl_line_get_line_width);

static mp_obj_t rm690b0_lvgl_line_set_line_width(mp_obj_t self_in, mp_obj_t width_obj) {
    rm690b0_lvgl_line_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_line_set_line_width(self, mp_obj_get_int(width_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_line_set_line_width_obj, rm690b0_lvgl_line_set_line_width);

MP_PROPERTY_GETSET(rm690b0_lvgl_line_line_width_obj,
    (mp_obj_t)&rm690b0_lvgl_line_get_line_width_obj,
    (mp_obj_t)&rm690b0_lvgl_line_set_line_width_obj);

//|     line_color: int
//|     """Stroke color."""
//|
static mp_obj_t rm690b0_lvgl_line_get_line_color(mp_obj_t self_in) {
    rm690b0_lvgl_line_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_line_get_line_color(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_line_get_line_color_obj, rm690b0_lvgl_line_get_line_color);

static mp_obj_t rm690b0_lvgl_line_set_line_color(mp_obj_t self_in, mp_obj_t color_obj) {
    rm690b0_lvgl_line_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_line_set_line_color(self, mp_obj_get_int(color_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_line_set_line_color_obj, rm690b0_lvgl_line_set_line_color);

MP_PROPERTY_GETSET(rm690b0_lvgl_line_line_color_obj,
    (mp_obj_t)&rm690b0_lvgl_line_get_line_color_obj,
    (mp_obj_t)&rm690b0_lvgl_line_set_line_color_obj);

//|     rounded: bool
//|     """Round end caps."""
//|
static mp_obj_t rm690b0_lvgl_line_get_rounded(mp_obj_t self_in) {
    rm690b0_lvgl_line_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(common_hal_rm690b0_lvgl_line_get_rounded(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_line_get_rounded_obj, rm690b0_lvgl_line_get_rounded);

static mp_obj_t rm690b0_lvgl_line_set_rounded(mp_obj_t self_in, mp_obj_t value) {
    rm690b0_lvgl_line_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_line_set_rounded(self, mp_obj_is_true(value));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_line_set_rounded_obj, rm690b0_lvgl_line_set_rounded);

MP_PROPERTY_GETSET(rm690b0_lvgl_line_rounded_obj,
    (mp_obj_t)&rm690b0_lvgl_line_get_rounded_obj,
    (mp_obj_t)&rm690b0_lvgl_line_set_rounded_obj);

static mp_obj_t rm690b0_lvgl_line_delete(mp_obj_t self_in) {
    rm690b0_lvgl_line_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_line_deinit(self);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_line_delete_obj, rm690b0_lvgl_line_delete);

static const mp_rom_map_elem_t rm690b0_lvgl_line_locals_dict_table[] = {
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

    // Line API
    // Line-specific
    { MP_ROM_QSTR(MP_QSTR_set_points), MP_ROM_PTR(&rm690b0_lvgl_line_set_points_obj) },
    { MP_ROM_QSTR(MP_QSTR_y_invert), MP_ROM_PTR(&rm690b0_lvgl_line_y_invert_obj) },
    { MP_ROM_QSTR(MP_QSTR_line_width), MP_ROM_PTR(&rm690b0_lvgl_line_line_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_line_color), MP_ROM_PTR(&rm690b0_lvgl_line_line_color_obj) },
    { MP_ROM_QSTR(MP_QSTR_rounded), MP_ROM_PTR(&rm690b0_lvgl_line_rounded_obj) },
    { MP_ROM_QSTR(MP_QSTR_delete), MP_ROM_PTR(&rm690b0_lvgl_line_delete_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_line_locals_dict, rm690b0_lvgl_line_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_line_type,
    MP_QSTR_Line,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_line_make_new,
    locals_dict, &rm690b0_lvgl_line_locals_dict,
    parent, &rm690b0_lvgl_widget_type
    );
