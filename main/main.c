
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

static uint64_t sonda[] = {

};
static size_t sonda_size = sizeof(sonda) / sizeof(sonda[0]);

static void temp_check_task(void *arg){
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;    // disable interrupt
    io_conf.mode = GPIO_MODE_OUTPUT;          // set as output mode
    io_conf.pin_bit_mask = OUTPUT_PIN_SELECT; // bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;                 // disable pull-down mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;                   // disable pull-up mode
    gpio_config(&io_conf); 
    if (sonda_size == 0) sonda_size = 1;
    set_gpio(12);
    int16_t temp[sonda_size];
    while (1){
        if( get_temperature (sonda, sonda_size, temp) == ESP_OK){
            for (uint8_t i = 0; i<(sonda_size); i++) {
                ESP_LOGI(TAG,"current sensor %d temperature = %.1f", i + 1, (float)(temp[i])/10.0f);
            }
        } else {    
            ESP_LOGW(TAG,"no DS18B20 detected");
        }
        ESP_LOGI(TAG,"temperatura = %d", temp[0]);
        if(temp[0] < TEMP_LOW_X_10){
            gpio_set_level(SWITCH_OUTPUT, 1); // se prende
            gpio_set_level(LED_OUTPUT, 0); // se prende
        }
        else if (temp[0] > TEMP_HIGHT_X_10){
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
