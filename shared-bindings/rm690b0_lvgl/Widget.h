// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"

// Base Widget object
typedef struct {
    mp_obj_base_t base;
    void *native_obj; // lv_obj_t* handle
    mp_obj_t callback; // Python callback for widget events
} rm690b0_lvgl_widget_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_widget_type;

mp_obj_t common_hal_rm690b0_lvgl_widget_get_callback(mp_obj_t widget);
void common_hal_rm690b0_lvgl_widget_set_callback(mp_obj_t widget, mp_obj_t callback);

void common_hal_rm690b0_lvgl_widget_construct(rm690b0_lvgl_widget_obj_t *self);
void common_hal_rm690b0_lvgl_widget_deinit(rm690b0_lvgl_widget_obj_t *self);
void common_hal_rm690b0_lvgl_widget_set_parent(rm690b0_lvgl_widget_obj_t *self, rm690b0_lvgl_widget_obj_t *parent);

mp_int_t common_hal_rm690b0_lvgl_widget_get_x(rm690b0_lvgl_widget_obj_t *self);
void common_hal_rm690b0_lvgl_widget_set_x(rm690b0_lvgl_widget_obj_t *self, mp_int_t x);

mp_int_t common_hal_rm690b0_lvgl_widget_get_y(rm690b0_lvgl_widget_obj_t *self);
void common_hal_rm690b0_lvgl_widget_set_y(rm690b0_lvgl_widget_obj_t *self, mp_int_t y);

mp_int_t common_hal_rm690b0_lvgl_widget_get_width(rm690b0_lvgl_widget_obj_t *self);
void common_hal_rm690b0_lvgl_widget_set_width(rm690b0_lvgl_widget_obj_t *self, mp_int_t width);

mp_int_t common_hal_rm690b0_lvgl_widget_get_height(rm690b0_lvgl_widget_obj_t *self);
void common_hal_rm690b0_lvgl_widget_set_height(rm690b0_lvgl_widget_obj_t *self, mp_int_t height);

void common_hal_rm690b0_lvgl_widget_set_style_bg_color(rm690b0_lvgl_widget_obj_t *self, uint32_t color);
void common_hal_rm690b0_lvgl_widget_set_style_bg_opa(rm690b0_lvgl_widget_obj_t *self, uint8_t opa);
void common_hal_rm690b0_lvgl_widget_set_style_text_color(rm690b0_lvgl_widget_obj_t *self, uint32_t color);
void common_hal_rm690b0_lvgl_widget_set_style_text_font(rm690b0_lvgl_widget_obj_t *self, mp_obj_t font);