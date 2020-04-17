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

struct {
    union {
        struct __attribute__((packed)) {
            uint8_t pms5003 : 1;
            uint8_t bme680 : 1;
            uint8_t mics6814 : 1;
            uint8_t __reserved : 5;
        };
        uint8_t raw;
    } valid;

    struct {
        float temperature;
        float pressure;
        float humidity;
        float gas_resistance;
    } bme680;

    struct {
        float co;
        float nh3;
        float no2;
    } mics6814;

    struct {
        float temp;
        float vrefint;
        uint64_t timestamp;
    } stm32;

    struct {
        uint16_t pm1;
        uint16_t pm2_5;
        uint16_t pm10;
    } pms5003;
} measure_value;

void print(char* s);
void println(char* s);
void print_float(char* name, float value);
void print_int(char* name, int value);

uint64_t get_tick();
void setup_pms5003();
void loop_pms5003();
void loop_adc();
void loop_bme680();
void loop_print();
float voltage_to_resistor_value(uint32_t voltage, uint32_t resistor);

void setup() {
    if(0 != bme680_create_structure()) {
        while(1) {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
            HAL_Delay(100);
        }
    }
}

// Get tick function, considering clock rollover (happens each ~50 days)
uint64_t get_tick() {
    static uint32_t last_tick = 0;
    static uint32_t rollover_times = 0;

    uint32_t tick = HAL_GetTick();
    // If the tick number ever decreases, a rollover happened
    if(tick < last_tick) rollover_times++;
    last_tick = tick;
    return (((uint64_t) rollover_times) << 32) | tick;
}

void loop() {
    HAL_Delay(1000);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

    measure_value.valid.pms5003 = 0;
    measure_value.valid.bme680 = 0;
    measure_value.valid.mics6814 = 0;

    loop_pms5003();
    loop_adc();
    loop_bme680();
    measure_value.stm32.timestamp = get_tick();

    loop_print();

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

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
    measure_value.valid.pms5003 = 1;
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
    measure_value.stm32.vrefint = 4096.0 / adc_dma[4] * STM32_VREFINT_VALUE;
    measure_value.stm32.temp = (STM32_TEMPERATURE_VOLTAGE_25C - adc_dma[3] * measure_value.stm32.vrefint / 4096) / STM32_TEMPERATURE_VOLTAGE_SLOPE + 25;

    measure_value.mics6814.co = voltage_to_resistor_value(adc_dma[0], RESISTOR_VALUE_CO);
    measure_value.mics6814.nh3 = voltage_to_resistor_value(adc_dma[1], RESISTOR_VALUE_NH3);
    measure_value.mics6814.no2 = voltage_to_resistor_value(adc_dma[2], RESISTOR_VALUE_NO2);
    measure_value.valid.mics6814 = 1;
}

void loop_bme680() {
    int32_t bme680_delay_ms = bme680_perform_measurement();
    if(-1 == bme680_delay_ms) return;

    struct bme680_field_data data;
    if(-1 == bme680_get_measurements(&data)) return;

    measure_value.bme680.temperature = data.temperature;
    measure_value.bme680.pressure = data.pressure;
    measure_value.bme680.humidity = data.humidity;
    measure_value.bme680.gas_resistance = data.gas_resistance;
    measure_value.valid.bme680 = 1;
}

void loop_print() {
    if(measure_value.valid.pms5003) {
        print_float("PMS5003 PM1.0", measure_value.pms5003.pm1);
        print_float("PMS5003 PM2.5", measure_value.pms5003.pm2_5);
        print_float("PMS5003 PM10 ", measure_value.pms5003.pm10);
    } else {
        println("PMS5003 Measure Error");
    }
    
    if(measure_value.valid.bme680) {
        print_float("BME680  Tmp", measure_value.bme680.temperature);
        print_float("BME680  Prs", measure_value.bme680.pressure);
        print_float("BME680  Hum", measure_value.bme680.humidity);
        print_float("BME680  Gas", measure_value.bme680.gas_resistance);
    } else {
        println("BME680 Measure Error");
    }
    
    if(measure_value.valid.mics6814) {
        print_float("MICS    CO ", measure_value.mics6814.co);
        print_float("MICS    NH3", measure_value.mics6814.nh3);
        print_float("MICS    NO2", measure_value.mics6814.no2);
    } else {
        println("MICS Measure Error");
    }

    print_float("STM32   Tmp", measure_value.stm32.temp);
    print_float("STM32   Vrf", measure_value.stm32.vrefint);
    print_int  ("STM32   Time", measure_value.stm32.timestamp);
    
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
