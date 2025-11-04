// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"

#include "shared-bindings/rm690b0_lvgl/Canvas.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

// Import Widget property objects for inheritance
extern const mp_obj_property_t rm690b0_lvgl_widget_x_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_y_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_width_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_height_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_color_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_opa_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_parent_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_delete_obj;

static void canvas_parse_points(mp_obj_t seq_obj, mp_int_t **out_coords, size_t *out_pairs) {
    size_t len;
    mp_obj_t *items;
    mp_obj_get_array(seq_obj, &len, &items);
    if (len < 2) {
        mp_raise_ValueError(MP_ERROR_TEXT("require at least 2 points"));
    }

    mp_int_t *buffer = m_new(mp_int_t, len * 2);
    size_t pair_index = 0;
    for (size_t i = 0; i < len; i++) {
        size_t tuple_len;
        mp_obj_t *tuple_items;
        mp_obj_get_array(items[i], &tuple_len, &tuple_items);
        if (tuple_len != 2) {
            m_del(mp_int_t, buffer, len * 2);
            mp_raise_ValueError(MP_ERROR_TEXT("points must be (x, y) pairs"));
        }
        buffer[pair_index++] = mp_obj_get_int(tuple_items[0]);
        buffer[pair_index++] = mp_obj_get_int(tuple_items[1]);
    }
    *out_coords = buffer;
    *out_pairs = len;
}

//| class Canvas(Widget):
//|     """Off-screen pixel buffer."""
//|
//|     def __init__(self, width: int, height: int, color_format: int = IMG_CF_TRUE_COLOR) -> None:
//|         """Create a new canvas."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_canvas_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_width, ARG_height, ARG_color_format };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_width, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_height, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_color_format, MP_ARG_INT, {.u_int = LV_IMG_CF_TRUE_COLOR } },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rm690b0_lvgl_canvas_obj_t *self = mp_obj_malloc(rm690b0_lvgl_canvas_obj_t, &rm690b0_lvgl_canvas_type);
    self->base.callback = mp_const_none;
    self->buffer = NULL;
    self->buffer_size = 0;

    common_hal_rm690b0_lvgl_canvas_construct(
        self, args[ARG_width].u_int, args[ARG_height].u_int, args[ARG_color_format].u_int);
    return MP_OBJ_FROM_PTR(self);
}

//|     def fill_bg(self, color: int, opacity: int = 255) -> None:
//|         """Fill entire canvas."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_canvas_fill_bg(size_t n_args, const mp_obj_t *args) {
    rm690b0_lvgl_canvas_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t opacity = 255;
    if (n_args > 2) {
        opacity = mp_obj_get_int(args[2]);
    }
    common_hal_rm690b0_lvgl_canvas_fill_bg(self, mp_obj_get_int(args[1]), opacity);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rm690b0_lvgl_canvas_fill_bg_obj, 2, 3, rm690b0_lvgl_canvas_fill_bg);

//|     def set_px(self, x: int, y: int, color: int, opacity: int = 255) -> None:
//|         """Draw a single pixel."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_canvas_set_px(size_t n_args, const mp_obj_t *args) {
    rm690b0_lvgl_canvas_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t opacity = 255;
    if (n_args > 4) {
        opacity = mp_obj_get_int(args[4]);
    }
    common_hal_rm690b0_lvgl_canvas_set_px(
        self,
        mp_obj_get_int(args[1]),
        mp_obj_get_int(args[2]),
        mp_obj_get_int(args[3]),
        opacity);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rm690b0_lvgl_canvas_set_px_obj, 4, 5, rm690b0_lvgl_canvas_set_px);

//|     def draw_line(self, points: Sequence[tuple[int, int]], color: int, width: int = 2) -> None:
//|         """Draw a polyline."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_canvas_draw_line(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_points, ARG_color, ARG_width };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_points, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_color, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_width, MP_ARG_INT, {.u_int = 2} },
    };
    rm690b0_lvgl_canvas_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_int_t *coords;
    size_t pair_count;
    canvas_parse_points(args[ARG_points].u_obj, &coords, &pair_count);
    common_hal_rm690b0_lvgl_canvas_draw_line(self, coords, pair_count, args[ARG_color].u_int, args[ARG_width].u_int);
    m_del(mp_int_t, coords, pair_count * 2);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(rm690b0_lvgl_canvas_draw_line_obj, 2, rm690b0_lvgl_canvas_draw_line);

static mp_obj_t rm690b0_lvgl_canvas_delete(mp_obj_t self_in) {
    rm690b0_lvgl_canvas_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_canvas_deinit(self);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_canvas_delete_obj, rm690b0_lvgl_canvas_delete);

static const mp_rom_map_elem_t rm690b0_lvgl_canvas_locals_dict_table[] = {
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

    // Canvas methods
    // Canvas API
    { MP_ROM_QSTR(MP_QSTR_fill_bg), MP_ROM_PTR(&rm690b0_lvgl_canvas_fill_bg_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_px), MP_ROM_PTR(&rm690b0_lvgl_canvas_set_px_obj) },
    { MP_ROM_QSTR(MP_QSTR_draw_line), MP_ROM_PTR(&rm690b0_lvgl_canvas_draw_line_obj) },
    { MP_ROM_QSTR(MP_QSTR_delete), MP_ROM_PTR(&rm690b0_lvgl_canvas_delete_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_canvas_locals_dict, rm690b0_lvgl_canvas_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_canvas_type,
    MP_QSTR_Canvas,
    MP_TYPE_FLAG_NONE,
    make_new, rm690b0_lvgl_canvas_make_new,
    locals_dict, &rm690b0_lvgl_canvas_locals_dict,
    parent, &rm690b0_lvgl_widget_type
);
