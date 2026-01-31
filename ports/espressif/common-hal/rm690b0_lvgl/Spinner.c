// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Spinner.h"
#include "common-hal/rm690b0_lvgl/Spinner.h"
#include "common-hal/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

void common_hal_rm690b0_lvgl_spinner_construct(rm690b0_lvgl_spinner_obj_t *self, mp_int_t time, mp_int_t arc_length) {
    // Create LVGL spinner on the active screen
    lv_obj_t *spinner = lv_spinner_create(lv_scr_act(), (uint32_t)time, (uint32_t)arc_length);
    if (spinner == NULL) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to create LVGL spinner"));
    }

    // Set default properties
    self->base.native_obj = spinner;
    self->base.callback = mp_const_none;
}
