
#pragma once

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "stddef.h"
#include "esp_log.h"

typedef struct {
	uint8_t b[9];
} rx_t;

/**
 * @brief set gpio for the 1 wire bus
 *
 * @param[in] gpio number
 *
 * @return
 * 	- ESP_OK
 * 	- ESP_ERR_INVALID_ARG
 */
esp_err_t set_gpio (uint8_t);

/**
 * @brief get temperature in °C x 10 for all ds18b20 devices in the bus
 *
 * @param[in] array of N ds18b20 rom addresses
 *
 * @param[out] array of N temperatures in °C x 10. -800 means not sensor response and -900 means a CRC check fails. 
 *
 * @return
 * 	- ESP_OK
 * 	- ESP_ERR_NOT_FOUND
 */
esp_err_t get_temperature (const uint64_t *, size_t, int16_t *);

/**
 * @brief discover rom address
 *
 * @param[in] gpio number
 *
 * @param[out] ds18b20 rom addresses
 *
 * @return
 * 	- ESP_OK
 * 	- ESP_ERR_INVALID_ARG
 *  - ESP_ERR_NOT_FOUND
*/
esp_err_t get_rom(uint8_t, const uint64_t *);