// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"

// Spinner object structure
typedef struct {
    rm690b0_lvgl_widget_obj_t base;
} rm690b0_lvgl_spinner_obj_t;

// Type object declaration
extern const mp_obj_type_t rm690b0_lvgl_spinner_type;

// Function declarations
void common_hal_rm690b0_lvgl_spinner_construct(rm690b0_lvgl_spinner_obj_t *self, mp_int_t time, mp_int_t arc_length);