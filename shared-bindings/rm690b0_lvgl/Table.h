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
} rm690b0_lvgl_table_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_table_type;

void common_hal_rm690b0_lvgl_table_construct(rm690b0_lvgl_table_obj_t *self);
void common_hal_rm690b0_lvgl_table_set_cell_value(rm690b0_lvgl_table_obj_t *self, uint16_t row, uint16_t col, const char *text);
void common_hal_rm690b0_lvgl_table_set_row_cnt(rm690b0_lvgl_table_obj_t *self, uint16_t row_cnt);
void common_hal_rm690b0_lvgl_table_set_col_cnt(rm690b0_lvgl_table_obj_t *self, uint16_t col_cnt);
void common_hal_rm690b0_lvgl_table_set_col_width(rm690b0_lvgl_table_obj_t *self, uint16_t col, uint16_t width);