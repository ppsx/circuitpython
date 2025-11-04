// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"
#include "shared-bindings/rm690b0_lvgl/Tabview.h"
#include "common-hal/rm690b0_lvgl/Widget.h"

uint16_t common_hal_rm690b0_lvgl_tabview_get_active_tab(rm690b0_lvgl_tabview_obj_t *self);
void common_hal_rm690b0_lvgl_tabview_set_active_tab(rm690b0_lvgl_tabview_obj_t *self, uint16_t idx);