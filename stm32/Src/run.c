#include "run.h"
#include "bme680/run_bme680.h"
#include "main.h"
#include <stdio.h>

extern ADC_HandleTypeDef hadc1;
extern I2C_HandleTypeDef hi2c1;
extern RTC_HandleTypeDef hrtc;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

measure_value_t measure_value;

extern void loop_pms5003();
extern void loop_adc();
extern void loop_esp8266();

void loop_print();
void deep_sleep(uint32_t seconds);

void setup() {
    if(0 != bme680_my_init()) {
        while(1) {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
            HAL_Delay(100);
        }
    }
}

void deep_sleep(uint32_t seconds) {
    RTC_TimeTypeDef rtc_time = {0, 0, 0};
    RTC_AlarmTypeDef rtc_alarm = {
        {
            seconds / 3600,
            (seconds % 3600) / 60,
            seconds % 60
        },
        RTC_ALARM_A
    };

    HAL_RTC_SetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
    HAL_RTC_SetAlarm_IT(&hrtc, &rtc_alarm, RTC_FORMAT_BIN);

    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    extern volatile uint32_t uwTick;
    uwTick += seconds * 1000;
}

void loop() {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

    measure_value.bme680.air_quality = -1;
    measure_value.bme680.co2 = -1;
    measure_value.bme680.humidity = -1;
    measure_value.bme680.pressure = -1;
    measure_value.bme680.temperature = -1;
    measure_value.bme680.tvoc = -1;
    measure_value.mics6814.co = -1;
    measure_value.mics6814.nh3 = -1;
    measure_value.mics6814.no2 = -1;
    measure_value.pms5003.pm1 = -1;
    measure_value.pms5003.pm2_5 = -1;
    measure_value.pms5003.pm10 = -1;
    measure_value.stm32.temp = -1;
    measure_value.stm32.vrefint = -1;

    for(int i = 0; i < 3 && (-1 == measure_value.pms5003.pm1); i++) {
        loop_pms5003();
    }
    loop_adc();
    bme680_my_loop();
    loop_print();
    loop_esp8266();

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    deep_sleep(5);
}

void loop_print() {
    extern char tcp_send_buf[1024];
    extern uint32_t tcp_send_len;

    tcp_send_len = sprintf(
        tcp_send_buf,

        "PMS5003 PM1.0 %d\r\n"
        "PMS5003 PM2.5 %d\r\n"
        "PMS5003 PM10  %d\r\n"

        "BME680  Tmp   %.4f\r\n"
        "BME680  Prs   %.4f\r\n"
        "BME680  Hum   %.4f\r\n"
        "BME680  TVOC  %.4f\r\n"
        "BME680  CO2   %.4f\r\n"
        "BME680  IAQ   %.4f\r\n"

        "MICS    CO    %.4f\r\n"
        "MICS    NH3   %.4f\r\n"
        "MICS    NO2   %.4f\r\n"

        "STM32   Tmp   %.4f\r\n"
        "STM32   Vref  %.4f\r\n",

        measure_value.pms5003.pm1,
        measure_value.pms5003.pm2_5,
        measure_value.pms5003.pm10,

        measure_value.bme680.temperature,
        measure_value.bme680.pressure,
        measure_value.bme680.humidity,
        measure_value.bme680.tvoc,
        measure_value.bme680.co2,
        measure_value.bme680.air_quality,

        measure_value.mics6814.co,
        measure_value.mics6814.nh3,
        measure_value.mics6814.no2,

        measure_value.stm32.temp,
        measure_value.stm32.vrefint
    );
}
