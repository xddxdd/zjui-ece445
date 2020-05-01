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
extern void GPS_Process(void);
uint32_t random_id = 0;

void loop_job(uint32_t do_upload);
void loop_print();
void deep_sleep(uint32_t seconds);

void setup() {
    if(0 != bme680_my_init()) fail();

    // Locate with GPS
    GPS_Process();

    // Continuously run to warm up some sensors,
    // and generate random session ID with measurement uncertainty in process
    for(int i = 0; i < 10; i++) {
        loop_job(0);
        uint32_t* ptr = (uint32_t*) &measure_value;
        for(int j = 0; j < sizeof(measure_value) / sizeof(uint32_t); j++) {
            random_id ^= ptr[j];
        }
        HAL_Delay(1000);
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
    loop_job(1);
    deep_sleep(5);
}

void loop_job(uint32_t do_upload) {
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

    // loop_pms5003();
    loop_adc();
    // bme680_my_loop();
    if(do_upload) {
        loop_print();
        // loop_esp8266();
    }
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

void loop_print() {
    extern char tcp_send_buf[1024];
    extern uint32_t tcp_send_len;

    char tcp_content_buf[1024];
    uint32_t tcp_content_len;

    tcp_content_len = snprintf(
        tcp_content_buf,
        1024,

        "pms5003_pm1,random_id=%lu value=%u\n"
        "pms5003_pm2_5,random_id=%lu value=%u\n"
        "pms5003_pm10,random_id=%lu value=%u\n"
        "bme680_tmp,random_id=%lu value=%.4f\n"
        "bme680_prs,random_id=%lu value=%.4f\n"
        "bme680_hum,random_id=%lu value=%.4f\n"
        "bme680_tvoc,random_id=%lu value=%.4f\n"
        "bme680_co2,random_id=%lu value=%.4f\n"
        "bme680_iaq,random_id=%lu value=%.4f\n"
        "mics_co,random_id=%lu value=%.4f\n"
        "mics_nh3,random_id=%lu value=%.4f\n"
        "mics_no2,random_id=%lu value=%.4f\n"
        "stm32_tmp,random_id=%lu value=%.4f\n"
        "stm32_vref,random_id=%lu value=%.4f\n",

        random_id, measure_value.pms5003.pm1,
        random_id, measure_value.pms5003.pm2_5,
        random_id, measure_value.pms5003.pm10,
        random_id, measure_value.bme680.temperature,
        random_id, measure_value.bme680.pressure,
        random_id, measure_value.bme680.humidity,
        random_id, measure_value.bme680.tvoc,
        random_id, measure_value.bme680.co2,
        random_id, measure_value.bme680.air_quality,
        random_id, measure_value.mics6814.co,
        random_id, measure_value.mics6814.nh3,
        random_id, measure_value.mics6814.no2,
        random_id, measure_value.stm32.temp,
        random_id, measure_value.stm32.vrefint
    );

    const char template[] = 
        "POST /write?u=%s&p=%s&db=%s HTTP/1.1\r\n"
        "Host: %s:%u\r\n"
        "User-Agent: zjui-ece/4.4.5\r\n"
        "Content-Length: %lu\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s"
    ;
    tcp_send_len = snprintf(
        tcp_send_buf,
        1024,
        
        template,

        HTTP_INFLUXDB_USER,
        HTTP_INFLUXDB_PASS,
        HTTP_INFLUXDB_DB,
        HTTP_INFLUXDB_IP,
        HTTP_INFLUXDB_PORT,
        tcp_content_len,
        tcp_content_buf
    );
    HAL_UART_Transmit(&huart1, (uint8_t*) tcp_send_buf, tcp_send_len, 1000);
}

void fail() {
    for(int i = 0; i < 3; i++) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
        HAL_Delay(100);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
        HAL_Delay(100);
    }
}