// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Scale.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

#define DEFAULT_SCALE_WIDTH  200
#define DEFAULT_SCALE_HEIGHT 200

static void ensure_scale_valid(rm690b0_lvgl_scale_obj_t *self) {
    if (self->scale == NULL || self->indicator == NULL) {
        mp_raise_RuntimeError(MP_ERROR_TEXT("Scale not initialized"));
    }
}

void common_hal_rm690b0_lvgl_scale_construct(rm690b0_lvgl_scale_obj_t *self, mp_int_t min_value, mp_int_t max_value) {
    if (max_value < min_value) {
        mp_raise_ValueError(MP_ERROR_TEXT("max_value must be >= min_value"));
    }

    lv_obj_t *meter = lv_meter_create(lv_scr_act());
    if (meter == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL meter"));
    }
    self->base.native_obj = meter;
    self->base.callback = mp_const_none;

    self->scale = lv_meter_add_scale(meter);
    if (self->scale == NULL) {
        lv_obj_del(meter);
        self->base.native_obj = NULL;
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create scale"));
    }

    lv_meter_scale_t *scale = (lv_meter_scale_t *)self->scale;
    lv_meter_set_scale_ticks(meter, scale, 41, 2, 8, lv_color_hex(0x888888));
    lv_meter_set_scale_major_ticks(meter, scale, 5, 4, 16, lv_color_hex(0xffffff), 12);
    lv_meter_set_scale_range(meter, scale, min_value, max_value, 270, 135);

    self->indicator = lv_meter_add_needle_line(meter, scale, 6, lv_color_hex(0x00aaff), -20);
    if (self->indicator == NULL) {
        lv_obj_del(meter);
        self->base.native_obj = NULL;
        self->scale = NULL;
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create indicator"));
    }

    self->min_value = min_value;
    self->max_value = max_value;
    self->angle_range = 270;
    self->rotation = 135;

    lv_obj_set_size(meter, DEFAULT_SCALE_WIDTH, DEFAULT_SCALE_HEIGHT);
    common_hal_rm690b0_lvgl_scale_set_value(self, min_value);
}

void common_hal_rm690b0_lvgl_scale_deinit(rm690b0_lvgl_scale_obj_t *self) {
    if (self->base.native_obj != NULL) {
        lv_obj_del((lv_obj_t *)self->base.native_obj);
        self->base.native_obj = NULL;
    }
    self->scale = NULL;
    self->indicator = NULL;
}

void common_hal_rm690b0_lvgl_scale_set_range(rm690b0_lvgl_scale_obj_t *self, mp_int_t min_value, mp_int_t max_value) {
    if (max_value < min_value) {
        mp_raise_ValueError(MP_ERROR_TEXT("max_value must be >= min_value"));
    }
    ensure_scale_valid(self);
    lv_meter_set_scale_range(
        (lv_obj_t *)self->base.native_obj,
        (lv_meter_scale_t *)self->scale,
        min_value,
        max_value,
        self->angle_range,
        self->rotation);
    self->min_value = min_value;
    self->max_value = max_value;
}

void common_hal_rm690b0_lvgl_scale_set_angles(rm690b0_lvgl_scale_obj_t *self, mp_int_t angle_range, mp_int_t rotation) {
    if (angle_range <= 0 || angle_range > 360) {
        mp_raise_ValueError(MP_ERROR_TEXT("angle_range must be 1-360"));
    }
    ensure_scale_valid(self);
    lv_meter_set_scale_range(
        (lv_obj_t *)self->base.native_obj,
        (lv_meter_scale_t *)self->scale,
        self->min_value,
        self->max_value,
        angle_range,
        rotation);
    self->angle_range = angle_range;
    self->rotation = rotation;
}

void common_hal_rm690b0_lvgl_scale_set_value(rm690b0_lvgl_scale_obj_t *self, mp_int_t value) {
    ensure_scale_valid(self);
    if (value < self->min_value) {
        value = self->min_value;
    } else if (value > self->max_value) {
        value = self->max_value;
    }
    lv_meter_set_indicator_value((lv_obj_t *)self->base.native_obj, (lv_meter_indicator_t *)self->indicator, value);
}

mp_int_t common_hal_rm690b0_lvgl_scale_get_value(rm690b0_lvgl_scale_obj_t *self) {
    // LVGL meter does not expose getter; track via indicator start_value
    if (self->indicator == NULL) {
        return self->min_value;
    }
    lv_meter_indicator_t *indicator = (lv_meter_indicator_t *)self->indicator;
    return indicator->start_value;
}

void common_hal_rm690b0_lvgl_scale_set_ticks(rm690b0_lvgl_scale_obj_t *self,
    mp_int_t tick_count, mp_int_t tick_width, mp_int_t tick_length, uint32_t tick_color) {
    ensure_scale_valid(self);
    lv_meter_set_scale_ticks(
        (lv_obj_t *)self->base.native_obj,
        (lv_meter_scale_t *)self->scale,
        tick_count,
        tick_width,
        tick_length,
        lv_color_hex(tick_color));
}

void common_hal_rm690b0_lvgl_scale_set_major_ticks(rm690b0_lvgl_scale_obj_t *self,
    mp_int_t nth, mp_int_t width, mp_int_t length, uint32_t color, mp_int_t label_gap) {
    ensure_scale_valid(self);
    lv_meter_set_scale_major_ticks(
        (lv_obj_t *)self->base.native_obj,
        (lv_meter_scale_t *)self->scale,
        nth,
        width,
        length,
        lv_color_hex(color),
        label_gap);
}
