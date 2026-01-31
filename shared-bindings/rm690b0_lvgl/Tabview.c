// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Tabview.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"
#include "shared-bindings/rm690b0_lvgl/Container.h"

// Import Widget property objects for inheritance
extern const mp_obj_property_t rm690b0_lvgl_widget_x_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_y_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_width_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_height_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_color_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_opa_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_parent_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_delete_obj;

//| class Tabview(Widget):
//|     """A widget to organize content in tabs."""
//|
//|     def __init__(self, tab_pos: int = 4, tab_size: int = 50) -> None:
//|         """Create a new Tabview widget.
//|
//|         :param int tab_pos: Position of tabs (DIR_TOP, DIR_BOTTOM, DIR_LEFT, DIR_RIGHT). Default is DIR_TOP.
//|         :param int tab_size: Size of the tab bar (height for top/bottom, width for left/right). Default is 50.
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_tabview_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_tab_pos, ARG_tab_size };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_tab_pos, MP_ARG_INT, {.u_int = 4} }, // LV_DIR_TOP = 4
        { MP_QSTR_tab_size, MP_ARG_INT, {.u_int = 50} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rm690b0_lvgl_tabview_obj_t *self = mp_obj_malloc(rm690b0_lvgl_tabview_obj_t, &rm690b0_lvgl_tabview_type);
    self->on_change_handler = mp_const_none;
    common_hal_rm690b0_lvgl_tabview_construct(self, args[ARG_tab_pos].u_int, args[ARG_tab_size].u_int);
    return MP_OBJ_FROM_PTR(self);
}

//|     on_change: Optional[Callable[[Tabview], None]]
//|     """The callback function to run when the active tab changes."""
//|
static mp_obj_t rm690b0_lvgl_tabview_get_on_change(mp_obj_t self_in) {
    rm690b0_lvgl_tabview_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return self->on_change_handler;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_tabview_get_on_change_obj, rm690b0_lvgl_tabview_get_on_change);

static mp_obj_t rm690b0_lvgl_tabview_set_on_change(mp_obj_t self_in, mp_obj_t callback) {
    rm690b0_lvgl_tabview_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (callback != mp_const_none && !mp_obj_is_callable(callback)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Callback must be callable or None"));
    }
    self->on_change_handler = callback;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_tabview_set_on_change_obj, rm690b0_lvgl_tabview_set_on_change);

MP_PROPERTY_GETSET(rm690b0_lvgl_tabview_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_tabview_get_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_tabview_set_on_change_obj);

//|     active_tab: int
//|     """The index of the currently active tab."""
//|
static mp_obj_t rm690b0_lvgl_tabview_get_active_tab(mp_obj_t self_in) {
    rm690b0_lvgl_tabview_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_tabview_get_active_tab(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_tabview_get_active_tab_obj, rm690b0_lvgl_tabview_get_active_tab);

static mp_obj_t rm690b0_lvgl_tabview_set_active_tab(mp_obj_t self_in, mp_obj_t idx_obj) {
    rm690b0_lvgl_tabview_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_tabview_set_active_tab(self, (uint16_t)mp_obj_get_int(idx_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_tabview_set_active_tab_obj, rm690b0_lvgl_tabview_set_active_tab);

MP_PROPERTY_GETSET(rm690b0_lvgl_tabview_active_tab_obj,
    (mp_obj_t)&rm690b0_lvgl_tabview_get_active_tab_obj,
    (mp_obj_t)&rm690b0_lvgl_tabview_set_active_tab_obj);

//|     def add_tab(self, name: str) -> Container:
//|         """Add a new tab.
//|
//|         :param str name: The title of the tab
//|         :return: A Container widget representing the tab's content area
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_tabview_add_tab(mp_obj_t self_in, mp_obj_t name_obj) {
    rm690b0_lvgl_tabview_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *name = mp_obj_str_get_str(name_obj);
    return common_hal_rm690b0_lvgl_tabview_add_tab(self, name);
}
static MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_tabview_add_tab_obj, rm690b0_lvgl_tabview_add_tab);

static const mp_rom_map_elem_t rm690b0_lvgl_tabview_locals_dict_table[] = {
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

    // Tabview specific
    { MP_ROM_QSTR(MP_QSTR_add_tab), MP_ROM_PTR(&rm690b0_lvgl_tabview_add_tab_obj) },
    { MP_ROM_QSTR(MP_QSTR_on_change), MP_ROM_PTR(&rm690b0_lvgl_tabview_on_change_obj) },
    { MP_ROM_QSTR(MP_QSTR_active_tab), MP_ROM_PTR(&rm690b0_lvgl_tabview_active_tab_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_tabview_locals_dict, rm690b0_lvgl_tabview_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_tabview_type,
    MP_QSTR_Tabview,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_tabview_make_new,
    locals_dict, &rm690b0_lvgl_tabview_locals_dict,
    parent, &rm690b0_lvgl_widget_type
    );
