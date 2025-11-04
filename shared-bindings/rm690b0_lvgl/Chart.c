// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"

#include "shared-bindings/rm690b0_lvgl/Chart.h"
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

static void chart_series_parse_values(mp_obj_t values_obj, mp_int_t **out_values, size_t *out_len) {
    size_t len;
    mp_obj_t *items;
    mp_obj_get_array(values_obj, &len, &items);

    if (len == 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("values must not be empty"));
    }

    mp_int_t *buffer = m_new(mp_int_t, len);
    for (size_t i = 0; i < len; i++) {
        buffer[i] = mp_obj_get_int(items[i]);
    }
    *out_values = buffer;
    *out_len = len;
}

//| class Chart(Widget):
//|     """LVGL chart widget."""
//|
//|     def __init__(self, chart_type: int = CHART_TYPE_LINE) -> None:
//|         """Create a chart."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_chart_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_chart_type };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_chart_type, MP_ARG_INT, {.u_int = LV_CHART_TYPE_LINE } },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rm690b0_lvgl_chart_obj_t *self = mp_obj_malloc(rm690b0_lvgl_chart_obj_t, &rm690b0_lvgl_chart_type);
    self->base.callback = mp_const_none;
    common_hal_rm690b0_lvgl_chart_construct(self, args[ARG_chart_type].u_int);
    return MP_OBJ_FROM_PTR(self);
}

//|     type: int
//|     """Chart rendering type."""
//|
static mp_obj_t rm690b0_lvgl_chart_get_type(mp_obj_t self_in) {
    rm690b0_lvgl_chart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_chart_get_type(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_chart_get_type_obj, rm690b0_lvgl_chart_get_type);

static mp_obj_t rm690b0_lvgl_chart_set_type(mp_obj_t self_in, mp_obj_t type_obj) {
    rm690b0_lvgl_chart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_chart_set_type(self, mp_obj_get_int(type_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_chart_set_type_obj, rm690b0_lvgl_chart_set_type);

MP_PROPERTY_GETSET(rm690b0_lvgl_chart_type_obj,
    (mp_obj_t)&rm690b0_lvgl_chart_get_type_obj,
    (mp_obj_t)&rm690b0_lvgl_chart_set_type_obj);

//|     point_count: int
//|     """Number of data points kept per series."""
//|
static mp_obj_t rm690b0_lvgl_chart_get_point_count(mp_obj_t self_in) {
    rm690b0_lvgl_chart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_chart_get_point_count(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_chart_get_point_count_obj, rm690b0_lvgl_chart_get_point_count);

static mp_obj_t rm690b0_lvgl_chart_set_point_count(mp_obj_t self_in, mp_obj_t value_obj) {
    rm690b0_lvgl_chart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_chart_set_point_count(self, mp_obj_get_int(value_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_chart_set_point_count_obj, rm690b0_lvgl_chart_set_point_count);

MP_PROPERTY_GETSET(rm690b0_lvgl_chart_point_count_obj,
    (mp_obj_t)&rm690b0_lvgl_chart_get_point_count_obj,
    (mp_obj_t)&rm690b0_lvgl_chart_set_point_count_obj);

//|     def set_range(self, axis: int, min_value: int, max_value: int) -> None:
//|         """Configure axis range."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_chart_set_range(size_t n_args, const mp_obj_t *args) {
    (void)n_args;
    rm690b0_lvgl_chart_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    common_hal_rm690b0_lvgl_chart_set_range(
        self,
        mp_obj_get_int(args[1]),
        mp_obj_get_int(args[2]),
        mp_obj_get_int(args[3]));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rm690b0_lvgl_chart_set_range_obj, 4, 4, rm690b0_lvgl_chart_set_range);

//|     def add_series(self, color: int, axis: int = CHART_AXIS_PRIMARY_Y) -> ChartSeries:
//|         """Add a new data series."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_chart_add_series(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_color, ARG_axis };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_color, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_axis, MP_ARG_INT, {.u_int = LV_CHART_AXIS_PRIMARY_Y} },
    };
    rm690b0_lvgl_chart_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    return common_hal_rm690b0_lvgl_chart_add_series(self, args[ARG_color].u_int, args[ARG_axis].u_int);
}
static MP_DEFINE_CONST_FUN_OBJ_KW(rm690b0_lvgl_chart_add_series_obj, 1, rm690b0_lvgl_chart_add_series);

//|     def refresh(self) -> None:
//|         """Force redraw."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_chart_refresh(mp_obj_t self_in) {
    rm690b0_lvgl_chart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_chart_refresh(self);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_chart_refresh_obj, rm690b0_lvgl_chart_refresh);

// ----------------
// ChartSeries type
// ----------------

//| class ChartSeries:
//|     """Handle returned from Chart.add_series()."""
//|
//|     def set_points(self, values: Sequence[int]) -> None:
//|         """Replace series data."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_chart_series_set_points(mp_obj_t self_in, mp_obj_t values_obj) {
    rm690b0_lvgl_chart_series_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t *values;
    size_t len;
    chart_series_parse_values(values_obj, &values, &len);
    common_hal_rm690b0_lvgl_chart_series_set_points(self, len, values);
    m_del(mp_int_t, values, len);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_chart_series_set_points_obj, rm690b0_lvgl_chart_series_set_points);

//|     def set_point(self, index: int, value: int) -> None:
//|         """Assign a single point."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_chart_series_set_point(mp_obj_t self_in, mp_obj_t index_obj, mp_obj_t value_obj) {
    rm690b0_lvgl_chart_series_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_chart_series_set_point(self, mp_obj_get_int(index_obj), mp_obj_get_int(value_obj));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(rm690b0_lvgl_chart_series_set_point_obj, rm690b0_lvgl_chart_series_set_point);

//|     def append(self, value: int) -> None:
//|         """Append value with LVGL's rolling buffer behavior."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_chart_series_append(mp_obj_t self_in, mp_obj_t value_obj) {
    rm690b0_lvgl_chart_series_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_chart_series_append(self, mp_obj_get_int(value_obj));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_chart_series_append_obj, rm690b0_lvgl_chart_series_append);

//|     color: int
//|     """Series color."""
//|
static mp_obj_t rm690b0_lvgl_chart_series_get_color(mp_obj_t self_in) {
    rm690b0_lvgl_chart_series_obj_t *self = MP_OBJ_TO_PTR(self_in);
    lv_obj_t *chart = (lv_obj_t *)self->chart->base.native_obj;
    lv_chart_series_t *series = (lv_chart_series_t *)self->series;
    if (chart == NULL || series == NULL) {
        return MP_OBJ_NEW_SMALL_INT(0);
    }
    lv_color_t color = series->color;
    return MP_OBJ_NEW_SMALL_INT(lv_color_to32(color));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_chart_series_get_color_obj, rm690b0_lvgl_chart_series_get_color);

static mp_obj_t rm690b0_lvgl_chart_series_set_color(mp_obj_t self_in, mp_obj_t color_obj) {
    rm690b0_lvgl_chart_series_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_chart_series_set_color(self, mp_obj_get_int(color_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_chart_series_set_color_obj, rm690b0_lvgl_chart_series_set_color);

MP_PROPERTY_GETSET(rm690b0_lvgl_chart_series_color_obj,
    (mp_obj_t)&rm690b0_lvgl_chart_series_get_color_obj,
    (mp_obj_t)&rm690b0_lvgl_chart_series_set_color_obj);

static const mp_rom_map_elem_t rm690b0_lvgl_chart_series_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_set_points), MP_ROM_PTR(&rm690b0_lvgl_chart_series_set_points_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_point), MP_ROM_PTR(&rm690b0_lvgl_chart_series_set_point_obj) },
    { MP_ROM_QSTR(MP_QSTR_append), MP_ROM_PTR(&rm690b0_lvgl_chart_series_append_obj) },
    { MP_ROM_QSTR(MP_QSTR_color), MP_ROM_PTR(&rm690b0_lvgl_chart_series_color_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_chart_series_locals_dict, rm690b0_lvgl_chart_series_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_chart_series_type,
    MP_QSTR_ChartSeries,
    MP_TYPE_FLAG_NONE,
    locals_dict, &rm690b0_lvgl_chart_series_locals_dict
);

static const mp_rom_map_elem_t rm690b0_lvgl_chart_locals_dict_table[] = {
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

    // Chart properties and methods
    // Chart-specific API
    { MP_ROM_QSTR(MP_QSTR_type), MP_ROM_PTR(&rm690b0_lvgl_chart_type_obj) },
    { MP_ROM_QSTR(MP_QSTR_point_count), MP_ROM_PTR(&rm690b0_lvgl_chart_point_count_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_range), MP_ROM_PTR(&rm690b0_lvgl_chart_set_range_obj) },
    { MP_ROM_QSTR(MP_QSTR_add_series), MP_ROM_PTR(&rm690b0_lvgl_chart_add_series_obj) },
    { MP_ROM_QSTR(MP_QSTR_refresh), MP_ROM_PTR(&rm690b0_lvgl_chart_refresh_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_chart_locals_dict, rm690b0_lvgl_chart_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_chart_type,
    MP_QSTR_Chart,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_chart_make_new,
    locals_dict, &rm690b0_lvgl_chart_locals_dict,
    parent, &rm690b0_lvgl_widget_type
);
