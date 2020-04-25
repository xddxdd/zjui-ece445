#include "main.h"
#include "run.h"

extern UART_HandleTypeDef huart2;
#define UART_PMS5003 huart2

void loop_pms5003() {
    uint8_t buf[32];
    int ret;

    // Timeout = 100ms, so cannot get mixed data of 2 transmissions
    if(HAL_OK != (ret = HAL_UART_Receive(&UART_PMS5003, buf, 32, 100))) {
        if(HAL_TIMEOUT == ret) {
            // Previous attempt timeout, so we got half data from last transmission
            // Now we can wait longer for the next transmission
            if(HAL_OK != HAL_UART_Receive(&UART_PMS5003, buf, 32, 1000)) return;
        } else {
            return;
        }
    }
    // Verify packet header and frame length (fixed)
    if(buf[0] != 0x42 || buf[1] != 0x4d || buf[2] != 0 || buf[3] != 28) return;

    // Verify packet sum
    uint16_t sum = 0;
    for(int i = 0; i < 30; i++) sum += buf[i];
    uint16_t sum_expected = (((uint32_t) buf[30]) << 8) | buf[31];
    if(sum != sum_expected) return;


    measure_value.pms5003.pm1 = (((uint32_t) buf[4]) << 8) | buf[5];
    measure_value.pms5003.pm2_5 = (((uint32_t) buf[6]) << 8) | buf[7];
    measure_value.pms5003.pm10 = (((uint32_t) buf[8]) << 8) | buf[9];
}
