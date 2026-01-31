// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include <string.h>

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Textarea.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"

// Import Widget property objects for inheritance
extern const mp_obj_property_t rm690b0_lvgl_widget_x_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_y_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_width_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_height_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_color_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_opa_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_text_color_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_text_font_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_parent_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_delete_obj;

//| class Textarea(Widget):
//|     """A multi-line text input widget."""
//|
//|     def __init__(self) -> None:
//|         """Create a new Textarea widget."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_textarea_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    rm690b0_lvgl_textarea_obj_t *self = mp_obj_malloc(rm690b0_lvgl_textarea_obj_t, &rm690b0_lvgl_textarea_type);
    self->on_change_handler = mp_const_none;
    common_hal_rm690b0_lvgl_textarea_construct(self);
    return MP_OBJ_FROM_PTR(self);
}

//|     text: str
//|     """Current text contents."""
//|
static mp_obj_t rm690b0_lvgl_textarea_get_text(mp_obj_t self_in) {
    rm690b0_lvgl_textarea_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *txt = common_hal_rm690b0_lvgl_textarea_get_text(self);
    return mp_obj_new_str(txt, strlen(txt));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_textarea_get_text_obj, rm690b0_lvgl_textarea_get_text);

static mp_obj_t rm690b0_lvgl_textarea_set_text(mp_obj_t self_in, mp_obj_t text_obj) {
    rm690b0_lvgl_textarea_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *txt = mp_obj_str_get_str(text_obj);
    common_hal_rm690b0_lvgl_textarea_set_text(self, txt);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_textarea_set_text_obj, rm690b0_lvgl_textarea_set_text);

MP_PROPERTY_GETSET(rm690b0_lvgl_textarea_text_obj,
    (mp_obj_t)&rm690b0_lvgl_textarea_get_text_obj,
    (mp_obj_t)&rm690b0_lvgl_textarea_set_text_obj);

//|     placeholder: str
//|     """Placeholder text shown when empty."""
//|
static mp_obj_t rm690b0_lvgl_textarea_get_placeholder(mp_obj_t self_in) {
    rm690b0_lvgl_textarea_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *txt = common_hal_rm690b0_lvgl_textarea_get_placeholder(self);
    return mp_obj_new_str(txt, strlen(txt));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_textarea_get_placeholder_obj, rm690b0_lvgl_textarea_get_placeholder);

static mp_obj_t rm690b0_lvgl_textarea_set_placeholder(mp_obj_t self_in, mp_obj_t text_obj) {
    rm690b0_lvgl_textarea_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *txt = mp_obj_str_get_str(text_obj);
    common_hal_rm690b0_lvgl_textarea_set_placeholder(self, txt);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_textarea_set_placeholder_obj, rm690b0_lvgl_textarea_set_placeholder);

MP_PROPERTY_GETSET(rm690b0_lvgl_textarea_placeholder_obj,
    (mp_obj_t)&rm690b0_lvgl_textarea_get_placeholder_obj,
    (mp_obj_t)&rm690b0_lvgl_textarea_set_placeholder_obj);

//|     password_mode: bool
//|     """Whether password mode is enabled (characters are masked)."""
//|
static mp_obj_t rm690b0_lvgl_textarea_get_password_mode(mp_obj_t self_in) {
    rm690b0_lvgl_textarea_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(common_hal_rm690b0_lvgl_textarea_get_password_mode(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_textarea_get_password_mode_obj, rm690b0_lvgl_textarea_get_password_mode);

static mp_obj_t rm690b0_lvgl_textarea_set_password_mode(mp_obj_t self_in, mp_obj_t enabled_obj) {
    rm690b0_lvgl_textarea_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_textarea_set_password_mode(self, mp_obj_is_true(enabled_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_textarea_set_password_mode_obj, rm690b0_lvgl_textarea_set_password_mode);

MP_PROPERTY_GETSET(rm690b0_lvgl_textarea_password_mode_obj,
    (mp_obj_t)&rm690b0_lvgl_textarea_get_password_mode_obj,
    (mp_obj_t)&rm690b0_lvgl_textarea_set_password_mode_obj);

//|     one_line: bool
//|     """If True, the textarea behaves as a single-line input."""
//|
static mp_obj_t rm690b0_lvgl_textarea_get_one_line(mp_obj_t self_in) {
    rm690b0_lvgl_textarea_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(common_hal_rm690b0_lvgl_textarea_get_one_line(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_textarea_get_one_line_obj, rm690b0_lvgl_textarea_get_one_line);

static mp_obj_t rm690b0_lvgl_textarea_set_one_line(mp_obj_t self_in, mp_obj_t enabled_obj) {
    rm690b0_lvgl_textarea_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_textarea_set_one_line(self, mp_obj_is_true(enabled_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_textarea_set_one_line_obj, rm690b0_lvgl_textarea_set_one_line);

MP_PROPERTY_GETSET(rm690b0_lvgl_textarea_one_line_obj,
    (mp_obj_t)&rm690b0_lvgl_textarea_get_one_line_obj,
    (mp_obj_t)&rm690b0_lvgl_textarea_set_one_line_obj);

//|     max_length: int
//|     """Maximum allowed characters (0 = unlimited)."""
//|
static mp_obj_t rm690b0_lvgl_textarea_get_max_length(mp_obj_t self_in) {
    rm690b0_lvgl_textarea_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int_from_uint(common_hal_rm690b0_lvgl_textarea_get_max_length(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_textarea_get_max_length_obj, rm690b0_lvgl_textarea_get_max_length);

static mp_obj_t rm690b0_lvgl_textarea_set_max_length(mp_obj_t self_in, mp_obj_t len_obj) {
    rm690b0_lvgl_textarea_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_textarea_set_max_length(self, (uint32_t)mp_obj_get_int(len_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_textarea_set_max_length_obj, rm690b0_lvgl_textarea_set_max_length);

MP_PROPERTY_GETSET(rm690b0_lvgl_textarea_max_length_obj,
    (mp_obj_t)&rm690b0_lvgl_textarea_get_max_length_obj,
    (mp_obj_t)&rm690b0_lvgl_textarea_set_max_length_obj);

//|     on_change: Optional[Callable[[Textarea], None]]
//|     """Callback invoked when the text changes."""
//|
static mp_obj_t rm690b0_lvgl_textarea_get_on_change(mp_obj_t self_in) {
    rm690b0_lvgl_textarea_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return self->on_change_handler;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_textarea_get_on_change_obj, rm690b0_lvgl_textarea_get_on_change);

static mp_obj_t rm690b0_lvgl_textarea_set_on_change(mp_obj_t self_in, mp_obj_t callback) {
    rm690b0_lvgl_textarea_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (callback != mp_const_none && !mp_obj_is_callable(callback)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Callback must be callable or None"));
    }
    self->on_change_handler = callback;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_textarea_set_on_change_obj, rm690b0_lvgl_textarea_set_on_change);

MP_PROPERTY_GETSET(rm690b0_lvgl_textarea_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_textarea_get_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_textarea_set_on_change_obj);

//|     on_focus: Optional[Callable[[Textarea], None]]
//|     """Callback invoked when the textarea receives focus (clicked/tapped)."""
//|
static mp_obj_t rm690b0_lvgl_textarea_get_on_focus(mp_obj_t self_in) {
    rm690b0_lvgl_textarea_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return self->on_focus_handler;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_textarea_get_on_focus_obj, rm690b0_lvgl_textarea_get_on_focus);

static mp_obj_t rm690b0_lvgl_textarea_set_on_focus(mp_obj_t self_in, mp_obj_t callback) {
    rm690b0_lvgl_textarea_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (callback != mp_const_none && !mp_obj_is_callable(callback)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Callback must be callable or None"));
    }
    self->on_focus_handler = callback;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_textarea_set_on_focus_obj, rm690b0_lvgl_textarea_set_on_focus);

MP_PROPERTY_GETSET(rm690b0_lvgl_textarea_on_focus_obj,
    (mp_obj_t)&rm690b0_lvgl_textarea_get_on_focus_obj,
    (mp_obj_t)&rm690b0_lvgl_textarea_set_on_focus_obj);

//|     on_submit: Optional[Callable[[Textarea], None]]
//|     """Callback invoked when the textarea reports LV_EVENT_READY (e.g. ✓ pressed)."""
//|
static mp_obj_t rm690b0_lvgl_textarea_get_on_submit(mp_obj_t self_in) {
    rm690b0_lvgl_textarea_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return self->on_submit_handler;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_textarea_get_on_submit_obj, rm690b0_lvgl_textarea_get_on_submit);

static mp_obj_t rm690b0_lvgl_textarea_set_on_submit(mp_obj_t self_in, mp_obj_t callback) {
    rm690b0_lvgl_textarea_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (callback != mp_const_none && !mp_obj_is_callable(callback)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Callback must be callable or None"));
    }
    self->on_submit_handler = callback;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_textarea_set_on_submit_obj, rm690b0_lvgl_textarea_set_on_submit);

MP_PROPERTY_GETSET(rm690b0_lvgl_textarea_on_submit_obj,
    (mp_obj_t)&rm690b0_lvgl_textarea_get_on_submit_obj,
    (mp_obj_t)&rm690b0_lvgl_textarea_set_on_submit_obj);

static const mp_rom_map_elem_t rm690b0_lvgl_textarea_locals_dict_table[] = {
    // Inherited Widget properties
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&rm690b0_lvgl_widget_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&rm690b0_lvgl_widget_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&rm690b0_lvgl_widget_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&rm690b0_lvgl_widget_height_obj) },

    // Inherited Widget methods
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_color), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_color_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_opa), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_opa_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_text_color), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_text_color_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_text_font), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_text_font_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_parent), MP_ROM_PTR(&rm690b0_lvgl_widget_set_parent_obj) },
    { MP_ROM_QSTR(MP_QSTR_delete), MP_ROM_PTR(&rm690b0_lvgl_widget_delete_obj) },

    // Textarea properties
    { MP_ROM_QSTR(MP_QSTR_text), MP_ROM_PTR(&rm690b0_lvgl_textarea_text_obj) },
    { MP_ROM_QSTR(MP_QSTR_placeholder), MP_ROM_PTR(&rm690b0_lvgl_textarea_placeholder_obj) },
    { MP_ROM_QSTR(MP_QSTR_password_mode), MP_ROM_PTR(&rm690b0_lvgl_textarea_password_mode_obj) },
    { MP_ROM_QSTR(MP_QSTR_one_line), MP_ROM_PTR(&rm690b0_lvgl_textarea_one_line_obj) },
    { MP_ROM_QSTR(MP_QSTR_max_length), MP_ROM_PTR(&rm690b0_lvgl_textarea_max_length_obj) },
    { MP_ROM_QSTR(MP_QSTR_on_change), MP_ROM_PTR(&rm690b0_lvgl_textarea_on_change_obj) },
    { MP_ROM_QSTR(MP_QSTR_on_focus), MP_ROM_PTR(&rm690b0_lvgl_textarea_on_focus_obj) },
    { MP_ROM_QSTR(MP_QSTR_on_submit), MP_ROM_PTR(&rm690b0_lvgl_textarea_on_submit_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_textarea_locals_dict, rm690b0_lvgl_textarea_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_textarea_type,
    MP_QSTR_Textarea,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_textarea_make_new,
    locals_dict, &rm690b0_lvgl_textarea_locals_dict,
    parent, &rm690b0_lvgl_widget_type
    );
