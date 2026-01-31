// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/builtin.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/stream.h"
#include "shared-bindings/rm690b0_lvgl/Font.h"
#include "lvgl.h"
// Include the specific header for Tiny TTF.
// Note: Depending on build system include paths, this might need adjustment,
// but based on component structure this path is relative to LVGL root or component root.
#include "src/extra/libs/tiny_ttf/lv_tiny_ttf.h"

void common_hal_rm690b0_lvgl_font_construct(rm690b0_lvgl_font_obj_t *self, mp_obj_t file_or_data, mp_int_t size) {
    lv_font_t *font = NULL;

    // If a string is provided, assume it's a file path and read it into memory
    if (mp_obj_is_str(file_or_data)) {
        mp_obj_t args[2] = {
            file_or_data,
            MP_OBJ_NEW_QSTR(MP_QSTR_rb),
        };
        mp_obj_t file = mp_vfs_open(2, args, (mp_map_t *)&mp_const_empty_map);
        mp_obj_t read_method = mp_load_attr(file, MP_QSTR_read);
        mp_obj_t content = mp_call_function_0(read_method);
        mp_stream_close(file);

        self->source = content;
    } else {
        self->source = file_or_data;
    }

    mp_buffer_info_t bufinfo;
    if (mp_get_buffer(self->source, &bufinfo, MP_BUFFER_READ)) {
        // We must ensure the buffer stays alive as long as the font exists
        // self->source holds the reference.
        font = lv_tiny_ttf_create_data(bufinfo.buf, bufinfo.len, (lv_coord_t)size);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("file_or_data must be a string path or a bytes-like object"));
    }

    if (font == NULL) {
        mp_raise_RuntimeError(MP_ERROR_TEXT("Failed to load font"));
    }

    self->native_font = font;
}

void common_hal_rm690b0_lvgl_font_set_size(rm690b0_lvgl_font_obj_t *self, mp_int_t size) {
    if (self->native_font == NULL) {
        return;
    }
    lv_tiny_ttf_set_size((lv_font_t *)self->native_font, (lv_coord_t)size);
}

void common_hal_rm690b0_lvgl_font_deinit(rm690b0_lvgl_font_obj_t *self) {
    if (self->native_font != NULL) {
        lv_tiny_ttf_destroy((lv_font_t *)self->native_font);
        self->native_font = NULL;
    }
}
