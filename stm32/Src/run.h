#pragma once

#include <stdint.h>

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
        uint16_t pm1;
        uint16_t pm2_5;
        uint16_t pm10;
    } pms5003;
} measure_value_t;

extern measure_value_t measure_value;

#define HTTP_INFLUXDB_IP    "192.168.0.254"
#define HTTP_INFLUXDB_PORT  "8086"
#define HTTP_INFLUXDB_USER  "***REMOVED***"
#define HTTP_INFLUXDB_PASS  "***REMOVED***"
#define HTTP_INFLUXDB_DB    "air_quality"

#define WIFI_SSID           "xqm2"
#define WIFI_PASSWORD       "98.01.20"

void setup();
void loop();
void fail();

// printf.c
void print(char* data);