// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#pragma once

#include <stdbool.h>

#include "py/obj.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"

// Textarea widget object
typedef struct {
    rm690b0_lvgl_widget_obj_t base;
    mp_obj_t on_change_handler; // Python callback when text changes
    mp_obj_t on_focus_handler;  // Callback when textarea is focused/clicked
    mp_obj_t on_submit_handler; // Callback when LV_EVENT_READY fires
} rm690b0_lvgl_textarea_obj_t;

extern const mp_obj_type_t rm690b0_lvgl_textarea_type;

void common_hal_rm690b0_lvgl_textarea_construct(rm690b0_lvgl_textarea_obj_t *self);
void common_hal_rm690b0_lvgl_textarea_set_text(rm690b0_lvgl_textarea_obj_t *self, const char *text);
const char *common_hal_rm690b0_lvgl_textarea_get_text(rm690b0_lvgl_textarea_obj_t *self);
void common_hal_rm690b0_lvgl_textarea_set_placeholder(rm690b0_lvgl_textarea_obj_t *self, const char *text);
const char *common_hal_rm690b0_lvgl_textarea_get_placeholder(rm690b0_lvgl_textarea_obj_t *self);
void common_hal_rm690b0_lvgl_textarea_set_password_mode(rm690b0_lvgl_textarea_obj_t *self, bool enabled);
bool common_hal_rm690b0_lvgl_textarea_get_password_mode(rm690b0_lvgl_textarea_obj_t *self);
void common_hal_rm690b0_lvgl_textarea_set_one_line(rm690b0_lvgl_textarea_obj_t *self, bool enabled);
bool common_hal_rm690b0_lvgl_textarea_get_one_line(rm690b0_lvgl_textarea_obj_t *self);
void common_hal_rm690b0_lvgl_textarea_set_max_length(rm690b0_lvgl_textarea_obj_t *self, uint32_t max_len);
uint32_t common_hal_rm690b0_lvgl_textarea_get_max_length(rm690b0_lvgl_textarea_obj_t *self);
