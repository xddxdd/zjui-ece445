#include "run.h"
#include "sensors/run_bme680.h"
#include "main.h"

extern ADC_HandleTypeDef hadc1;
extern CRC_HandleTypeDef hcrc;
extern I2C_HandleTypeDef hi2c1;
extern RTC_HandleTypeDef hrtc;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

void float_to_string(float number, char* buf);

void setup() {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    int8_t ret;
    if(0 != (ret = bme680_create_structure())) {
        char buf[] = "ERR INIT _\r\n";
        const char map[] = "0123456789ABCDEF";

        if(ret < 0) ret = -ret;
        buf[9] = map[ret];

        HAL_UART_Transmit(&huart1, (uint8_t*) buf, 12, HAL_MAX_DELAY);
    }
}

void loop() {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_Delay(1000);

    int32_t bme680_delay_ms = bme680_perform_measurement();
    if(-1 == bme680_delay_ms) {
        HAL_UART_Transmit(&huart1, (uint8_t*) "ERR RQ\r\n", 8, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
        HAL_Delay(1000);
        return;
    }

    struct bme680_field_data data;
    if(-1 == bme680_get_measurements(&data)) {
        HAL_UART_Transmit(&huart1, (uint8_t*) "ERR RD\r\n", 8, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
        HAL_Delay(1000);
        return;
    }
    
    char buf[] = "_=________.__\r\n";

    buf[0] = 'T';
    float_to_string(data.temperature, &buf[2]);
    HAL_UART_Transmit(&huart1, (uint8_t*) buf, sizeof(buf), HAL_MAX_DELAY);
    
    buf[0] = 'P';
    float_to_string(data.pressure, &buf[2]);
    HAL_UART_Transmit(&huart1, (uint8_t*) buf, sizeof(buf), HAL_MAX_DELAY);


    buf[0] = 'P';
    float_to_string(data.humidity, &buf[2]);
    HAL_UART_Transmit(&huart1, (uint8_t*) buf, sizeof(buf), HAL_MAX_DELAY);

    buf[0] = 'P';
    float_to_string(data.gas_resistance, &buf[2]);
    HAL_UART_Transmit(&huart1, (uint8_t*) buf, sizeof(buf), HAL_MAX_DELAY);

    HAL_UART_Transmit(&huart1, (uint8_t*) "\r\n", 2, HAL_MAX_DELAY);
}

void float_to_string(float number, char* buf) {
    const char map[] = "0123456789ABCDEF";
    buf[0] = map[((int) (number / 10000000)) % 10];
    buf[1] = map[((int) (number / 1000000)) % 10];
    buf[2] = map[((int) (number / 100000)) % 10];
    buf[3] = map[((int) (number / 10000)) % 10];
    buf[4] = map[((int) (number / 1000)) % 10];
    buf[5] = map[((int) (number / 100)) % 10];
    buf[6] = map[((int) (number / 10)) % 10];
    buf[7] = map[((int) (number)) % 10];
    buf[8] = '.';
    buf[9] = map[((int) (number * 10)) % 10];
    buf[10] = map[((int) (number * 100)) % 10];
}