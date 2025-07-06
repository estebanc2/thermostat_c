#pragma once
#include "driver/gpio.h"
#include "esp_err.h"
#include "stddef.h"
#include <stdlib.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ONE_WIRE_GPIO 12

typedef struct {
	uint8_t b[9];
} rx_t;

void send_1_byte (uint8_t);
uint8_t read_1_byte (void);
esp_err_t initialize (void);