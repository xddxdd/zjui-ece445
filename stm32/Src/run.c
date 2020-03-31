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

uint32_t adc_dma[5];
volatile uint32_t adc_dma_finished;

#define PRINT_DEVICE huart1
const char map[] = "0123456789ABCDEF";

void print(char* s);
void println(char* s);
void print_float(char* name, float value);
void print_int(char* name, int value);

void setup() {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    int8_t ret;
    if(0 != (ret = bme680_create_structure())) {
        char buf[] = "ERR INIT _";

        if(ret < 0) ret = -ret;
        buf[9] = map[ret];

        println(buf);
    }
}

void loop() {
    HAL_Delay(1000);

    adc_dma_finished = 0;
    HAL_ADC_Start_DMA(&hadc1, adc_dma, 5);
    while(!adc_dma_finished);
    HAL_Delay(100);
    HAL_ADC_Stop_DMA(&hadc1);

    print_int("CO", adc_dma[0]);
    print_int("NH3", adc_dma[1]);
    print_int("NO2", adc_dma[2]);
    print_int("Temp", adc_dma[3]);
    print_int("Vrefint", adc_dma[4]);
}

// void loop() {
//     HAL_Delay(1000);

//     int32_t bme680_delay_ms = bme680_perform_measurement();
//     if(-1 == bme680_delay_ms) {
//         println("bme680: error requesting");
//         return;
//     }

//     struct bme680_field_data data;
//     if(-1 == bme680_get_measurements(&data)) {
//         println("bme680: error reading");
//         return;
//     }

//     print_float("Tmp", data.temperature);
//     print_float("Prs", data.pressure);
//     print_float("Hum", data.humidity);
//     print_float("Gas", data.gas_resistance);

//     HAL_UART_Transmit(&PRINT_DEVICE, (uint8_t*) "\r\n", 2, HAL_MAX_DELAY);
// }

/*****************************************
 * Common functions
 *****************************************/

void print(char* s) {
    int32_t len = 0;
    while(s[len++]);
    HAL_UART_Transmit(&PRINT_DEVICE, (uint8_t*) s, len, HAL_MAX_DELAY);
}

void println(char* s) {
    print(s);
    print("\r\n");
}

void print_int(char* name, int value) {
    char buf[256];

    int32_t len = 0;
    while(name[len]) {
        buf[len] = name[len];
        len++;
    }
    if(len > 256 - 16) len = 256 - 16;
    buf[len++] = ' ';
    buf[len++] = '=';
    buf[len++] = ' ';

    buf[len++] = map[((int) (value / 10000000)) % 10];
    buf[len++] = map[((int) (value / 1000000)) % 10];
    buf[len++] = map[((int) (value / 100000)) % 10];
    buf[len++] = map[((int) (value / 10000)) % 10];
    buf[len++] = map[((int) (value / 1000)) % 10];
    buf[len++] = map[((int) (value / 100)) % 10];
    buf[len++] = map[((int) (value / 10)) % 10];
    buf[len++] = map[((int) (value)) % 10];
    buf[len++] = '.';
    buf[len++] = map[((int) (value * 10)) % 10];
    buf[len++] = map[((int) (value * 100)) % 10];

    buf[len++] = '\r';
    buf[len++] = '\n';

    HAL_UART_Transmit(&PRINT_DEVICE, (uint8_t*) buf, len, HAL_MAX_DELAY);
}

void print_float(char* name, float value) {
    char buf[256];

    int32_t len = 0;
    while(name[len]) {
        buf[len] = name[len];
        len++;
    }
    if(len > 256 - 16) len = 256 - 16;
    buf[len++] = ' ';
    buf[len++] = '=';
    buf[len++] = ' ';

    buf[len++] = map[((int) (value / 10000000)) % 10];
    buf[len++] = map[((int) (value / 1000000)) % 10];
    buf[len++] = map[((int) (value / 100000)) % 10];
    buf[len++] = map[((int) (value / 10000)) % 10];
    buf[len++] = map[((int) (value / 1000)) % 10];
    buf[len++] = map[((int) (value / 100)) % 10];
    buf[len++] = map[((int) (value / 10)) % 10];
    buf[len++] = map[((int) (value)) % 10];
    buf[len++] = '.';
    buf[len++] = map[((int) (value * 10)) % 10];
    buf[len++] = map[((int) (value * 100)) % 10];

    buf[len++] = '\r';
    buf[len++] = '\n';

    HAL_UART_Transmit(&PRINT_DEVICE, (uint8_t*) buf, len, HAL_MAX_DELAY);
}
