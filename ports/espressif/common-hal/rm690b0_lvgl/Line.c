// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Line.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

void common_hal_rm690b0_lvgl_line_construct(rm690b0_lvgl_line_obj_t *self) {
    lv_obj_t *line = lv_line_create(lv_scr_act());
    if (line == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL line"));
    }
    self->base.native_obj = line;
    self->base.callback = mp_const_none;
    lv_obj_set_style_line_width(line, 2, LV_PART_MAIN);
    lv_obj_set_style_line_color(line, lv_color_hex(0xffffff), LV_PART_MAIN);
}

void common_hal_rm690b0_lvgl_line_deinit(rm690b0_lvgl_line_obj_t *self) {
    if (self->points != NULL) {
        m_free(self->points);
        self->points = NULL;
        self->point_count = 0;
    }
    if (self->base.native_obj != NULL) {
        lv_obj_del((lv_obj_t *)self->base.native_obj);
        self->base.native_obj = NULL;
    }
}

void common_hal_rm690b0_lvgl_line_set_points(rm690b0_lvgl_line_obj_t *self, const mp_int_t *coords, size_t coord_count) {
    if ((coord_count & 1) != 0 || coord_count < 4) {
        mp_raise_ValueError(MP_ERROR_TEXT("Invalid point list"));
    }
    lv_obj_t *line = (lv_obj_t *)self->base.native_obj;
    if (line == NULL) {
        return;
    }

    size_t point_count = coord_count / 2;
    lv_point_t *points = m_new(lv_point_t, point_count);
    for (size_t i = 0; i < point_count; i++) {
        points[i].x = (lv_coord_t)coords[i * 2];
        points[i].y = (lv_coord_t)coords[i * 2 + 1];
    }

    if (self->points != NULL) {
        m_free(self->points);
    }
    self->points = points;
    self->point_count = point_count;
    lv_line_set_points(line, points, (uint16_t)point_count);
}

void common_hal_rm690b0_lvgl_line_set_y_invert(rm690b0_lvgl_line_obj_t *self, bool invert) {
    lv_obj_t *line = (lv_obj_t *)self->base.native_obj;
    if (line == NULL) {
        return;
    }
    lv_line_set_y_invert(line, invert);
}

bool common_hal_rm690b0_lvgl_line_get_y_invert(rm690b0_lvgl_line_obj_t *self) {
    lv_obj_t *line = (lv_obj_t *)self->base.native_obj;
    if (line == NULL) {
        return false;
    }
    return lv_line_get_y_invert(line);
}

void common_hal_rm690b0_lvgl_line_set_line_width(rm690b0_lvgl_line_obj_t *self, mp_int_t width) {
    if (width < 1) {
        width = 1;
    }
    lv_obj_t *line = (lv_obj_t *)self->base.native_obj;
    if (line == NULL) {
        return;
    }
    lv_obj_set_style_line_width(line, width, LV_PART_MAIN);
}

mp_int_t common_hal_rm690b0_lvgl_line_get_line_width(rm690b0_lvgl_line_obj_t *self) {
    lv_obj_t *line = (lv_obj_t *)self->base.native_obj;
    if (line == NULL) {
        return 0;
    }
    return lv_obj_get_style_line_width(line, LV_PART_MAIN);
}

void common_hal_rm690b0_lvgl_line_set_line_color(rm690b0_lvgl_line_obj_t *self, uint32_t color) {
    lv_obj_t *line = (lv_obj_t *)self->base.native_obj;
    if (line == NULL) {
        return;
    }
    lv_obj_set_style_line_color(line, lv_color_hex(color), LV_PART_MAIN);
}

uint32_t common_hal_rm690b0_lvgl_line_get_line_color(rm690b0_lvgl_line_obj_t *self) {
    lv_obj_t *line = (lv_obj_t *)self->base.native_obj;
    if (line == NULL) {
        return 0;
    }
    lv_color_t color = lv_obj_get_style_line_color(line, LV_PART_MAIN);
    return lv_color_to32(color);
}

void common_hal_rm690b0_lvgl_line_set_rounded(rm690b0_lvgl_line_obj_t *self, bool rounded) {
    lv_obj_t *line = (lv_obj_t *)self->base.native_obj;
    if (line == NULL) {
        return;
    }
    lv_obj_set_style_line_rounded(line, rounded, LV_PART_MAIN);
}

bool common_hal_rm690b0_lvgl_line_get_rounded(rm690b0_lvgl_line_obj_t *self) {
    lv_obj_t *line = (lv_obj_t *)self->base.native_obj;
    if (line == NULL) {
        return false;
    }
    return lv_obj_get_style_line_rounded(line, LV_PART_MAIN);
}
