// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"

typedef struct {
    rm690b0_lvgl_widget_obj_t base;
} rm690b0_lvgl_container_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_container_type;

void common_hal_rm690b0_lvgl_container_construct(rm690b0_lvgl_container_obj_t *self);
void common_hal_rm690b0_lvgl_container_set_flex_flow(rm690b0_lvgl_container_obj_t *self, mp_int_t flow);
void common_hal_rm690b0_lvgl_container_set_flex_align(rm690b0_lvgl_container_obj_t *self, mp_int_t main_place, mp_int_t cross_place, mp_int_t track_cross_place);
void common_hal_rm690b0_lvgl_container_set_padding(rm690b0_lvgl_container_obj_t *self, mp_int_t padding);