#include "ds18b20.h"

// ROM commands
#define MATCH_ROM 0x55
#define SKIP_ROM 0xCC
#define READ_ROM 0x33
//Function commands
#define CONVERT_T 0x44
#define READ_SCRATCH 0xBE
//1 wire times
#define MASTER_RESET_PULSE_DURATION 480 // Reset time high. Reset time low.
#define RESPONSE_MAX_DURATION 60        // Presence detect high.
#define PRESENCE_PULSE_MAX_DURATION 240 // Presence detect low.
#define RECOVERY_DURATION 1             // Bus recovery time.
#define TIME_SLOT_START_DURATION 1      // Time slot start.
#define TIME_SLOT_DURATION 80           // Time slot.
#define VALID_DATA_DURATION 15          // Valid data duration.

#ifdef CONFIG_IDF_TARGET_ESP8266
#define esp_delay_us(x) os_delay_us(x)
#else
#define esp_delay_us(x) esp_rom_delay_us(x)
#endif

static uint8_t pin;

esp_err_t set_gpio(uint8_t gpio)
{
    pin = gpio;
    gpio_config_t config;
    config.intr_type = GPIO_INTR_DISABLE;
    config.mode = GPIO_MODE_INPUT;
    config.pin_bit_mask = (1ULL << pin);
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.pull_up_en = GPIO_PULLUP_ENABLE;
    if (gpio_config(&config) != ESP_OK)
    {
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}

static esp_err_t initialize (){
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);
    esp_delay_us(MASTER_RESET_PULSE_DURATION);
    gpio_set_direction(pin, GPIO_MODE_INPUT);
    esp_delay_us(RECOVERY_DURATION);
    uint8_t response_time = 0;
    while (gpio_get_level(pin) == 1)
    {
        if (response_time > RESPONSE_MAX_DURATION)
        {
            return ESP_ERR_TIMEOUT;
        }
        ++response_time;
        esp_delay_us(1);
    }
    uint8_t presence_time = 0;
    while (gpio_get_level(pin) == 0)
    {
        if (presence_time > PRESENCE_PULSE_MAX_DURATION)
        {
            return ESP_ERR_TIMEOUT;
        }
        ++presence_time;
        esp_delay_us(1);
    }
    esp_delay_us(MASTER_RESET_PULSE_DURATION - response_time - presence_time);
    return ESP_OK;
}

static uint8_t read_1_bit(void)
{
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);
    esp_delay_us(TIME_SLOT_START_DURATION);
    gpio_set_direction(pin, GPIO_MODE_INPUT);
    esp_delay_us(VALID_DATA_DURATION - TIME_SLOT_START_DURATION);
    uint8_t bit = gpio_get_level(pin);
    esp_delay_us(TIME_SLOT_DURATION - RECOVERY_DURATION - VALID_DATA_DURATION);
    return bit;
}

static void send_1_bit(const uint8_t bit)
{
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);
    if (bit == 0)
    {
        esp_delay_us(58);//TIME_SLOT_DURATION);
        gpio_set_direction(pin, GPIO_MODE_INPUT);
        esp_delay_us(RECOVERY_DURATION);
    }
    else
    {
        esp_delay_us(TIME_SLOT_START_DURATION);
        gpio_set_direction(pin, GPIO_MODE_INPUT);
        esp_delay_us(TIME_SLOT_DURATION);
    }
}

static void send_1_byte (uint8_t byte){
    for (uint8_t i = 0; i < 8; ++i){
        send_1_bit(byte & 1);
        byte >>= 1;
    }
}

static uint8_t read_1_byte (void){
    uint8_t byte = 0;
    for (uint8_t i = 0; i < 8; ++i)
    {
        byte >>= 1;
        if (read_1_bit() != 0)
        {
            byte |= 0x80;
        }
    }
    return byte;
}

static uint8_t compute_crc8(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0;
    uint8_t i, j;
    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x01) {
                crc = (crc >> 1) ^ 0x8C;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

static esp_err_t crc_check (uint8_t *data, size_t len){
    uint8_t calculated_crc = compute_crc8((const uint8_t *)data, len-1);
    if (calculated_crc == data[len-1]){
        return ESP_OK;
    }else{
        return ESP_ERR_INVALID_CRC;
    }
}

esp_err_t get_temperature(const uint64_t *sonda, size_t sondas, int16_t *temp){
    if (initialize() != ESP_OK){
        return ESP_ERR_NOT_FOUND;
    }
    send_1_byte(SKIP_ROM);
    send_1_byte(CONVERT_T);
    vTaskDelay(750 / portTICK_PERIOD_MS);
    for (size_t i = 0; i < sondas; i++) {
        if (initialize() != ESP_OK){
            if (temp) temp[i] = -800; //ESP_ERR_NOT_FOUND; 
        } else {
            if (sonda[0] != 0) { 
                send_1_byte(MATCH_ROM);
                uint64_t pos = 0;
                for (uint8_t k = 0; k < 64; k++){
                     pos = (uint64_t)1 << k;
                    if ((sonda[i] & pos) == 0){
                        send_1_bit(0);
                    } else {
                        send_1_bit(1);
                        }
                }
            } else {
                send_1_byte(SKIP_ROM);
            }
            send_1_byte(READ_SCRATCH);
            rx_t rx = {0};
            for (uint8_t j = 0; j < 9; ++j)
            {
                rx.b[j] = read_1_byte();
            }
            if (crc_check(rx.b, 9) != ESP_OK){
                if (temp) temp[i] = -900;//ESP_ERR_INVALID_CRC
            } else{
                if (temp) temp[i] = (10 * (int16_t)((rx.b[1] << 8) | rx.b[0]))/ 16;
            }
        }
    }
    return ESP_OK;
}

esp_err_t get_rom(uint8_t pin, const uint64_t * rom){
    if (set_gpio(pin) != ESP_OK){
        return ESP_ERR_INVALID_ARG;
    }
    if (initialize() != ESP_OK){
        return ESP_ERR_NOT_FOUND; 
    }
    send_1_byte(READ_ROM);
    uint8_t rx[8] = {0};
    for (uint8_t i = 0; i < 8; i++){
        rx[i] = read_1_byte();
    }
    if (crc_check(rx, 8) != ESP_OK){
        return ESP_ERR_INVALID_CRC; 
    }
    uint64_t code = 0;
    for (int i = 0; i < 8; i++) {
        code |= ((uint64_t)rx[i]) << (8 * i);
    }
    if (rom) {
        *((uint64_t*)rom) = code;
    }
    return ESP_OK;
}