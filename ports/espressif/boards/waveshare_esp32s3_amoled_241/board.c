// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2024 Waveshare Electronics
//
// SPDX-License-Identifier: MIT

#include "supervisor/board.h"
#include "mpconfigboard.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "hal/gpio_ll.h"
#include "rom/ets_sys.h" // For ets_delay_us

#define POWER_PIN           (GPIO_NUM_16)
#define I2C_SDA_PIN         (GPIO_NUM_47)
#define I2C_SCL_PIN         (GPIO_NUM_48)

#define RTC_ADDR            (0x51)
#define IMU_ADDR            (0x6B)

static const char *TAG = "board";

void board_i2c_init(void);

// --- Low-level I2C bit-bang implementation ---
// This is used for one-time setup before CircuitPython's busio takes over.

static void i2c_delay(void) {
    // A short delay for 400kHz I2C
    ets_delay_us(2);
}

static void i2c_start(void) {
    gpio_set_direction(I2C_SDA_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(I2C_SDA_PIN, 1);
    gpio_set_level(I2C_SCL_PIN, 1);
    i2c_delay();
    gpio_set_level(I2C_SDA_PIN, 0);
    i2c_delay();
    gpio_set_level(I2C_SCL_PIN, 0);
    i2c_delay();
}

static void i2c_stop(void) {
    gpio_set_direction(I2C_SDA_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(I2C_SDA_PIN, 0);
    gpio_set_level(I2C_SCL_PIN, 1);
    i2c_delay();
    gpio_set_level(I2C_SDA_PIN, 1);
    i2c_delay();
}

static bool i2c_write_byte(uint8_t byte) {
    gpio_set_direction(I2C_SDA_PIN, GPIO_MODE_OUTPUT);
    for (int i = 0; i < 8; i++) {
        gpio_set_level(I2C_SDA_PIN, (byte & 0x80) ? 1 : 0);
        byte <<= 1;
        i2c_delay();
        gpio_set_level(I2C_SCL_PIN, 1);
        i2c_delay();
        gpio_set_level(I2C_SCL_PIN, 0);
    }
    // Get ACK
    gpio_set_direction(I2C_SDA_PIN, GPIO_MODE_INPUT);
    i2c_delay();
    gpio_set_level(I2C_SCL_PIN, 1);
    i2c_delay();
    bool ack = (gpio_get_level(I2C_SDA_PIN) == 0);
    gpio_set_level(I2C_SCL_PIN, 0);
    return ack;
}

static void i2c_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, size_t len) {
    i2c_start();
    if (!i2c_write_byte(dev_addr << 1)) {
        i2c_stop();
        ESP_LOGE(TAG, "I2C write failed: No ACK on addr 0x%02X", dev_addr);
        return;
    }
    if (!i2c_write_byte(reg_addr)) {
        i2c_stop();
        ESP_LOGE(TAG, "I2C write failed: No ACK on reg 0x%02X", reg_addr);
        return;
    }
    for (size_t i = 0; i < len; i++) {
        if (!i2c_write_byte(data[i])) {
            ESP_LOGE(TAG, "I2C write failed: No ACK on data byte %d", i);
            break; // Still issue a stop
        }
    }
    i2c_stop();
}

void board_i2c_init(void) {
    // 1. Configure I2C pins with internal pull-ups and set to open-drain
    gpio_config_t i2c_pin_config = {
        .pin_bit_mask = (1ULL << I2C_SDA_PIN) | (1ULL << I2C_SCL_PIN),
        .mode = GPIO_MODE_INPUT_OUTPUT_OD, // Open-drain
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&i2c_pin_config);

    // --- 2. Initialize RTC (PCF85063) ---
    ESP_LOGI(TAG, "Initializing RTC (PCF85063)...");
    uint8_t rtc_data[] = {0x00}; // Clear TEST1 and STOP bits
    i2c_write_reg(RTC_ADDR, 0x00, rtc_data, 1);

    // --- 3. Initialize IMU (QMI8658C) ---
    ESP_LOGI(TAG, "Initializing IMU (QMI8658C)...");
    uint8_t imu_data;

    // Reset
    imu_data = 0xB6; i2c_write_reg(IMU_ADDR, 0x60, &imu_data, 1);
    vTaskDelay(pdMS_TO_TICKS(20));

    // On-demand calibration
    imu_data = 0xB6; i2c_write_reg(IMU_ADDR, 0x60, &imu_data, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    imu_data = 0xA2; i2c_write_reg(IMU_ADDR, 0x0A, &imu_data, 1); // CTRL9
    vTaskDelay(pdMS_TO_TICKS(2200)); // Wait for calibration
    imu_data = 0x00; i2c_write_reg(IMU_ADDR, 0x0A, &imu_data, 1); // CTRL9 NOP

    // Configure control registers
    imu_data = 0x78; i2c_write_reg(IMU_ADDR, 0x02, &imu_data, 1); // CTRL1
    imu_data = 0x25; i2c_write_reg(IMU_ADDR, 0x03, &imu_data, 1); // CTRL2 (Accel)
    imu_data = 0x35; i2c_write_reg(IMU_ADDR, 0x04, &imu_data, 1); // CTRL3 (Gyro)
    imu_data = 0x00; i2c_write_reg(IMU_ADDR, 0x06, &imu_data, 1); // CTRL5
    imu_data = 0xC0; i2c_write_reg(IMU_ADDR, 0x09, &imu_data, 1); // CTRL8

    // Enable sensors
    imu_data = 0x03; i2c_write_reg(IMU_ADDR, 0x08, &imu_data, 1); // CTRL7
    vTaskDelay(pdMS_TO_TICKS(50));

    // 4. Revert pins to a neutral state for CircuitPython
    // Set to high-impedance input. busio will reconfigure them.
    gpio_set_direction(I2C_SDA_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(I2C_SCL_PIN, GPIO_MODE_INPUT);
}

void board_init(void) {
    // Enable power to the display and other peripherals
    gpio_config_t power_pin_config = {
        .pin_bit_mask = (1ULL << POWER_PIN),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&power_pin_config);
    gpio_set_level(POWER_PIN, 1);

    // Initialize I2C peripherals
    board_i2c_init();
}

// Use the MP_WEAK supervisor/shared/board.c versions of routines not defined here.
