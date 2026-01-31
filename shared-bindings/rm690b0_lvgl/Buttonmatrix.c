// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include <string.h>
#include "shared-bindings/rm690b0_lvgl/Buttonmatrix.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"
#include "lvgl.h"

// Import Widget property objects for inheritance
extern const mp_obj_property_t rm690b0_lvgl_widget_x_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_y_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_width_obj;
extern const mp_obj_property_t rm690b0_lvgl_widget_height_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_color_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_style_bg_opa_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_set_parent_obj;
extern const mp_obj_fun_builtin_fixed_t rm690b0_lvgl_widget_delete_obj;

//| class Buttonmatrix(Widget):
//|     """A matrix of buttons."""
//|
//|     def __init__(self, buttons: Optional[List[str]] = None) -> None:
//|         """Create a new Buttonmatrix widget.
//|
//|         :param List[str] buttons: List of button texts. Use "\n" for new row.
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_buttonmatrix_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_buttons };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_buttons, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rm690b0_lvgl_buttonmatrix_obj_t *self = mp_obj_malloc(rm690b0_lvgl_buttonmatrix_obj_t, &rm690b0_lvgl_buttonmatrix_type);

    self->on_click_handler = mp_const_none;
    self->buttons_list = mp_const_none;
    self->btn_map = NULL;

    common_hal_rm690b0_lvgl_buttonmatrix_construct(self, args[ARG_buttons].u_obj);
    return MP_OBJ_FROM_PTR(self);
}

//|     def set_map(self, buttons: List[str]) -> None:
//|         """Set the button texts.
//|
//|         :param List[str] buttons: List of button texts. Use "\n" for new row.
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_buttonmatrix_set_map(mp_obj_t self_in, mp_obj_t buttons) {
    rm690b0_lvgl_buttonmatrix_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_buttonmatrix_set_map(self, buttons);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_buttonmatrix_set_map_obj, rm690b0_lvgl_buttonmatrix_set_map);

//|     selected_btn: int
//|     """The index of the currently selected button."""
//|
static mp_obj_t rm690b0_lvgl_buttonmatrix_get_selected_btn(mp_obj_t self_in) {
    rm690b0_lvgl_buttonmatrix_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_buttonmatrix_get_selected_btn(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_buttonmatrix_get_selected_btn_obj, rm690b0_lvgl_buttonmatrix_get_selected_btn);

static mp_obj_t rm690b0_lvgl_buttonmatrix_set_selected_btn(mp_obj_t self_in, mp_obj_t idx_obj) {
    rm690b0_lvgl_buttonmatrix_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_buttonmatrix_set_selected_btn(self, (uint16_t)mp_obj_get_int(idx_obj));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_buttonmatrix_set_selected_btn_obj, rm690b0_lvgl_buttonmatrix_set_selected_btn);

MP_PROPERTY_GETSET(rm690b0_lvgl_buttonmatrix_selected_btn_obj,
    (mp_obj_t)&rm690b0_lvgl_buttonmatrix_get_selected_btn_obj,
    (mp_obj_t)&rm690b0_lvgl_buttonmatrix_set_selected_btn_obj);

//|     on_click: Optional[Callable[[Buttonmatrix], None]]
//|     """The callback function to run when a button is clicked."""
//|
static mp_obj_t rm690b0_lvgl_buttonmatrix_get_on_click(mp_obj_t self_in) {
    rm690b0_lvgl_buttonmatrix_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return self->on_click_handler;
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_buttonmatrix_get_on_click_obj, rm690b0_lvgl_buttonmatrix_get_on_click);

static mp_obj_t rm690b0_lvgl_buttonmatrix_set_on_click(mp_obj_t self_in, mp_obj_t callback) {
    rm690b0_lvgl_buttonmatrix_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (callback != mp_const_none && !mp_obj_is_callable(callback)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Callback must be callable or None"));
    }
    self->on_click_handler = callback;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm690b0_lvgl_buttonmatrix_set_on_click_obj, rm690b0_lvgl_buttonmatrix_set_on_click);

MP_PROPERTY_GETSET(rm690b0_lvgl_buttonmatrix_on_click_obj,
    (mp_obj_t)&rm690b0_lvgl_buttonmatrix_get_on_click_obj,
    (mp_obj_t)&rm690b0_lvgl_buttonmatrix_set_on_click_obj);

//|     selected_btn_text: str
//|     """Text of the currently selected button."""
//|
static mp_obj_t rm690b0_lvgl_buttonmatrix_get_selected_btn_text(mp_obj_t self_in) {
    rm690b0_lvgl_buttonmatrix_obj_t *self = MP_OBJ_TO_PTR(self_in);
    lv_obj_t *btnm = (lv_obj_t *)self->base.native_obj;
    int32_t idx = lv_btnmatrix_get_selected_btn(btnm);
    if (idx < 0) {
        return mp_obj_new_str("", 0);
    }
    const char *txt = lv_btnmatrix_get_btn_text(btnm, idx);
    if (txt == NULL) {
        return mp_obj_new_str("", 0);
    }
    return mp_obj_new_str(txt, strlen(txt));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_buttonmatrix_get_selected_btn_text_obj, rm690b0_lvgl_buttonmatrix_get_selected_btn_text);
MP_PROPERTY_GETTER(rm690b0_lvgl_buttonmatrix_selected_btn_text_obj,
    (mp_obj_t)&rm690b0_lvgl_buttonmatrix_get_selected_btn_text_obj);

static const mp_rom_map_elem_t rm690b0_lvgl_buttonmatrix_locals_dict_table[] = {
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

    // Buttonmatrix-specific properties
    // Buttonmatrix methods
    { MP_ROM_QSTR(MP_QSTR_set_map), MP_ROM_PTR(&rm690b0_lvgl_buttonmatrix_set_map_obj) },

    // Properties
    { MP_ROM_QSTR(MP_QSTR_selected_btn), MP_ROM_PTR(&rm690b0_lvgl_buttonmatrix_selected_btn_obj) },
    { MP_ROM_QSTR(MP_QSTR_on_click), MP_ROM_PTR(&rm690b0_lvgl_buttonmatrix_on_click_obj) },
    { MP_ROM_QSTR(MP_QSTR_selected_btn_text), MP_ROM_PTR(&rm690b0_lvgl_buttonmatrix_selected_btn_text_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_buttonmatrix_locals_dict, rm690b0_lvgl_buttonmatrix_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_buttonmatrix_type,
    MP_QSTR_Buttonmatrix,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_buttonmatrix_make_new,
    locals_dict, &rm690b0_lvgl_buttonmatrix_locals_dict,
    parent, &rm690b0_lvgl_widget_type
    );
