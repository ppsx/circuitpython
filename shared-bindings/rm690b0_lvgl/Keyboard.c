// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include <string.h>
#include "lvgl.h"
#include "shared-bindings/rm690b0_lvgl/Keyboard.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"
#include "shared-bindings/rm690b0_lvgl/Textarea.h"

// Import Widget property objects for inheritance
extern const mp_obj_property_t rm690b0_lvgl_widget_x_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_y_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_width_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_height_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_color_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_opa_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_parent_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_delete_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_parent_obj;

//| class Keyboard(Widget):
//|     """On-screen keyboard widget."""
//|
//|     def __init__(self) -> None:
//|         """Create a new Keyboard widget."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_keyboard_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    rm690b0_lvgl_keyboard_obj_t *self = mp_obj_malloc(rm690b0_lvgl_keyboard_obj_t, &rm690b0_lvgl_keyboard_type);
    self->on_change_handler = mp_const_none;
    common_hal_rm690b0_lvgl_keyboard_construct(self);
    return MP_OBJ_FROM_PTR(self);
}

//|     def set_textarea(self, textarea: Optional[Textarea]) -> None:
//|         """Assign a Textarea to receive keyboard input."""
//|         ...
static mp_obj_t rm690b0_lvgl_keyboard_set_textarea(mp_obj_t self_in, mp_obj_t ta_obj) {
    rm690b0_lvgl_keyboard_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_keyboard_set_textarea(self, ta_obj);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_keyboard_set_textarea_obj, rm690b0_lvgl_keyboard_set_textarea);

//|     def set_mode(self, mode: int) -> None:
//|         """Set keyboard mode (see KBD_MODE_* constants)."""
//|         ...
static mp_obj_t rm690b0_lvgl_keyboard_set_mode(mp_obj_t self_in, mp_obj_t mode_obj) {
    rm690b0_lvgl_keyboard_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_keyboard_set_mode(self, (uint8_t)mp_obj_get_int(mode_obj));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_keyboard_set_mode_obj, rm690b0_lvgl_keyboard_set_mode);

//|     mode: int
//|     """Current keyboard mode."""
//|
static mp_obj_t rm690b0_lvgl_keyboard_get_mode(mp_obj_t self_in) {
    rm690b0_lvgl_keyboard_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int_from_uint(common_hal_rm690b0_lvgl_keyboard_get_mode(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_keyboard_get_mode_obj, rm690b0_lvgl_keyboard_get_mode);
MP_PROPERTY_GETSET(rm690b0_lvgl_keyboard_mode_obj,
    (mp_obj_t)&rm690b0_lvgl_keyboard_get_mode_obj,
    (mp_obj_t)&rm690b0_lvgl_keyboard_set_mode_obj);

//|     popovers: bool
//|     """Show popovers on key press (default False)."""
//|
static mp_obj_t rm690b0_lvgl_keyboard_get_popovers(mp_obj_t self_in) {
    rm690b0_lvgl_keyboard_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(common_hal_rm690b0_lvgl_keyboard_get_popovers(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_keyboard_get_popovers_obj, rm690b0_lvgl_keyboard_get_popovers);

static mp_obj_t rm690b0_lvgl_keyboard_set_popovers(mp_obj_t self_in, mp_obj_t enabled_obj) {
    rm690b0_lvgl_keyboard_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_keyboard_set_popovers(self, mp_obj_is_true(enabled_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_keyboard_set_popovers_obj, rm690b0_lvgl_keyboard_set_popovers);

MP_PROPERTY_GETSET(rm690b0_lvgl_keyboard_popovers_obj,
    (mp_obj_t)&rm690b0_lvgl_keyboard_get_popovers_obj,
    (mp_obj_t)&rm690b0_lvgl_keyboard_set_popovers_obj);

//|     selected_btn_text: str
//|     """Text of the currently selected key."""
//|
static mp_obj_t rm690b0_lvgl_keyboard_get_selected_btn_text(mp_obj_t self_in) {
    rm690b0_lvgl_keyboard_obj_t *self = MP_OBJ_TO_PTR(self_in);
    lv_obj_t *kb = (lv_obj_t *)self->base.native_obj;
    int32_t idx = lv_btnmatrix_get_selected_btn(kb);
    if (idx < 0) {
        return mp_obj_new_str("", 0);
    }
    const char *txt = lv_btnmatrix_get_btn_text(kb, idx);
    if (txt == NULL) {
        return mp_obj_new_str("", 0);
    }
    return mp_obj_new_str(txt, strlen(txt));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_keyboard_get_selected_btn_text_obj, rm690b0_lvgl_keyboard_get_selected_btn_text);
MP_PROPERTY_GETTER(rm690b0_lvgl_keyboard_selected_btn_text_obj,
    (mp_obj_t)&rm690b0_lvgl_keyboard_get_selected_btn_text_obj);

// Explicit setter method for convenience
static mp_obj_t rm690b0_lvgl_keyboard_set_popovers_method(mp_obj_t self_in, mp_obj_t enabled_obj) {
    rm690b0_lvgl_keyboard_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_keyboard_set_popovers(self, mp_obj_is_true(enabled_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_keyboard_set_popovers_method_obj, rm690b0_lvgl_keyboard_set_popovers_method);

//|     on_change: Optional[Callable[[Keyboard], None]]
//|     """Callback invoked on key press/value change."""
//|
static mp_obj_t rm690b0_lvgl_keyboard_get_on_change(mp_obj_t self_in) {
    rm690b0_lvgl_keyboard_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return self->on_change_handler;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_keyboard_get_on_change_obj, rm690b0_lvgl_keyboard_get_on_change);

static mp_obj_t rm690b0_lvgl_keyboard_set_on_change(mp_obj_t self_in, mp_obj_t callback) {
    rm690b0_lvgl_keyboard_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (callback != mp_const_none && !mp_obj_is_callable(callback)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Callback must be callable or None"));
    }
    self->on_change_handler = callback;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_keyboard_set_on_change_obj, rm690b0_lvgl_keyboard_set_on_change);

MP_PROPERTY_GETSET(rm690b0_lvgl_keyboard_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_keyboard_get_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_keyboard_set_on_change_obj);

static const mp_rom_map_elem_t rm690b0_lvgl_keyboard_locals_dict_table[] = {
    // Inherited Widget properties
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&rm690b0_lvgl_widget_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&rm690b0_lvgl_widget_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&rm690b0_lvgl_widget_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&rm690b0_lvgl_widget_height_obj) },

    // Inherited Widget methods
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_color), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_color_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_color), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_color_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_opa), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_opa_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_parent), MP_ROM_PTR(&rm690b0_lvgl_widget_set_parent_obj) },
    { MP_ROM_QSTR(MP_QSTR_delete), MP_ROM_PTR(&rm690b0_lvgl_widget_delete_obj) },

    // Keyboard specific
    // Keyboard methods
    { MP_ROM_QSTR(MP_QSTR_set_textarea), MP_ROM_PTR(&rm690b0_lvgl_keyboard_set_textarea_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_mode), MP_ROM_PTR(&rm690b0_lvgl_keyboard_set_mode_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_popovers), MP_ROM_PTR(&rm690b0_lvgl_keyboard_set_popovers_method_obj) },

    // Properties
    { MP_ROM_QSTR(MP_QSTR_mode), MP_ROM_PTR(&rm690b0_lvgl_keyboard_mode_obj) },
    { MP_ROM_QSTR(MP_QSTR_popovers), MP_ROM_PTR(&rm690b0_lvgl_keyboard_popovers_obj) },
    { MP_ROM_QSTR(MP_QSTR_on_change), MP_ROM_PTR(&rm690b0_lvgl_keyboard_on_change_obj) },
    { MP_ROM_QSTR(MP_QSTR_selected_btn_text), MP_ROM_PTR(&rm690b0_lvgl_keyboard_selected_btn_text_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_keyboard_locals_dict, rm690b0_lvgl_keyboard_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_keyboard_type,
    MP_QSTR_Keyboard,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_keyboard_make_new,
    locals_dict, &rm690b0_lvgl_keyboard_locals_dict,
    parent, &rm690b0_lvgl_widget_type
);
