#include "run.h"
#include "bme680/run_bme680.h"
#include "main.h"
#include <stdio.h>

extern ADC_HandleTypeDef hadc1;
extern CRC_HandleTypeDef hcrc;
extern I2C_HandleTypeDef hi2c1;
extern RTC_HandleTypeDef hrtc;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

measure_value_t measure_value;

extern void loop_pms5003();
extern void loop_adc();

void loop_print();

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

void loop() {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

    measure_value.valid.pms5003 = 0;
    measure_value.valid.bme680 = 0;
    measure_value.valid.mics6814 = 0;

    for(int i = 0; i < 3 && !measure_value.valid.pms5003; i++) {
        loop_pms5003();
    }
    loop_adc();
    bme680_my_loop();

    loop_print();

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

    RTC_TimeTypeDef rtc_time;
    rtc_time.Hours = 0;
    rtc_time.Minutes = 0;
    rtc_time.Seconds = 0;

    RTC_AlarmTypeDef rtc_alarm;
    rtc_alarm.Alarm = RTC_ALARM_A;
    rtc_alarm.AlarmTime.Hours = 0;
    rtc_alarm.AlarmTime.Minutes = 0;
    rtc_alarm.AlarmTime.Seconds = 5;

    HAL_RTC_SetTime(&hrtc, &rtc_time, RTC_FORMAT_BCD);
    HAL_RTC_SetAlarm_IT(&hrtc, &rtc_alarm, RTC_FORMAT_BCD);

    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    extern volatile uint32_t uwTick;
    uwTick += rtc_alarm.AlarmTime.Seconds * 1000;
}

void loop_print() {
    if(measure_value.valid.pms5003) {
        printf("PMS5003 PM1.0 %d\r\n", measure_value.pms5003.pm1);
        printf("PMS5003 PM2.5 %d\r\n", measure_value.pms5003.pm2_5);
        printf("PMS5003 PM10  %d\r\n", measure_value.pms5003.pm10);
    } else {
        printf("PMS5003 Measure Error\r\n");
    }
    
    if(measure_value.valid.bme680) {
        printf("BME680  Tmp   %.4f\r\n", measure_value.bme680.temperature);
        printf("BME680  Prs   %.4f\r\n", measure_value.bme680.pressure);
        printf("BME680  Hum   %.4f\r\n", measure_value.bme680.humidity);
        printf("BME680  TVOC  %.4f\r\n", measure_value.bme680.tvoc);
        printf("BME680  CO2   %.4f\r\n", measure_value.bme680.co2);
        printf("BME680  IAQ   %.4f\r\n", measure_value.bme680.air_quality);
    } else {
        printf("BME680 Measure Error\r\n");
    }
    
    if(measure_value.valid.mics6814) {
        printf("MICS    CO    %.4f\r\n", measure_value.mics6814.co);
        printf("MICS    NH3   %.4f\r\n", measure_value.mics6814.nh3);
        printf("MICS    NO2   %.4f\r\n", measure_value.mics6814.no2);
    } else {
        printf("MICS Measure Error\r\n");
    }

    printf("STM32   Tmp   %.4f\r\n", measure_value.stm32.temp);
    printf("STM32   Vrf   %.4f\r\n", measure_value.stm32.vrefint);
    
    printf("\r\n");
}
