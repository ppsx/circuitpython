// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Przemyslaw Patrick Socha
//
// SPDX-License-Identifier: MIT

#include "py/obj.h"
#include "py/runtime.h"

#include "shared-bindings/rm690b0_lvgl/RM690B0_LVGL.h"
#include "shared-bindings/rm690b0_lvgl/Widget.h"
#include "shared-bindings/rm690b0_lvgl/Label.h"
#include "shared-bindings/rm690b0_lvgl/Button.h"
#include "shared-bindings/rm690b0_lvgl/Slider.h"
#include "shared-bindings/rm690b0_lvgl/Checkbox.h"
#include "shared-bindings/rm690b0_lvgl/Switch.h"
#include "shared-bindings/rm690b0_lvgl/Bar.h"
#include "shared-bindings/rm690b0_lvgl/Arc.h"
#include "shared-bindings/rm690b0_lvgl/Dropdown.h"
#include "shared-bindings/rm690b0_lvgl/Roller.h"
#include "shared-bindings/rm690b0_lvgl/Spinner.h"
#include "shared-bindings/rm690b0_lvgl/Container.h"
#include "shared-bindings/rm690b0_lvgl/Msgbox.h"
#include "shared-bindings/rm690b0_lvgl/List.h"
#include "shared-bindings/rm690b0_lvgl/Spinbox.h"
#include "shared-bindings/rm690b0_lvgl/Tabview.h"
#include "shared-bindings/rm690b0_lvgl/Table.h"
#include "shared-bindings/rm690b0_lvgl/Buttonmatrix.h"
#include "shared-bindings/rm690b0_lvgl/Textarea.h"
#include "shared-bindings/rm690b0_lvgl/Keyboard.h"
#include "shared-bindings/rm690b0_lvgl/Chart.h"
#include "shared-bindings/rm690b0_lvgl/Canvas.h"
#include "shared-bindings/rm690b0_lvgl/Line.h"
#include "shared-bindings/rm690b0_lvgl/Scale.h"
#include "shared-bindings/rm690b0_lvgl/Font.h"
#include "lvgl.h"

//| """RM690B0 LVGL Integration
//|
//| This module provides LVGL (LittlevGL) integration for the RM690B0 AMOLED display.
//| It wraps the esp_lvgl_port component and provides a simple API for initializing
//| the display and touch input for use with LVGL.
//|
//| Example::
//|
//|     import rm690b0_lvgl
//|
//|     # Initialize LVGL with the RM690B0 display
//|     lvgl = rm690b0_lvgl.RM690B0_LVGL()
//|     lvgl.init_display()
//|     lvgl.init_touch()
//|
//|     # LVGL is now ready to use
//|     # Use standard LVGL API through lv module
//| """
//|

//| def init_display() -> None:
//|     """Initialize the LVGL display driver
//|
//|     This function initializes the LVGL display driver using the existing
//|     RM690B0 panel handle. It sets up the flush callback that will render
//|     LVGL content to the AMOLED display.
//|
//|     This must be called before creating any LVGL objects.
//|
//|     Example::
//|
//|         import rm690b0_lvgl
//|         lvgl = rm690b0_lvgl.RM690B0_LVGL()
//|         lvgl.init_display()
//|     """
//|     ...
//|

//| def init_touch() -> None:
//|     """Initialize the LVGL touch input driver
//|
//|     This function initializes the LVGL touch input driver using the
//|     FT6336U touch controller. It sets up the input callback that will
//|     feed touch events to LVGL widgets.
//|
//|     init_display() must be called before init_touch().
//|
//|     Example::
//|
//|         import rm690b0_lvgl
//|         lvgl = rm690b0_lvgl.RM690B0_LVGL()
//|         lvgl.init_display()
//|         lvgl.init_touch()
//|     """
//|     ...
//|

//| def task_handler() -> None:
//|     """Process LVGL tasks
//|
//|     This function should be called periodically to process LVGL tasks
//|     such as animations, timers, and input handling. In most cases,
//|     LVGL will handle this automatically through its tick timer.
//|
//|     Example::
//|
//|         import rm690b0_lvgl
//|         import time
//|
//|         lvgl = rm690b0_lvgl.RM690B0_LVGL()
//|         lvgl.init_display()
//|         lvgl.init_touch()
//|
//|         while True:
//|             lvgl.task_handler()
//|             time.sleep(0.01)
//|     """
//|     ...
//|

static const mp_rom_map_elem_t rm690b0_lvgl_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_rm690b0_lvgl) },
    { MP_ROM_QSTR(MP_QSTR_RM690B0_LVGL), MP_ROM_PTR(&rm690b0_lvgl_rm690b0_lvgl_type) },
    { MP_ROM_QSTR(MP_QSTR_Widget), MP_ROM_PTR(&rm690b0_lvgl_widget_type) },
    { MP_ROM_QSTR(MP_QSTR_Label), MP_ROM_PTR(&rm690b0_lvgl_label_type) },
    { MP_ROM_QSTR(MP_QSTR_Button), MP_ROM_PTR(&rm690b0_lvgl_button_type) },
    { MP_ROM_QSTR(MP_QSTR_Slider), MP_ROM_PTR(&rm690b0_lvgl_slider_type) },
    { MP_ROM_QSTR(MP_QSTR_Checkbox), MP_ROM_PTR(&rm690b0_lvgl_checkbox_type) },
    { MP_ROM_QSTR(MP_QSTR_Switch), MP_ROM_PTR(&rm690b0_lvgl_switch_type) },
    { MP_ROM_QSTR(MP_QSTR_Bar), MP_ROM_PTR(&rm690b0_lvgl_bar_type) },
    { MP_ROM_QSTR(MP_QSTR_Arc), MP_ROM_PTR(&rm690b0_lvgl_arc_type) },
    { MP_ROM_QSTR(MP_QSTR_Dropdown), MP_ROM_PTR(&rm690b0_lvgl_dropdown_type) },
    { MP_ROM_QSTR(MP_QSTR_Roller), MP_ROM_PTR(&rm690b0_lvgl_roller_type) },
    { MP_ROM_QSTR(MP_QSTR_Spinner), MP_ROM_PTR(&rm690b0_lvgl_spinner_type) },
    { MP_ROM_QSTR(MP_QSTR_Container), MP_ROM_PTR(&rm690b0_lvgl_container_type) },
    { MP_ROM_QSTR(MP_QSTR_Msgbox), MP_ROM_PTR(&rm690b0_lvgl_msgbox_type) },
    { MP_ROM_QSTR(MP_QSTR_List), MP_ROM_PTR(&rm690b0_lvgl_list_type) },
    { MP_ROM_QSTR(MP_QSTR_Spinbox), MP_ROM_PTR(&rm690b0_lvgl_spinbox_type) },
    { MP_ROM_QSTR(MP_QSTR_Tabview), MP_ROM_PTR(&rm690b0_lvgl_tabview_type) },
    { MP_ROM_QSTR(MP_QSTR_Table), MP_ROM_PTR(&rm690b0_lvgl_table_type) },
    { MP_ROM_QSTR(MP_QSTR_Buttonmatrix), MP_ROM_PTR(&rm690b0_lvgl_buttonmatrix_type) },
    { MP_ROM_QSTR(MP_QSTR_Textarea), MP_ROM_PTR(&rm690b0_lvgl_textarea_type) },
    { MP_ROM_QSTR(MP_QSTR_Keyboard), MP_ROM_PTR(&rm690b0_lvgl_keyboard_type) },
    { MP_ROM_QSTR(MP_QSTR_Chart), MP_ROM_PTR(&rm690b0_lvgl_chart_type) },
    { MP_ROM_QSTR(MP_QSTR_ChartSeries), MP_ROM_PTR(&rm690b0_lvgl_chart_series_type) },
    { MP_ROM_QSTR(MP_QSTR_Canvas), MP_ROM_PTR(&rm690b0_lvgl_canvas_type) },
    { MP_ROM_QSTR(MP_QSTR_Line), MP_ROM_PTR(&rm690b0_lvgl_line_type) },
    { MP_ROM_QSTR(MP_QSTR_Scale), MP_ROM_PTR(&rm690b0_lvgl_scale_type) },
    { MP_ROM_QSTR(MP_QSTR_Font), MP_ROM_PTR(&rm690b0_lvgl_font_type) },

    // Direction Constants
    { MP_ROM_QSTR(MP_QSTR_DIR_NONE), MP_ROM_INT(0x00) },
    { MP_ROM_QSTR(MP_QSTR_DIR_LEFT), MP_ROM_INT(0x01) },
    { MP_ROM_QSTR(MP_QSTR_DIR_RIGHT), MP_ROM_INT(0x02) },
    { MP_ROM_QSTR(MP_QSTR_DIR_TOP), MP_ROM_INT(0x04) },
    { MP_ROM_QSTR(MP_QSTR_DIR_BOTTOM), MP_ROM_INT(0x08) },
    { MP_ROM_QSTR(MP_QSTR_DIR_HOR), MP_ROM_INT(0x03) },
    { MP_ROM_QSTR(MP_QSTR_DIR_VER), MP_ROM_INT(0x0C) },
    { MP_ROM_QSTR(MP_QSTR_DIR_ALL), MP_ROM_INT(0x0F) },

    // Flex Flow Constants
    { MP_ROM_QSTR(MP_QSTR_FLEX_FLOW_ROW), MP_ROM_INT(0x00) },
    { MP_ROM_QSTR(MP_QSTR_FLEX_FLOW_COLUMN), MP_ROM_INT(0x01) },
    { MP_ROM_QSTR(MP_QSTR_FLEX_FLOW_ROW_WRAP), MP_ROM_INT(0x00 | 0x04) },
    { MP_ROM_QSTR(MP_QSTR_FLEX_FLOW_ROW_REVERSE), MP_ROM_INT(0x00 | 0x08) },
    { MP_ROM_QSTR(MP_QSTR_FLEX_FLOW_ROW_WRAP_REVERSE), MP_ROM_INT(0x00 | 0x04 | 0x08) },
    { MP_ROM_QSTR(MP_QSTR_FLEX_FLOW_COLUMN_WRAP), MP_ROM_INT(0x01 | 0x04) },
    { MP_ROM_QSTR(MP_QSTR_FLEX_FLOW_COLUMN_REVERSE), MP_ROM_INT(0x01 | 0x08) },
    { MP_ROM_QSTR(MP_QSTR_FLEX_FLOW_COLUMN_WRAP_REVERSE), MP_ROM_INT(0x01 | 0x04 | 0x08) },

    // Flex Align Constants
    { MP_ROM_QSTR(MP_QSTR_FLEX_ALIGN_START), MP_ROM_INT(0) },
    { MP_ROM_QSTR(MP_QSTR_FLEX_ALIGN_END), MP_ROM_INT(1) },
    { MP_ROM_QSTR(MP_QSTR_FLEX_ALIGN_CENTER), MP_ROM_INT(2) },
    { MP_ROM_QSTR(MP_QSTR_FLEX_ALIGN_SPACE_EVENLY), MP_ROM_INT(3) },
    { MP_ROM_QSTR(MP_QSTR_FLEX_ALIGN_SPACE_AROUND), MP_ROM_INT(4) },
    { MP_ROM_QSTR(MP_QSTR_FLEX_ALIGN_SPACE_BETWEEN), MP_ROM_INT(5) },

    // Keyboard mode constants
    { MP_ROM_QSTR(MP_QSTR_KBD_MODE_TEXT_LOWER), MP_ROM_INT(0) },
    { MP_ROM_QSTR(MP_QSTR_KBD_MODE_TEXT_UPPER), MP_ROM_INT(1) },
    { MP_ROM_QSTR(MP_QSTR_KBD_MODE_SPECIAL), MP_ROM_INT(2) },
    { MP_ROM_QSTR(MP_QSTR_KBD_MODE_NUMBER), MP_ROM_INT(3) },
    { MP_ROM_QSTR(MP_QSTR_KBD_MODE_USER_1), MP_ROM_INT(4) },
    { MP_ROM_QSTR(MP_QSTR_KBD_MODE_USER_2), MP_ROM_INT(5) },
    { MP_ROM_QSTR(MP_QSTR_KBD_MODE_USER_3), MP_ROM_INT(6) },
    { MP_ROM_QSTR(MP_QSTR_KBD_MODE_USER_4), MP_ROM_INT(7) },

    // Chart constants
    { MP_ROM_QSTR(MP_QSTR_CHART_TYPE_NONE), MP_ROM_INT(LV_CHART_TYPE_NONE) },
    { MP_ROM_QSTR(MP_QSTR_CHART_TYPE_LINE), MP_ROM_INT(LV_CHART_TYPE_LINE) },
    { MP_ROM_QSTR(MP_QSTR_CHART_TYPE_BAR), MP_ROM_INT(LV_CHART_TYPE_BAR) },
    { MP_ROM_QSTR(MP_QSTR_CHART_TYPE_SCATTER), MP_ROM_INT(LV_CHART_TYPE_SCATTER) },
    { MP_ROM_QSTR(MP_QSTR_CHART_AXIS_PRIMARY_Y), MP_ROM_INT(LV_CHART_AXIS_PRIMARY_Y) },
    { MP_ROM_QSTR(MP_QSTR_CHART_AXIS_SECONDARY_Y), MP_ROM_INT(LV_CHART_AXIS_SECONDARY_Y) },
    { MP_ROM_QSTR(MP_QSTR_CHART_AXIS_PRIMARY_X), MP_ROM_INT(LV_CHART_AXIS_PRIMARY_X) },
    { MP_ROM_QSTR(MP_QSTR_CHART_AXIS_SECONDARY_X), MP_ROM_INT(LV_CHART_AXIS_SECONDARY_X) },

    // Canvas color formats
    { MP_ROM_QSTR(MP_QSTR_IMG_CF_TRUE_COLOR), MP_ROM_INT(LV_IMG_CF_TRUE_COLOR) },
    { MP_ROM_QSTR(MP_QSTR_IMG_CF_TRUE_COLOR_ALPHA), MP_ROM_INT(LV_IMG_CF_TRUE_COLOR_ALPHA) },

};

static MP_DEFINE_CONST_DICT(rm690b0_lvgl_module_globals, rm690b0_lvgl_module_globals_table);

const mp_obj_module_t rm690b0_lvgl_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&rm690b0_lvgl_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_rm690b0_lvgl, rm690b0_lvgl_module);
