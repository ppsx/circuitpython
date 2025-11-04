// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "shared-bindings/rm690b0_lvgl/RM690B0_LVGL.h"
#include "py/runtime.h"
#include "py/objproperty.h"
#include "shared-bindings/busio/I2C.h"

//| class RM690B0_LVGL:
//|     """LVGL integration for RM690B0 AMOLED display
//|
//|     This class provides LVGL integration for the RM690B0 AMOLED display
//|     on the Waveshare ESP32-S3 Touch AMOLED 2.41 board. It wraps the
//|     esp_lvgl_port component and provides initialization for both the
//|     display and touch input.
//|     """
//|
//|     def __init__(self) -> None:
//|         """Construct a new RM690B0_LVGL instance
//|
//|         This initializes the internal state but does not initialize
//|         the display or touch. Call init_display() and init_touch()
//|         to complete initialization.
//|
//|         Example::
//|
//|             import rm690b0_lvgl
//|             lvgl = rm690b0_lvgl.RM690B0_LVGL()
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_rm690b0_lvgl_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    
    rm690b0_lvgl_rm690b0_lvgl_obj_t *self = mp_obj_malloc(rm690b0_lvgl_rm690b0_lvgl_obj_t, &rm690b0_lvgl_rm690b0_lvgl_type);
    common_hal_rm690b0_lvgl_rm690b0_lvgl_construct(self);
    
    return MP_OBJ_FROM_PTR(self);
}

//|     def deinit(self) -> None:
//|         """Deinitialize the LVGL integration
//|
//|         This releases all resources used by the LVGL integration,
//|         including display and touch drivers. After calling this,
//|         the object can no longer be used.
//|
//|         Example::
//|
//|             lvgl.deinit()
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_rm690b0_lvgl_deinit(mp_obj_t self_in) {
    rm690b0_lvgl_rm690b0_lvgl_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_rm690b0_lvgl_deinit(self);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_rm690b0_lvgl_deinit_obj, rm690b0_lvgl_rm690b0_lvgl_deinit);

//|     def __enter__(self) -> RM690B0_LVGL:
//|         """Context manager entry (no-op, returns self)"""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_rm690b0_lvgl___enter__(mp_obj_t self_in) {
    return self_in;
}
static MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_rm690b0_lvgl___enter___obj, rm690b0_lvgl_rm690b0_lvgl___enter__);

//|     def __exit__(self) -> None:
//|         """Context manager exit, automatically deinitializes"""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_rm690b0_lvgl___exit__(size_t n_args, const mp_obj_t *args) {
    (void)n_args;
    common_hal_rm690b0_lvgl_rm690b0_lvgl_deinit(MP_OBJ_TO_PTR(args[0]));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rm690b0_lvgl_rm690b0_lvgl___exit___obj, 4, 4, rm690b0_lvgl_rm690b0_lvgl___exit__);

//|     def init_display(self) -> None:
//|         """Initialize the LVGL display driver
//|
//|         This function initializes the LVGL display driver using the existing
//|         RM690B0 panel handle. It sets up the flush callback that will render
//|         LVGL content to the AMOLED display.
//|
//|         This must be called before creating any LVGL objects.
//|
//|         Example::
//|
//|             lvgl = rm690b0_lvgl.RM690B0_LVGL()
//|             lvgl.init_display()
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_init_display(mp_obj_t self_in) {
    rm690b0_lvgl_rm690b0_lvgl_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_init_display(self);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_init_display_obj, rm690b0_lvgl_init_display);

//|     def init_touch(self, i2c: busio.I2C) -> None:
//|         """Initialize the LVGL touch input driver
//|
//|         This function initializes the LVGL touch input driver using the
//|         provided I2C bus. It sets up the input callback that will
//|         feed touch events to LVGL widgets.
//|
//|         init_display() must be called before init_touch().
//|
//|         :param busio.I2C i2c: The I2C bus connected to the touch controller
//|
//|         Example::
//|
//|             import board
//|             import busio
//|             import rm690b0_lvgl
//|
//|             lvgl = rm690b0_lvgl.RM690B0_LVGL()
//|             lvgl.init_display()
//|
//|             i2c = busio.I2C(board.SCL, board.SDA)
//|             lvgl.init_touch(i2c)
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_init_touch(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_i2c };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_i2c, MP_ARG_REQUIRED | MP_ARG_OBJ },
    };
    rm690b0_lvgl_rm690b0_lvgl_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    mp_obj_t i2c = args[ARG_i2c].u_obj;
    if (!mp_obj_is_type(i2c, &busio_i2c_type)) {
        mp_raise_TypeError_varg(MP_ERROR_TEXT("Expected a %q"), busio_i2c_type.name);
    }
    common_hal_rm690b0_lvgl_init_touch(self, i2c);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(rm690b0_lvgl_init_touch_obj, 1, rm690b0_lvgl_init_touch);

//|     def task_handler(self) -> None:
//|         """Process LVGL tasks
//|
//|         This function should be called periodically to process LVGL tasks
//|         such as animations, timers, and input handling. In most cases,
//|         LVGL will handle this automatically through its tick timer.
//|
//|         Example::
//|
//|             import time
//|             
//|             lvgl = rm690b0_lvgl.RM690B0_LVGL()
//|             lvgl.init_display()
//|             lvgl.init_touch()
//|             
//|             while True:
//|                 lvgl.task_handler()
//|                 time.sleep(0.01)
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_task_handler(mp_obj_t self_in) {
    rm690b0_lvgl_rm690b0_lvgl_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_task_handler(self);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_task_handler_obj, rm690b0_lvgl_task_handler);

//|     def init_rendering(self) -> None:
//|         """Initialize LVGL rendering subsystem before touch initialization
//|
//|         This function explicitly initializes LVGL's image rendering subsystem
//|         by creating and rendering a temporary off-screen image. This must be
//|         called BEFORE init_touch() to prevent a race condition where touch
//|         callbacks fire during first-time image cache initialization, which
//|         can cause board resets.
//|
//|         Call this after init_display() but before init_touch().
//|
//|         Example::
//|
//|             import board
//|             import busio
//|             import rm690b0_lvgl
//|
//|             lvgl = rm690b0_lvgl.RM690B0_LVGL()
//|             lvgl.init_display()
//|             
//|             # CRITICAL: Initialize rendering before touch
//|             lvgl.init_rendering()
//|             
//|             # Now safe to initialize touch
//|             i2c = busio.I2C(board.SCL, board.SDA)
//|             lvgl.init_touch(i2c)
//|
//|         Note: If you don't call this function, you can also avoid the issue
//|         by loading and rendering at least one image before calling init_touch().
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_init_rendering(mp_obj_t self_in) {
    rm690b0_lvgl_rm690b0_lvgl_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_init_rendering(self);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_init_rendering_obj, rm690b0_lvgl_init_rendering);

//|     def test_draw(self) -> None:
//|         """Draw a test pattern to verify LVGL rendering
//|
//|         This is a diagnostic function that creates simple LVGL widgets
//|         to verify that the display driver is working correctly.
//|
//|         Example::
//|
//|             lvgl = rm690b0_lvgl.RM690B0_LVGL()
//|             lvgl.init_display()
//|             lvgl.test_draw()
//|         """
//|         ...
//|
static mp_obj_t rm690b0_lvgl_test_draw(mp_obj_t self_in) {
    rm690b0_lvgl_rm690b0_lvgl_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_rm690b0_lvgl_test_draw(self);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_test_draw_obj, rm690b0_lvgl_test_draw);

//|     def scroll_screen(self, *, x: int = 0, y: int = 0, animated: bool = False) -> None:
//|         """Scroll the active screen to the given coordinate."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_scroll_screen(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_x, ARG_y, ARG_animated };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_animated, MP_ARG_BOOL, {.u_bool = false} },
    };
    rm690b0_lvgl_rm690b0_lvgl_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    common_hal_rm690b0_lvgl_scroll_screen(
        self,
        args[ARG_x].u_int,
        args[ARG_y].u_int,
        args[ARG_animated].u_bool);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(rm690b0_lvgl_scroll_screen_obj, 1, rm690b0_lvgl_scroll_screen);

//|     def get_scroll_y(self) -> int:
//|         """Return the current vertical scroll offset of the active screen."""
//|         ...
//|
static mp_obj_t rm690b0_lvgl_get_scroll_y(mp_obj_t self_in) {
    rm690b0_lvgl_rm690b0_lvgl_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_get_scroll_y(self));
}
static MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_get_scroll_y_obj, rm690b0_lvgl_get_scroll_y);

//|     width: int
//|     """Display width in pixels (read-only)"""
//|
static mp_obj_t rm690b0_lvgl_get_width(mp_obj_t self_in) {
    rm690b0_lvgl_rm690b0_lvgl_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_get_width(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_get_width_obj, rm690b0_lvgl_get_width);

MP_PROPERTY_GETTER(rm690b0_lvgl_width_obj,
    (mp_obj_t)&rm690b0_lvgl_get_width_obj);

//|     height: int
//|     """Display height in pixels (read-only)"""
//|
static mp_obj_t rm690b0_lvgl_get_height(mp_obj_t self_in) {
    rm690b0_lvgl_rm690b0_lvgl_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_rm690b0_lvgl_get_height(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(rm690b0_lvgl_get_height_obj, rm690b0_lvgl_get_height);

MP_PROPERTY_GETTER(rm690b0_lvgl_height_obj,
    (mp_obj_t)&rm690b0_lvgl_get_height_obj);

static const mp_rom_map_elem_t rm690b0_lvgl_rm690b0_lvgl_locals_dict_table[] = {
    // Methods
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&rm690b0_lvgl_rm690b0_lvgl_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&rm690b0_lvgl_rm690b0_lvgl___enter___obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&rm690b0_lvgl_rm690b0_lvgl___exit___obj) },
    { MP_ROM_QSTR(MP_QSTR_init_display), MP_ROM_PTR(&rm690b0_lvgl_init_display_obj) },
    { MP_ROM_QSTR(MP_QSTR_init_touch), MP_ROM_PTR(&rm690b0_lvgl_init_touch_obj) },
    { MP_ROM_QSTR(MP_QSTR_task_handler), MP_ROM_PTR(&rm690b0_lvgl_task_handler_obj) },
    { MP_ROM_QSTR(MP_QSTR_init_rendering), MP_ROM_PTR(&rm690b0_lvgl_init_rendering_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_draw), MP_ROM_PTR(&rm690b0_lvgl_test_draw_obj) },
    { MP_ROM_QSTR(MP_QSTR_scroll_screen), MP_ROM_PTR(&rm690b0_lvgl_scroll_screen_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_scroll_y), MP_ROM_PTR(&rm690b0_lvgl_get_scroll_y_obj) },
    
    // Properties
    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&rm690b0_lvgl_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&rm690b0_lvgl_height_obj) },
};
static MP_DEFINE_CONST_DICT(rm690b0_lvgl_rm690b0_lvgl_locals_dict, rm690b0_lvgl_rm690b0_lvgl_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rm690b0_lvgl_rm690b0_lvgl_type,
    MP_QSTR_RM690B0_LVGL,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rm690b0_lvgl_rm690b0_lvgl_make_new,
    locals_dict, &rm690b0_lvgl_rm690b0_lvgl_locals_dict
);
