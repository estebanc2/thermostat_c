#include "one_wire.h"


#define TT0 60
#define TT1 15
#define TR0 2
#define TR1 8
#define TR_DELAY 55

#define MASTER_RESET_PULSE_DURATION 480 // Reset time high. Reset time low.
#define RESPONSE_MAX_DURATION 60        // Presence detect high.
#define PRESENCE_PULSE_MAX_DURATION 240 // Presence detect low.
#define RECOVERY_DURATION 1             // Bus recovery time.
#define TIME_SLOT_START_DURATION 1      // Time slot start.
#define TIME_SLOT_DURATION 80           // Time slot.
#define VALID_DATA_DURATION 15          // Valid data duration.

esp_err_t initialize (){
    gpio_set_direction(ONE_WIRE_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(ONE_WIRE_GPIO, 0);
    os_delay_us(MASTER_RESET_PULSE_DURATION);
    gpio_set_level(ONE_WIRE_GPIO, 1);
    gpio_set_direction(ONE_WIRE_GPIO, GPIO_MODE_INPUT);
    os_delay_us(RECOVERY_DURATION);
    uint8_t response_time = 0;
    while (gpio_get_level(ONE_WIRE_GPIO) == 1)
    {
        if (response_time > RESPONSE_MAX_DURATION)
        {
            return ESP_ERR_TIMEOUT;
        }
        ++response_time;
        os_delay_us(1);
    }
    uint8_t presence_time = 0;
    while (gpio_get_level(ONE_WIRE_GPIO) == 0)
    {
        if (presence_time > PRESENCE_PULSE_MAX_DURATION)
        {
            return ESP_ERR_TIMEOUT;
        }
        ++presence_time;
        os_delay_us(1);
    }
    os_delay_us(MASTER_RESET_PULSE_DURATION - response_time - presence_time);
    return ESP_OK;
}

static uint8_t _read_bit(void)
{
    gpio_set_direction(ONE_WIRE_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(ONE_WIRE_GPIO, 0);
    os_delay_us(TIME_SLOT_START_DURATION);
    //gpio_set_level(ONE_WIRE_GPIO, 1);
    gpio_set_direction(ONE_WIRE_GPIO, GPIO_MODE_INPUT);
    os_delay_us(VALID_DATA_DURATION - TIME_SLOT_START_DURATION);
    uint8_t bit = gpio_get_level(ONE_WIRE_GPIO);
    os_delay_us(TIME_SLOT_DURATION - RECOVERY_DURATION - VALID_DATA_DURATION);
    return bit;
}

static void _send_bit(const uint8_t bit)
{
    gpio_set_direction(ONE_WIRE_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(ONE_WIRE_GPIO, 0);
    if (bit == 0)
    {
        os_delay_us(58);//TIME_SLOT_DURATION);
        gpio_set_level(ONE_WIRE_GPIO, 1);
        gpio_set_direction(ONE_WIRE_GPIO, GPIO_MODE_INPUT);
        os_delay_us(RECOVERY_DURATION);
    }
    else
    {
        os_delay_us(TIME_SLOT_START_DURATION);
        gpio_set_level(ONE_WIRE_GPIO, 1);
        gpio_set_direction(ONE_WIRE_GPIO, GPIO_MODE_INPUT);
        os_delay_us(TIME_SLOT_DURATION);
    }
}

void send_1_byte (uint8_t byte){
    for (uint8_t i = 0; i < 8; ++i){
        _send_bit(byte & 1);
        byte >>= 1;
    }
}

uint8_t read_1_byte (void){
    uint8_t byte = 0;
    for (uint8_t i = 0; i < 8; ++i)
    {
        byte >>= 1;
        if (_read_bit() != 0)
        {
            byte |= 0x80;
        }
    }
    return byte;
}
