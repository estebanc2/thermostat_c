#pragma once
#include "one_wire.h"
#include "yogurtera.h"


// ROM commands
#define READ_ROM 0x33
#define MATCH_ROM 0x55
#define SEARCH_ROM 0xF0
#define ALARM_SEARCH 0xEC
#define SKIP_ROM 0xCC
//Function commands
#define CONVERT_T 0x44
#define READ_SCRATCH 0xBE
#define WRITE_SCRATCH 0x4E
#define COPY_SCRATCH 0x48
#define READ_E2 0xB8
#define READ_PWR 0xB4
//config. resolution
#define RES_9  0x1F
#define RES_10 0x3F
#define RES_11 0x5F
#define RES_12 0x7F 

/*
* MASTER TX
*  _____
        |        _______________________ 
        |________|
*          t0                   t1
*
*/

/*
* 9 bytes from scratchpad means:
* -----------------
* |byte|Scratchpad|ROMcode
* |  0 | temp LSB |CRC
* |  1 | temp MSB |code MSB
* |  2 | TH       |code 
* |  3 | TL       |code 
* |  4 | config   |code 
* |  5 | reserv 1 |code 
* |  6 | reserv 2 |code LSB
* |  7 | reserv 3 |Family code
* |  8 | CRC      |
*/

/*
  * RECONTRAGUARDA CON ESTO!!
  * @brief  CPU do while loop for some time.
  *         In FreeRTOS task, please call FreeRTOS apis.
  *
  * @param  uint32_t us : Delay time in us.
  *
  * @return None
  *
 void ets_delay_us(uint32_t us);
 */


esp_err_t ds18b20_read(const uint8_t *, int16_t *);