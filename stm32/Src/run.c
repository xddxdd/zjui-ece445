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

struct measure_value {
    union {
        struct {
            uint8_t bme680_valid : 1;
            uint8_t mics6814_valid : 1;
            uint8_t reserved : 6;
        };
        uint8_t raw;
    } status;

    float bme680_temperature;
    float bme680_pressure;
    float bme680_humidity;
    float bme680_gas_resistance;

    float mics6814_co;
    float mics6814_nh3;
    float mics6814_no2;

    float stm32_temp;
    float stm32_vrefint;
} measure_value;

void print(char* s);
void println(char* s);
void print_float(char* name, float value);
void print_int(char* name, int value);
void loop_adc();
void loop_bme680();
void loop_print();
float voltage_to_resistor_value(uint32_t voltage, uint32_t resistor);

void setup() {
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

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

    measure_value.status.bme680_valid = 0;
    measure_value.status.mics6814_valid = 0;

    loop_adc();
    loop_bme680();
    loop_print();

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

#define RESISTOR_VALUE_CO               47000
#define RESISTOR_VALUE_NH3              47000
#define RESISTOR_VALUE_NO2              47000

#define STM32_VREFINT_VALUE             1.20f
#define STM32_TEMPERATURE_VOLTAGE_SLOPE 0.0043f
#define STM32_TEMPERATURE_VOLTAGE_25C   1.43f

void loop_adc() {
    adc_dma_finished = 0;
    HAL_ADC_Start_DMA(&hadc1, adc_dma, 5);
    while(!adc_dma_finished);
    HAL_Delay(100);
    HAL_ADC_Stop_DMA(&hadc1);

    // See __LL_ADC_CALC_TEMPERATURE_TYP_PARAMS for temperature formula.
    measure_value.stm32_vrefint = 4096.0 / adc_dma[4] * STM32_VREFINT_VALUE;
    measure_value.stm32_temp = (STM32_TEMPERATURE_VOLTAGE_25C - adc_dma[3] * measure_value.stm32_vrefint / 4096) / STM32_TEMPERATURE_VOLTAGE_SLOPE + 25;

    measure_value.mics6814_co = voltage_to_resistor_value(adc_dma[0], RESISTOR_VALUE_CO);
    measure_value.mics6814_nh3 = voltage_to_resistor_value(adc_dma[1], RESISTOR_VALUE_NH3);
    measure_value.mics6814_no2 = voltage_to_resistor_value(adc_dma[2], RESISTOR_VALUE_NO2);
    measure_value.status.mics6814_valid = 1;
}

void loop_bme680() {
    int32_t bme680_delay_ms = bme680_perform_measurement();
    if(-1 == bme680_delay_ms) return;

    struct bme680_field_data data;
    if(-1 == bme680_get_measurements(&data)) return;

    measure_value.bme680_temperature = data.temperature;
    measure_value.bme680_pressure = data.pressure;
    measure_value.bme680_humidity = data.humidity;
    measure_value.bme680_gas_resistance = data.gas_resistance;
    measure_value.status.bme680_valid = 1;
}

void loop_print() {
    if(measure_value.status.bme680_valid) {
        print_float("BME680 Tmp", measure_value.bme680_temperature);
        print_float("BME680 Prs", measure_value.bme680_pressure);
        print_float("BME680 Hum", measure_value.bme680_humidity);
        print_float("BME680 Gas", measure_value.bme680_gas_resistance);
    } else {
        println("BME680 Measure Error");
    }
    
    if(measure_value.status.mics6814_valid) {
        print_float("MICS   CO ", measure_value.mics6814_co);
        print_float("MICS   NH3", measure_value.mics6814_nh3);
        print_float("MICS   NO2", measure_value.mics6814_no2);
    } else {
        println("MICS Measure Error");
    }
    
    print_float("STM32  Tmp", measure_value.stm32_temp);
    print_float("STM32  Vrf", measure_value.stm32_vrefint);
    
    println("");
}

float voltage_to_resistor_value(uint32_t voltage, uint32_t resistor) {
    return 1.0 * resistor * voltage / (4096 - voltage);
}

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
