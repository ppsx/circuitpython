// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Canvas.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

static size_t canvas_buffer_size(lv_img_cf_t color_format, mp_int_t width, mp_int_t height) {
    switch (color_format) {
        case LV_IMG_CF_TRUE_COLOR:
            return LV_IMG_BUF_SIZE_TRUE_COLOR(width, height);
        case LV_IMG_CF_TRUE_COLOR_ALPHA:
            return LV_IMG_BUF_SIZE_TRUE_COLOR_ALPHA(width, height);
        default:
            mp_raise_ValueError(MP_ERROR_TEXT("Unsupported color_format"));
    }
}

void common_hal_rm690b0_lvgl_canvas_construct(rm690b0_lvgl_canvas_obj_t *self, mp_int_t width, mp_int_t height, mp_int_t color_format) {
    if (width <= 0 || height <= 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("width and height must be > 0"));
    }

    lv_obj_t *canvas = lv_canvas_create(lv_scr_act());
    if (canvas == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL canvas"));
    }
    self->base.native_obj = canvas;

    lv_img_cf_t cf = (lv_img_cf_t)color_format;
    size_t buf_size = canvas_buffer_size(cf, width, height);
    uint8_t *buffer = m_malloc(buf_size);
    if (buffer == NULL) {
        lv_obj_del(canvas);
        self->base.native_obj = NULL;
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Canvas buffer alloc failed"));
    }

    self->buffer = buffer;
    self->buffer_size = buf_size;
    self->color_format = color_format;
    self->buf_width = width;
    self->buf_height = height;
    self->base.callback = mp_const_none;

    lv_canvas_set_buffer(canvas, buffer, (lv_coord_t)width, (lv_coord_t)height, cf);
    lv_canvas_fill_bg(canvas, lv_color_hex(0x000000), LV_OPA_TRANSP);
}

void common_hal_rm690b0_lvgl_canvas_deinit(rm690b0_lvgl_canvas_obj_t *self) {
    if (self->buffer != NULL) {
        m_free(self->buffer);
        self->buffer = NULL;
        self->buffer_size = 0;
    }
    if (self->base.native_obj != NULL) {
        lv_obj_del((lv_obj_t *)self->base.native_obj);
        self->base.native_obj = NULL;
    }
}

static lv_obj_t *canvas_native(rm690b0_lvgl_canvas_obj_t *self) {
    return (lv_obj_t *)self->base.native_obj;
}

void common_hal_rm690b0_lvgl_canvas_fill_bg(rm690b0_lvgl_canvas_obj_t *self, uint32_t color, mp_int_t opacity) {
    lv_obj_t *canvas = canvas_native(self);
    if (canvas == NULL) {
        return;
    }
    if (opacity < 0) {
        opacity = 0;
    } else if (opacity > 255) {
        opacity = 255;
    }
    lv_canvas_fill_bg(canvas, lv_color_hex(color), (lv_opa_t)opacity);
}

void common_hal_rm690b0_lvgl_canvas_set_px(rm690b0_lvgl_canvas_obj_t *self, mp_int_t x, mp_int_t y, uint32_t color, mp_int_t opacity) {
    lv_obj_t *canvas = canvas_native(self);
    if (canvas == NULL) {
        return;
    }
    lv_canvas_set_px_color(canvas, (lv_coord_t)x, (lv_coord_t)y, lv_color_hex(color));
    if (opacity >= 0 && opacity <= 255 && self->color_format == LV_IMG_CF_TRUE_COLOR_ALPHA) {
        lv_canvas_set_px_opa(canvas, (lv_coord_t)x, (lv_coord_t)y, (lv_opa_t)opacity);
    }
}

void common_hal_rm690b0_lvgl_canvas_draw_line(rm690b0_lvgl_canvas_obj_t *self, const mp_int_t *coords, size_t point_count, uint32_t color, mp_int_t width) {
    if (point_count < 2) {
        mp_raise_ValueError(MP_ERROR_TEXT("Need at least two points"));
    }

    lv_obj_t *canvas = canvas_native(self);
    if (canvas == NULL) {
        return;
    }

    lv_point_t *points = m_new(lv_point_t, point_count);
    for (size_t i = 0; i < point_count; i++) {
        points[i].x = (lv_coord_t)coords[i * 2];
        points[i].y = (lv_coord_t)coords[i * 2 + 1];
    }

    lv_draw_line_dsc_t dsc;
    lv_draw_line_dsc_init(&dsc);
    dsc.color = lv_color_hex(color);
    dsc.width = width < 1 ? 1 : width;

    lv_canvas_draw_line(canvas, points, (uint32_t)point_count, &dsc);
    m_del(lv_point_t, points, point_count);
}
