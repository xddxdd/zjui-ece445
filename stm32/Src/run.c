#include "run.h"
#include "bme680/run_bme680.h"
#include "main.h"
#include <stdio.h>

// Disable uploading calibration & testing
#define TEST_MODE 1

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
uint32_t id = 10;

void loop_job(uint32_t do_upload);
void loop_print();
void deep_sleep(uint32_t seconds);

void setup() {
    // Virtual release of key of battery module
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);

    if(0 != bme680_my_init()) blink(6);

    // Locate with GPS
    measure_value.gps.latitude[0] = '0';
    measure_value.gps.latitude[1] = '\0';
    measure_value.gps.longitude[0] = '0';
    measure_value.gps.longitude[1] = '\0';

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

    // Continuously run to warm up some sensors,
    // and generate random session ID with measurement uncertainty in process (disabled for now)
    for(int i = 0; i < 5; i++) {
        // Turn on LED & MICS6814 & GPS, and preheat for 30s
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
        loop_job(0);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

        HAL_Delay(1000);
    }
    // // Turn off MICS6814 & GPS
    // // Not necessary since they will be used immediately afterwards
    // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
    // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
}

void deep_sleep(uint32_t seconds) {
#if 0 == TEST_MODE
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
#endif
}

void loop() {
    // Turn on LED & MICS6814 & GPS
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

    // Preheat MICS6814 for 30s, and give GPS 30s for scanning satellites
    #if 0 == TEST_MODE
        // Don't need to toggle battery module every time
        // since MICS6814 & GPS are on and drawing lots of current
        HAL_Delay(30000);
    #endif
    loop_job(1);

    // Turn off LED & MICS6814 & GPS
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);

    // Wait 85 cycles of 10.5s each, total = 892.5s
    #if 0 == TEST_MODE
        for(size_t i = 0; i < 85; i++) {
            // Toggle battery module each time
            deep_sleep(10);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
            HAL_Delay(500);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
        }
    #endif
}

void loop_job(uint32_t do_upload) {
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
    measure_value.stm32.vbat = -1;

    if(do_upload) {
        #if 0 == TEST_MODE
            GPS_Process();
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
        #endif
    }

    loop_pms5003();
    loop_adc();
    bme680_my_loop();

    if(do_upload) {
        loop_print();
        #if 0 == TEST_MODE
            loop_esp8266();
        #endif
    }

    // Turn off LED
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

void loop_print() {
    char tcp_content_buf[TCP_BUF_SIZE];
    uint32_t tcp_content_len;
    tcp_content_len = snprintf(
        tcp_content_buf,
        TCP_BUF_SIZE,

        #if 0 == TEST_MODE
            "pms5003_pm1,id=%lu value=%u\n"
            "pms5003_pm2_5,id=%lu value=%u\n"
            "pms5003_pm10,id=%lu value=%u\n"
            "bme680_tmp,id=%lu value=%f\n"
            "bme680_prs,id=%lu value=%f\n"
            "bme680_hum,id=%lu value=%f\n"
            "bme680_tvoc,id=%lu value=%f\n"
            "bme680_co2,id=%lu value=%f\n"
            "bme680_iaq,id=%lu value=%f\n"
            "mics_co,id=%lu value=%f\n"
            "mics_nh3,id=%lu value=%f\n"
            "mics_no2,id=%lu value=%f\n"
            "stm32_tmp,id=%lu value=%f\n"
            "stm32_vref,id=%lu value=%f\n"
            "stm32_vbat,id=%lu value=%f\n"
            "gps_lat,id=%lu value=%s\n"
            "gps_lon,id=%lu value=%s\n"
        #else
            "pms5003_pm1,id=%lu value=%u\r\n"
            "pms5003_pm2_5,id=%lu value=%u\r\n"
            "pms5003_pm10,id=%lu value=%u\r\n"
            "bme680_tmp,id=%lu value=%f\r\n"
            "bme680_prs,id=%lu value=%f\r\n"
            "bme680_hum,id=%lu value=%f\r\n"
            "bme680_tvoc,id=%lu value=%f\r\n"
            "bme680_co2,id=%lu value=%f\r\n"
            "bme680_iaq,id=%lu value=%f\r\n"
            "mics_co,id=%lu value=%f\r\n"
            "mics_nh3,id=%lu value=%f\r\n"
            "mics_no2,id=%lu value=%f\r\n"
            "stm32_tmp,id=%lu value=%f\r\n"
            "stm32_vref,id=%lu value=%f\r\n"
            "stm32_vbat,id=%lu value=%f\r\n"
            "gps_lat,id=%lu value=%s\r\n"
            "gps_lon,id=%lu value=%s\r\n"
        #endif
        
        ,

        id, measure_value.pms5003.pm1,
        id, measure_value.pms5003.pm2_5,
        id, measure_value.pms5003.pm10,
        id, measure_value.bme680.temperature,
        id, measure_value.bme680.pressure,
        id, measure_value.bme680.humidity,
        id, measure_value.bme680.tvoc,
        id, measure_value.bme680.co2,
        id, measure_value.bme680.air_quality,
        id, measure_value.mics6814.co,
        id, measure_value.mics6814.nh3,
        id, measure_value.mics6814.no2,
        id, measure_value.stm32.temp,
        id, measure_value.stm32.vrefint,
        id, measure_value.stm32.vbat,
        id, measure_value.gps.latitude,
        id, measure_value.gps.longitude
    );
    #if 1 == TEST_MODE
        HAL_UART_Transmit(&huart1, (uint8_t*) tcp_content_buf, tcp_content_len, 1000);
    #endif

    #if 0 == TEST_MODE
    extern char tcp_send_buf[TCP_BUF_SIZE];
    extern uint32_t tcp_send_len;
    const char template[] = 
        "POST /write?u=" HTTP_INFLUXDB_USER "&p=" HTTP_INFLUXDB_PASS "&db=" HTTP_INFLUXDB_DB " HTTP/1.1\r\n"
        "Host: " HTTP_INFLUXDB_IP ":" HTTP_INFLUXDB_PORT "\r\n"
        "User-Agent: zjui-ece/4.4.5\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Connection: close\r\n"
        "Content-Length: %lu\r\n"
        "\r\n"
        "%s"
    ;
    tcp_send_len = snprintf(
        tcp_send_buf,
        TCP_BUF_SIZE,
        template,
        tcp_content_len,
        tcp_content_buf
    );
    #endif
}

void blink(uint32_t times) {
    for(int i = 0; i < times; i++) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
        HAL_Delay(100);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
        HAL_Delay(100);
    }
    HAL_Delay(500);
}