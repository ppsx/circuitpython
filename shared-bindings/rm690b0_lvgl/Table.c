// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Table.h"
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

//| class Table(Widget):
//|     """A table widget."""
//|
//|     def __init__(self) -> None:
//|         """Create a new Table widget."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_table_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    rm690b0_lvgl_table_obj_t *self = mp_obj_malloc(rm690b0_lvgl_table_obj_t, &rm690b0_lvgl_table_type);
    common_hal_rm690b0_lvgl_table_construct(self);
    return MP_OBJ_FROM_PTR(self);
}

//|     def set_cell_value(self, row: int, col: int, text: str) -> None:
//|         """Set the value of a cell.
//|
//|         :param int row: Row index
//|         :param int col: Column index
//|         :param str text: Text to display
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_table_set_cell_value(size_t n_args, const mp_obj_t *args) {
    rm690b0_lvgl_table_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint16_t row = (uint16_t)mp_obj_get_int(args[1]);
    uint16_t col = (uint16_t)mp_obj_get_int(args[2]);
    const char *text = mp_obj_str_get_str(args[3]);
    common_hal_rm690b0_lvgl_table_set_cell_value(self, row, col, text);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rm690b0_lvgl_table_set_cell_value_obj, 4, 4, rm690b0_lvgl_table_set_cell_value);

//|     def set_row_cnt(self, cnt: int) -> None:
//|         """Set the number of rows.
//|
//|         :param int cnt: Number of rows
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_table_set_row_cnt(mp_obj_t self_in, mp_obj_t cnt_obj) {
    rm690b0_lvgl_table_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint16_t cnt = (uint16_t)mp_obj_get_int(cnt_obj);
    common_hal_rm690b0_lvgl_table_set_row_cnt(self, cnt);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_table_set_row_cnt_obj, rm690b0_lvgl_table_set_row_cnt);

//|     def set_col_cnt(self, cnt: int) -> None:
//|         """Set the number of columns.
//|
//|         :param int cnt: Number of columns
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_table_set_col_cnt(mp_obj_t self_in, mp_obj_t cnt_obj) {
    rm690b0_lvgl_table_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint16_t cnt = (uint16_t)mp_obj_get_int(cnt_obj);
    common_hal_rm690b0_lvgl_table_set_col_cnt(self, cnt);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_table_set_col_cnt_obj, rm690b0_lvgl_table_set_col_cnt);

//|     def set_col_width(self, col: int, width: int) -> None:
//|         """Set the width of a column.
//|
//|         :param int col: Column index
//|         :param int width: Width in pixels
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_table_set_col_width(mp_obj_t self_in, mp_obj_t col_obj, mp_obj_t width_obj) {
    rm690b0_lvgl_table_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint16_t col = (uint16_t)mp_obj_get_int(col_obj);
    uint16_t width = (uint16_t)mp_obj_get_int(width_obj);
    common_hal_rm690b0_lvgl_table_set_col_width(self, col, width);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(rm690b0_lvgl_table_set_col_width_obj, rm690b0_lvgl_table_set_col_width);

static const mp_rom_map_elem_t rm690b0_lvgl_table_locals_dict_table[] = {
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

    // Table methods
    { MP_ROM_QSTR(MP_QSTR_set_cell_value), MP_ROM_PTR(&rm690b0_lvgl_table_set_cell_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_row_cnt), MP_ROM_PTR(&rm690b0_lvgl_table_set_row_cnt_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_col_cnt), MP_ROM_PTR(&rm690b0_lvgl_table_set_col_cnt_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_col_width), MP_ROM_PTR(&rm690b0_lvgl_table_set_col_width_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_table_locals_dict, rm690b0_lvgl_table_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_table_type,
    MP_QSTR_Table,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_table_make_new,
    locals_dict, &rm690b0_lvgl_table_locals_dict,
    parent, &rm690b0_lvgl_widget_type
);