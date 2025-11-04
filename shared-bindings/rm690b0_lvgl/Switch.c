// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Switch.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"

// Import Widget property objects for inheritance
extern const mp_obj_property_t rm690b0_lvgl_widget_x_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_y_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_width_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_height_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_color_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_opa_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_parent_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_delete_obj;

//| class Switch(Widget):
//|     """A switch widget for toggling options on/off with a sliding toggle."""
//|
//|     def __init__(self) -> None:
//|         """Create a new Switch widget.
//|         
//|         A switch is similar to a checkbox but styled as an on/off toggle.
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_switch_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    
    rm690b0_lvgl_switch_obj_t *self = mp_obj_malloc(rm690b0_lvgl_switch_obj_t, &rm690b0_lvgl_switch_type);
    
    // Initialize callback to None
    self->on_change_handler = mp_const_none;
    
    common_hal_rm690b0_lvgl_switch_construct(self);
    return MP_OBJ_FROM_PTR(self);
}

//|     checked: bool
//|     """The state of the switch (True=ON/False=OFF)."""
//|
static mp_obj_t rm690b0_lvgl_switch_get_checked(mp_obj_t self_in) {
    rm690b0_lvgl_switch_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(common_hal_rm690b0_lvgl_switch_get_checked(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_switch_get_checked_obj, rm690b0_lvgl_switch_get_checked);

static mp_obj_t rm690b0_lvgl_switch_set_checked(mp_obj_t self_in, mp_obj_t checked_obj) {
    rm690b0_lvgl_switch_obj_t *self = MP_OBJ_TO_PTR(self_in);
    bool checked = mp_obj_is_true(checked_obj);
    common_hal_rm690b0_lvgl_switch_set_checked(self, checked);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_switch_set_checked_obj, rm690b0_lvgl_switch_set_checked);

MP_PROPERTY_GETSET(rm690b0_lvgl_switch_checked_obj,
    (mp_obj_t)&rm690b0_lvgl_switch_get_checked_obj,
    (mp_obj_t)&rm690b0_lvgl_switch_set_checked_obj);

//|     on_change: Optional[Callable[[Switch], None]]
//|     """The callback function to run when the switch state changes.
//|     The callback receives the switch instance as an argument."""
//|
static mp_obj_t rm690b0_lvgl_switch_get_on_change(mp_obj_t self_in) {
    rm690b0_lvgl_switch_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return self->on_change_handler;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_switch_get_on_change_obj, rm690b0_lvgl_switch_get_on_change);

static mp_obj_t rm690b0_lvgl_switch_set_on_change(mp_obj_t self_in, mp_obj_t callback) {
    rm690b0_lvgl_switch_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (callback != mp_const_none && !mp_obj_is_callable(callback)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Callback must be callable or None"));
    }
    self->on_change_handler = callback;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_switch_set_on_change_obj, rm690b0_lvgl_switch_set_on_change);

MP_PROPERTY_GETSET(rm690b0_lvgl_switch_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_switch_get_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_switch_set_on_change_obj);

//|     def toggle(self) -> None:
//|         """Toggle the switch state (ON <-> OFF)."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_switch_toggle(mp_obj_t self_in) {
    rm690b0_lvgl_switch_obj_t *self = MP_OBJ_TO_PTR(self_in);
    bool current = common_hal_rm690b0_lvgl_switch_get_checked(self);
    common_hal_rm690b0_lvgl_switch_set_checked(self, !current);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_switch_toggle_obj, rm690b0_lvgl_switch_toggle);

static const mp_rom_map_elem_t rm690b0_lvgl_switch_locals_dict_table[] = {
    // Inherited Widget properties
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&rm690b0_lvgl_widget_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&rm690b0_lvgl_widget_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&rm690b0_lvgl_widget_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&rm690b0_lvgl_widget_height_obj) },
    
    // Inherited Widget methods
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_color), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_color_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_opa), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_opa_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_parent), MP_ROM_PTR(&rm690b0_lvgl_widget_set_parent_obj) },
    { MP_ROM_QSTR(MP_QSTR_delete), MP_ROM_PTR(&rm690b0_lvgl_widget_delete_obj) },
    
    // Switch-specific properties
    { MP_ROM_QSTR(MP_QSTR_checked), MP_ROM_PTR(&rm690b0_lvgl_switch_checked_obj) },
    { MP_ROM_QSTR(MP_QSTR_on_change), MP_ROM_PTR(&rm690b0_lvgl_switch_on_change_obj) },
    
    // Switch-specific methods
    { MP_ROM_QSTR(MP_QSTR_toggle), MP_ROM_PTR(&rm690b0_lvgl_switch_toggle_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_switch_locals_dict, rm690b0_lvgl_switch_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_switch_type,
    MP_QSTR_Switch,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_switch_make_new,
    locals_dict, &rm690b0_lvgl_switch_locals_dict,
    parent, &rm690b0_lvgl_widget_type
);