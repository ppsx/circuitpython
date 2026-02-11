// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/rm690b0_lvgl/Roller.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"
#include "shared-bindings/util.h"

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

//| class Roller(Widget):
//|     """Roller widget for selecting from a list of options."""
//|
//|     def __init__(self, options: str = "") -> None:
//|         """Create a Roller widget.
//|
//|         :param str options: Options separated by newlines (e.g. "Option 1\\nOption 2")
//|         """
//|         ...
static mp_obj_t rm690b0_lvgl_roller_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_options };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_options, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rm690b0_lvgl_roller_obj_t *self = mp_obj_malloc(rm690b0_lvgl_roller_obj_t, &rm690b0_lvgl_roller_type);
    common_hal_rm690b0_lvgl_roller_construct(self);

    if (args[ARG_options].u_obj != mp_const_none) {
        const char *options = mp_obj_str_get_str(args[ARG_options].u_obj);
        common_hal_rm690b0_lvgl_roller_set_options(self, options, 0); // 0 = LV_ROLLER_MODE_NORMAL
    }

    return MP_OBJ_FROM_PTR(self);
}

//|     options: str
//|     """The options available in the roller, separated by newlines.
//|     Setting this property updates the roller options."""
static mp_obj_t rm690b0_lvgl_roller_get_options(mp_obj_t self_in) {
    rm690b0_lvgl_roller_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *options = common_hal_rm690b0_lvgl_roller_get_options(self);
    if (options == NULL) {
        return mp_const_empty_bytes;
    }
    return mp_obj_new_str(options, strlen(options));
}

static mp_obj_t rm690b0_lvgl_roller_set_options(mp_obj_t self_in, mp_obj_t options_obj) {
    rm690b0_lvgl_roller_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *options = mp_obj_str_get_str(options_obj);
    common_hal_rm690b0_lvgl_roller_set_options(self, options, 0); // 0 = LV_ROLLER_MODE_NORMAL
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_roller_set_options_obj, rm690b0_lvgl_roller_set_options);
MP_PROPERTY_GETSET(rm690b0_lvgl_roller_options_obj,
    (mp_obj_t)&rm690b0_lvgl_roller_get_options,
    (mp_obj_t)&rm690b0_lvgl_roller_set_options_obj);

//|     selected: int
//|     """The index of the currently selected option."""
static mp_obj_t rm690b0_lvgl_roller_get_selected(mp_obj_t self_in) {
    rm690b0_lvgl_roller_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(common_hal_rm690b0_lvgl_roller_get_selected(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_roller_get_selected_obj, rm690b0_lvgl_roller_get_selected);

static mp_obj_t rm690b0_lvgl_roller_set_selected(mp_obj_t self_in, mp_obj_t index_obj) {
    rm690b0_lvgl_roller_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t index = mp_obj_get_int(index_obj);
    common_hal_rm690b0_lvgl_roller_set_selected(self, index, true); // Use animation by default
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_roller_set_selected_obj, rm690b0_lvgl_roller_set_selected);
MP_PROPERTY_GETSET(rm690b0_lvgl_roller_selected_obj,
    (mp_obj_t)&rm690b0_lvgl_roller_get_selected_obj,
    (mp_obj_t)&rm690b0_lvgl_roller_set_selected_obj);

//|     selected_str: str
//|     """The text of the currently selected option (read-only)."""
static mp_obj_t rm690b0_lvgl_roller_get_selected_str(mp_obj_t self_in) {
    rm690b0_lvgl_roller_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *str = common_hal_rm690b0_lvgl_roller_get_selected_str(self);
    return mp_obj_new_str(str, strlen(str));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_roller_get_selected_str_obj, rm690b0_lvgl_roller_get_selected_str);
MP_PROPERTY_GETSET(rm690b0_lvgl_roller_selected_str_obj,
    (mp_obj_t)&rm690b0_lvgl_roller_get_selected_str_obj,
    MP_ROM_NONE);

//|     visible_row_count: int
//|     """The number of visible rows in the roller."""
static mp_obj_t rm690b0_lvgl_roller_set_visible_row_count(mp_obj_t self_in, mp_obj_t count_obj) {
    rm690b0_lvgl_roller_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t count = mp_obj_get_int(count_obj);
    common_hal_rm690b0_lvgl_roller_set_visible_row_count(self, count);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_roller_set_visible_row_count_obj, rm690b0_lvgl_roller_set_visible_row_count);

static mp_obj_t rm690b0_lvgl_roller_get_visible_row_count(mp_obj_t self_in) {
    // LVGL doesn't have a direct get function for this, returning None
    // or we could store it in the object struct if needed.
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_roller_get_visible_row_count_obj, rm690b0_lvgl_roller_get_visible_row_count);
MP_PROPERTY_GETSET(rm690b0_lvgl_roller_visible_row_count_obj,
    (mp_obj_t)&rm690b0_lvgl_roller_get_visible_row_count_obj,
    (mp_obj_t)&rm690b0_lvgl_roller_set_visible_row_count_obj);

//|     on_change: Optional[Callable[[Roller], None]]
//|     """Callback function when selection changes."""
static mp_obj_t rm690b0_lvgl_roller_get_on_change(mp_obj_t self_in) {
    rm690b0_lvgl_roller_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return common_hal_rm690b0_lvgl_widget_get_callback(MP_OBJ_FROM_PTR(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_roller_get_on_change_obj, rm690b0_lvgl_roller_get_on_change);

static mp_obj_t rm690b0_lvgl_roller_set_on_change(mp_obj_t self_in, mp_obj_t callback) {
    rm690b0_lvgl_roller_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_widget_set_callback(MP_OBJ_FROM_PTR(self), callback);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_roller_set_on_change_obj, rm690b0_lvgl_roller_set_on_change);
MP_PROPERTY_GETSET(rm690b0_lvgl_roller_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_roller_get_on_change_obj,
    (mp_obj_t)&rm690b0_lvgl_roller_set_on_change_obj);

static const mp_rom_map_elem_t rm690b0_lvgl_roller_locals_dict_table[] = {
    // Properties
    { MP_ROM_QSTR(MP_QSTR_options), MP_ROM_PTR(&rm690b0_lvgl_roller_options_obj) },
    { MP_ROM_QSTR(MP_QSTR_selected), MP_ROM_PTR(&rm690b0_lvgl_roller_selected_obj) },
    { MP_ROM_QSTR(MP_QSTR_selected_str), MP_ROM_PTR(&rm690b0_lvgl_roller_selected_str_obj) },
    { MP_ROM_QSTR(MP_QSTR_visible_row_count), MP_ROM_PTR(&rm690b0_lvgl_roller_visible_row_count_obj) },
    { MP_ROM_QSTR(MP_QSTR_on_change), MP_ROM_PTR(&rm690b0_lvgl_roller_on_change_obj) },

    // Inherited from Widget
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&rm690b0_lvgl_widget_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&rm690b0_lvgl_widget_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&rm690b0_lvgl_widget_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&rm690b0_lvgl_widget_height_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_color), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_color_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_bg_opa), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_bg_opa_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_text_color), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_text_color_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_style_text_font), MP_ROM_PTR(&rm690b0_lvgl_widget_set_style_text_font_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_parent), MP_ROM_PTR(&rm690b0_lvgl_widget_set_parent_obj) },
    { MP_ROM_QSTR(MP_QSTR_delete), MP_ROM_PTR(&rm690b0_lvgl_widget_delete_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_roller_locals_dict, rm690b0_lvgl_roller_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_roller_type,
    MP_QSTR_Roller,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_roller_make_new,
    locals_dict, &rm690b0_lvgl_roller_locals_dict,
    parent, &rm690b0_lvgl_widget_type
    );
