// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Table.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

void common_hal_rm690b0_lvgl_table_construct(rm690b0_lvgl_table_obj_t *self) {
    lv_obj_t *table = lv_table_create(lv_scr_act());
    if (table == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL table"));
        return;
    }

    self->base.native_obj = table;
    self->base.callback = mp_const_none;
}

void common_hal_rm690b0_lvgl_table_set_cell_value(rm690b0_lvgl_table_obj_t *self, uint16_t row, uint16_t col, const char *text) {
    lv_obj_t *table = (lv_obj_t *)self->base.native_obj;
    lv_table_set_cell_value(table, row, col, text);
}

void common_hal_rm690b0_lvgl_table_set_row_cnt(rm690b0_lvgl_table_obj_t *self, uint16_t row_cnt) {
    lv_obj_t *table = (lv_obj_t *)self->base.native_obj;
    lv_table_set_row_cnt(table, row_cnt);
}

void common_hal_rm690b0_lvgl_table_set_col_cnt(rm690b0_lvgl_table_obj_t *self, uint16_t col_cnt) {
    lv_obj_t *table = (lv_obj_t *)self->base.native_obj;
    lv_table_set_col_cnt(table, col_cnt);
}

void common_hal_rm690b0_lvgl_table_set_col_width(rm690b0_lvgl_table_obj_t *self, uint16_t col, uint16_t width) {
    lv_obj_t *table = (lv_obj_t *)self->base.native_obj;
    lv_table_set_col_width(table, col, (lv_coord_t)width);
}
