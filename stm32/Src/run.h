#pragma once

#include <stdint.h>

#define HTTP_INFLUXDB_IP    "***REMOVED***"
#define HTTP_INFLUXDB_PORT  "8086"
#define HTTP_INFLUXDB_USER  "***REMOVED***"
#define HTTP_INFLUXDB_PASS  "***REMOVED***"
#define HTTP_INFLUXDB_DB    "air_quality"

#define WIFI_SSID           "ZJUWLAN"
#define WIFI_PASSWORD       ""

#define ZJUWLAN_USERNAME    "***REMOVED***"
#define ZJUWLAN_PASSWORD    "***REMOVED***"

#define TCP_BUF_SIZE        2048
#define GPS_FIELD_SIZE      32
typedef struct {

    struct {
        float temperature;
        float pressure;
        float humidity;
        float air_quality;
        float tvoc;
        float co2;
    } bme680;

    struct {
        float co;
        float nh3;
        float no2;
    } mics6814;

    struct {
        float temp;
        float vrefint;
    } stm32;

    struct {
        char latitude[GPS_FIELD_SIZE];
        char longitude[GPS_FIELD_SIZE];
    } gps;

    struct {
        uint16_t pm1;
        uint16_t pm2_5;
        uint16_t pm10;
    } pms5003;
} measure_value_t;

extern measure_value_t measure_value;

void setup();
void loop();
void blink(uint32_t times);

// printf.c
void print(char* data);