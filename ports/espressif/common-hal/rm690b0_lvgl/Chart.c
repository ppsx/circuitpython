// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Chart.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

#define DEFAULT_CHART_WIDTH  260
#define DEFAULT_CHART_HEIGHT 150

static lv_chart_type_t sanitize_chart_type(mp_int_t chart_type) {
    switch (chart_type) {
        case LV_CHART_TYPE_LINE:
        case LV_CHART_TYPE_BAR:
        case LV_CHART_TYPE_SCATTER:
            return (lv_chart_type_t)chart_type;
        default:
            return LV_CHART_TYPE_LINE;
    }
}

void common_hal_rm690b0_lvgl_chart_construct(rm690b0_lvgl_chart_obj_t *self, mp_int_t chart_type) {
    lv_obj_t *chart = lv_chart_create(lv_scr_act());
    if (chart == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL chart"));
    }

    self->base.native_obj = chart;
    self->base.callback = mp_const_none;

    lv_obj_set_size(chart, DEFAULT_CHART_WIDTH, DEFAULT_CHART_HEIGHT);
    lv_chart_set_point_count(chart, 10);
    lv_chart_set_type(chart, sanitize_chart_type(chart_type));
}

void common_hal_rm690b0_lvgl_chart_set_type(rm690b0_lvgl_chart_obj_t *self, mp_int_t chart_type) {
    lv_obj_t *chart = (lv_obj_t *)self->base.native_obj;
    if (chart == NULL) {
        return;
    }
    lv_chart_set_type(chart, sanitize_chart_type(chart_type));
}

mp_int_t common_hal_rm690b0_lvgl_chart_get_type(rm690b0_lvgl_chart_obj_t *self) {
    lv_obj_t *chart = (lv_obj_t *)self->base.native_obj;
    if (chart == NULL) {
        return LV_CHART_TYPE_NONE;
    }
    return lv_chart_get_type(chart);
}

void common_hal_rm690b0_lvgl_chart_set_point_count(rm690b0_lvgl_chart_obj_t *self, mp_int_t count) {
    if (count <= 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("point_count must be > 0"));
    }
    lv_obj_t *chart = (lv_obj_t *)self->base.native_obj;
    if (chart == NULL) {
        return;
    }
    lv_chart_set_point_count(chart, (uint16_t)count);
}

mp_int_t common_hal_rm690b0_lvgl_chart_get_point_count(rm690b0_lvgl_chart_obj_t *self) {
    lv_obj_t *chart = (lv_obj_t *)self->base.native_obj;
    if (chart == NULL) {
        return 0;
    }
    return lv_chart_get_point_count(chart);
}

void common_hal_rm690b0_lvgl_chart_set_range(rm690b0_lvgl_chart_obj_t *self, mp_int_t axis, mp_int_t min_value, mp_int_t max_value) {
    lv_obj_t *chart = (lv_obj_t *)self->base.native_obj;
    if (chart == NULL) {
        return;
    }
    if (max_value < min_value) {
        mp_raise_ValueError(MP_ERROR_TEXT("max_value must be >= min_value"));
    }
    lv_chart_set_range(chart, (lv_chart_axis_t)axis, (lv_coord_t)min_value, (lv_coord_t)max_value);
}

mp_obj_t common_hal_rm690b0_lvgl_chart_add_series(rm690b0_lvgl_chart_obj_t *self, uint32_t color, mp_int_t axis) {
    lv_obj_t *chart = (lv_obj_t *)self->base.native_obj;
    if (chart == NULL) {
        return mp_const_none;
    }

    lv_chart_series_t *series = lv_chart_add_series(chart, lv_color_hex(color), (lv_chart_axis_t)axis);
    if (series == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to allocate chart series"));
    }

    rm690b0_lvgl_chart_series_obj_t *py_series =
        mp_obj_malloc(rm690b0_lvgl_chart_series_obj_t, &rm690b0_lvgl_chart_series_type);
    py_series->chart = self;
    py_series->series = series;

    return MP_OBJ_FROM_PTR(py_series);
}

void common_hal_rm690b0_lvgl_chart_refresh(rm690b0_lvgl_chart_obj_t *self) {
    lv_obj_t *chart = (lv_obj_t *)self->base.native_obj;
    if (chart == NULL) {
        return;
    }
    lv_chart_refresh(chart);
}

static lv_chart_series_t *chart_series_native(rm690b0_lvgl_chart_series_obj_t *series_obj, lv_obj_t **chart_out) {
    if (chart_out != NULL) {
        *chart_out = NULL;
    }
    if (series_obj == NULL || series_obj->chart == NULL) {
        return NULL;
    }
    lv_obj_t *chart = (lv_obj_t *)series_obj->chart->base.native_obj;
    if (chart_out != NULL) {
        *chart_out = chart;
    }
    return (lv_chart_series_t *)series_obj->series;
}

void common_hal_rm690b0_lvgl_chart_series_set_points(rm690b0_lvgl_chart_series_obj_t *series_obj, size_t value_count, const mp_int_t *values) {
    lv_obj_t *chart = NULL;
    lv_chart_series_t *series = chart_series_native(series_obj, &chart);
    if (chart == NULL || series == NULL) {
        return;
    }
    uint16_t point_count = lv_chart_get_point_count(chart);
    size_t limit = value_count < point_count ? value_count : point_count;
    for (size_t i = 0; i < limit; i++) {
        lv_chart_set_value_by_id(chart, series, (uint16_t)i, (lv_coord_t)values[i]);
    }
    lv_chart_refresh(chart);
}

void common_hal_rm690b0_lvgl_chart_series_set_point(rm690b0_lvgl_chart_series_obj_t *series_obj, mp_int_t index, mp_int_t value) {
    if (index < 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("index must be >= 0"));
    }
    lv_obj_t *chart = NULL;
    lv_chart_series_t *series = chart_series_native(series_obj, &chart);
    if (chart == NULL || series == NULL) {
        return;
    }
    lv_chart_set_value_by_id(chart, series, (uint16_t)index, (lv_coord_t)value);
}

void common_hal_rm690b0_lvgl_chart_series_append(rm690b0_lvgl_chart_series_obj_t *series_obj, mp_int_t value) {
    lv_obj_t *chart = NULL;
    lv_chart_series_t *series = chart_series_native(series_obj, &chart);
    if (chart == NULL || series == NULL) {
        return;
    }
    lv_chart_set_next_value(chart, series, (lv_coord_t)value);
}

void common_hal_rm690b0_lvgl_chart_series_set_color(rm690b0_lvgl_chart_series_obj_t *series_obj, uint32_t color) {
    lv_obj_t *chart = NULL;
    lv_chart_series_t *series = chart_series_native(series_obj, &chart);
    if (chart == NULL || series == NULL) {
        return;
    }
    lv_chart_set_series_color(chart, series, lv_color_hex(color));
}
