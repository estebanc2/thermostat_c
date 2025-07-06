#include "ds18b20.h"
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

static esp_err_t crc_check (rx_t rx){
	rx_t rx_to_test = rx;
    // Calculamos el CRC de los primeros 7 bytes
    uint8_t calculated_crc = compute_crc8((const uint8_t *)&rx_to_test, 8);
    //printf("calc: %d, recei: %d\n", calculated_crc, rx.b[8]);
    if (calculated_crc == rx.b[8]){
        return ESP_OK;
    }else{
        return ESP_ERR_INVALID_CRC;
    }
}

esp_err_t ds18b20_read(const uint8_t *device, int16_t *temp_x_10){
    rx_t rx = {0};
    if (initialize() != ESP_OK){
        *temp_x_10 = -990;
        return ESP_ERR_INVALID_STATE; 
    }
    if (device == NULL){
        send_1_byte(SKIP_ROM);
    } else{
        send_1_byte(MATCH_ROM);
        for (uint8_t i = 0; i < 8; ++i){
            send_1_byte(*(device++));
        }
    }
    send_1_byte(CONVERT_T);
    vTaskDelay(750 / portTICK_PERIOD_MS);
    if (initialize() != ESP_OK){
        *temp_x_10 = -980;
        return ESP_ERR_INVALID_STATE; 
    }
    if (device == NULL){
        send_1_byte(SKIP_ROM);
    } else{
        send_1_byte(MATCH_ROM);
        for (uint8_t i = 0; i < 8; ++i){
            send_1_byte(*(device++));
        }
    }
    send_1_byte(READ_SCRATCH);
    for (uint8_t i = 0; i < 9; ++i)
    {
        rx.b[i] = read_1_byte();
    }
    if (crc_check(rx) != ESP_OK){
        *temp_x_10 = -970;
        return ESP_ERR_INVALID_CRC; 
    }
    *temp_x_10 = (10 * (rx.b[1] << 8) | rx.b[0]) / 16;
    return ESP_OK;
}

