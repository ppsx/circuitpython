// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2024 Waveshare Electronics
//
// SPDX-License-Identifier: MIT

#pragma once

#define MICROPY_HW_BOARD_NAME "Waveshare ESP32-S3-Touch-AMOLED-2.41"
#define MICROPY_HW_MCU_NAME   "ESP32S3"

// USB identifiers
#define USB_VID 0x303A
#define USB_PID 0x82CE
#define USB_MANUFACTURER "Waveshare"
#define USB_PRODUCT "ESP32-S3-Touch-AMOLED-2.41"

// I2C bus
#define CIRCUITPY_BOARD_I2C         (1)
#define CIRCUITPY_BOARD_I2C_PIN     {{.scl = &pin_GPIO48, .sda = &pin_GPIO47}}

// SPI bus for SD Card
#define CIRCUITPY_BOARD_SPI         (1)
#define CIRCUITPY_BOARD_SPI_PIN     {{.clock = &pin_GPIO4, .mosi = &pin_GPIO5, .miso = &pin_GPIO6}}

// Default UART bus
#define DEFAULT_UART_BUS_RX (&pin_GPIO44)
#define DEFAULT_UART_BUS_TX (&pin_GPIO43)

// Disable unnecessary modules to save space
#define CIRCUITPY_ESP32_CAMERA (0)
