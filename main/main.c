/*
    SWITCH_CONTROL_C - An app to manage electric switches from WiFi

    Copyright (C) 2023  Esteban Castro  ecastro@miratucuadra.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "esp_log.h"
#include "ds18b20.h"

// GPIO assigment
#define SWITCH_OUTPUT 4
#define LED_OUTPUT 2

#define UNUSED_1_OUTPUT 5
#define UNUSED_2_OUTPUT 13
#define UNUSED_3_OUTPUT 14
#define UNUSED_4_OUTPUT 16
#define OUTPUT_PIN_SELECT ((1ULL << SWITCH_OUTPUT) | (1ULL << LED_OUTPUT) | (1ULL << UNUSED_1_OUTPUT) | (1ULL << UNUSED_2_OUTPUT) | (1ULL << UNUSED_3_OUTPUT) | (1ULL << UNUSED_4_OUTPUT))
//histeresis
#define TEMP_LOW_X_10 390
#define TEMP_HIGHT_X_10 410

static const char *TAG = "MAIN";

static void temp_check_task(void *arg){
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;    // disable interrupt
    io_conf.mode = GPIO_MODE_OUTPUT;          // set as output mode
    io_conf.pin_bit_mask = OUTPUT_PIN_SELECT; // bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;                 // disable pull-down mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;                   // disable pull-up mode
    gpio_config(&io_conf); 
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << ONE_WIRE_GPIO);
    gpio_config(&io_conf); 
    while (1){
        int16_t temp_x_10; // podria pedir: ds18b20_temp _x_10
        ds18b20_read( NULL, &temp_x_10 );
        ESP_LOGI(TAG,"temperatura = %d", temp_x_10);
        if(temp_x_10 < TEMP_LOW_X_10){
            gpio_set_level(SWITCH_OUTPUT, 1); // se prende
            gpio_set_level(LED_OUTPUT, 0); // se prende
        }
        else if (temp_x_10 > TEMP_HIGHT_X_10){
            gpio_set_level(SWITCH_OUTPUT, 0); //se apaga
            gpio_set_level(LED_OUTPUT, 1); // se apaga
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
        gpio_set_level(LED_OUTPUT, 0); // se prende
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
void app_main()
{
    xTaskCreate(temp_check_task, "temp_check_task", 2048, NULL, 20, NULL); 
}
