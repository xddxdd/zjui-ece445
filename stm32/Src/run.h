#pragma once

#include <stdint.h>

typedef struct {
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

void setup();
void loop();