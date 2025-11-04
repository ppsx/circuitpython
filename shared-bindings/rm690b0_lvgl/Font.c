// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Font.h"

//| class Font:
//|     """A font loaded from a TTF file or data using Tiny TTF."""
//|
//|     def __init__(self, file_or_data: Union[str, ReadableBuffer], size: int) -> None:
//|         """Load a font from a file path or data buffer.
//|
//|         :param file_or_data: Path to the TTF file or bytes containing TTF data
//|         :param int size: Font size in pixels
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_font_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_file_or_data, ARG_size };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_file_or_data, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_size, MP_ARG_REQUIRED | MP_ARG_INT },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rm690b0_lvgl_font_obj_t *self = mp_obj_malloc(rm690b0_lvgl_font_obj_t, &rm690b0_lvgl_font_type);

    common_hal_rm690b0_lvgl_font_construct(self, args[ARG_file_or_data].u_obj, args[ARG_size].u_int);
    return MP_OBJ_FROM_PTR(self);
}

//|     def set_size(self, size: int) -> None:
//|         """Set the font size in pixels."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_font_set_size(mp_obj_t self_in, mp_obj_t size_obj) {
    rm690b0_lvgl_font_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_font_set_size(self, mp_obj_get_int(size_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_font_set_size_obj, rm690b0_lvgl_font_set_size);

//|     def deinit(self) -> None:
//|         """Free the resources associated with the font."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_font_deinit(mp_obj_t self_in) {
    rm690b0_lvgl_font_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_font_deinit(self);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_font_deinit_obj, rm690b0_lvgl_font_deinit);

//|     def __enter__(self) -> Font:
//|         """No-op used by Context Managers."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_font_enter(mp_obj_t self_in) {
    return self_in;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_font_enter_obj, rm690b0_lvgl_font_enter);

//|     def __exit__(self) -> None:
//|         """Automatically deinitializes the font when exiting a context."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_font_exit(size_t n_args, const mp_obj_t *args) {
    (void)n_args;
    common_hal_rm690b0_lvgl_font_deinit(MP_OBJ_TO_PTR(args[0]));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rm690b0_lvgl_font_exit_obj, 4, 4, rm690b0_lvgl_font_exit);

static const mp_rom_map_elem_t rm690b0_lvgl_font_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_set_size), MP_ROM_PTR(&rm690b0_lvgl_font_set_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&rm690b0_lvgl_font_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&rm690b0_lvgl_font_enter_obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&rm690b0_lvgl_font_exit_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_font_locals_dict, rm690b0_lvgl_font_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_font_type,
    MP_QSTR_Font,
    MP_TYPE_FLAG_NONE,
    make_new, rm690b0_lvgl_font_make_new,
    locals_dict, &rm690b0_lvgl_font_locals_dict
);