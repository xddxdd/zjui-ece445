#include "main.h"
#include "run.h"
#include <math.h>

extern ADC_HandleTypeDef hadc1;
uint32_t adc_dma[5];
volatile uint32_t adc_dma_finished;

#define RESISTOR_VALUE_CO               47000
#define RESISTOR_VALUE_NH3              47000
#define RESISTOR_VALUE_NO2              47000

#define MICS6814_BASE_RESISTANCE_CO     1000000
#define MICS6814_BASE_RESISTANCE_NH3    1000000
#define MICS6814_BASE_RESISTANCE_NO2    10000

// #define MICS6814_BASE_ANALOG_CO     (1.0 * MICS6814_BASE_RESISTANCE_CO / (MICS6814_BASE_RESISTANCE_CO + RESISTOR_VALUE_CO))
// #define MICS6814_BASE_ANALOG_NH3    (1.0 * MICS6814_BASE_RESISTANCE_NH3 / (MICS6814_BASE_RESISTANCE_NH3 + RESISTOR_VALUE_NH3))
// #define MICS6814_BASE_ANALOG_NO2    (1.0 * MICS6814_BASE_RESISTANCE_NO2 / (MICS6814_BASE_RESISTANCE_NO2 + RESISTOR_VALUE_NO2))

#define MICS6814_1X_PPM_CO              4.5
#define MICS6814_1X_PPM_NH3             0.7
#define MICS6814_1X_PPM_NO2             0.15
#define MICS6814_LOG_SLOPE_CO           -0.8
#define MICS6814_LOG_SLOPE_NH3          -0.55
#define MICS6814_LOG_SLOPE_NO2          1.0

#define STM32_VREFINT_VALUE             1.20f
#define STM32_TEMPERATURE_VOLTAGE_SLOPE 0.0043f
#define STM32_TEMPERATURE_VOLTAGE_25C   1.43f

float voltage_to_resistor_value(uint32_t voltage, uint32_t resistor);

void loop_adc() {
    adc_dma_finished = 0;
    HAL_ADC_Start_DMA(&hadc1, adc_dma, 5);
    while(!adc_dma_finished);
    HAL_Delay(100);
    HAL_ADC_Stop_DMA(&hadc1);

    // See __LL_ADC_CALC_TEMPERATURE_TYP_PARAMS for temperature formula.
    measure_value.stm32.vrefint = 4096.0 / adc_dma[4] * STM32_VREFINT_VALUE;
    measure_value.stm32.temp = (STM32_TEMPERATURE_VOLTAGE_25C - adc_dma[3] * measure_value.stm32.vrefint / 4096) / STM32_TEMPERATURE_VOLTAGE_SLOPE + 25;

    // MICS-6814 formula from:
    // https://github.com/noorkhokhar99/MICS6814/blob/master/MICS6814.cpp

    // measure_value.mics6814.co = 1.0 * adc_dma[0] / MICS6814_BASE_ANALOG_CO * (1 - MICS6814_BASE_ANALOG_CO) / (4096.0 - adc_dma[0]);
    // measure_value.mics6814.nh3 = 1.0 * adc_dma[1] / MICS6814_BASE_ANALOG_NH3 * (1 - MICS6814_BASE_ANALOG_NH3) / (4096.0 - adc_dma[1]);
    // measure_value.mics6814.no2 = 1.0 * adc_dma[2] / MICS6814_BASE_ANALOG_NO2 * (1 - MICS6814_BASE_ANALOG_NO2) / (4096.0 - adc_dma[2]);

    // measure_value.mics6814.co = pow(measure_value.mics6814.co, -1.179) * 4.385;
    // measure_value.mics6814.nh3 = pow(measure_value.mics6814.nh3, -1.67) * 1.47;
    // measure_value.mics6814.no2 = pow(measure_value.mics6814.no2, 1.007) * 6.855;

    measure_value.mics6814.co = voltage_to_resistor_value(adc_dma[0], RESISTOR_VALUE_CO);
    measure_value.mics6814.nh3 = voltage_to_resistor_value(adc_dma[1], RESISTOR_VALUE_NH3);
    measure_value.mics6814.no2 = voltage_to_resistor_value(adc_dma[2], RESISTOR_VALUE_NO2);

    measure_value.mics6814.co = MICS6814_1X_PPM_CO * powf(10, log10f(measure_value.mics6814.co / MICS6814_BASE_RESISTANCE_CO) / MICS6814_LOG_SLOPE_CO);
    measure_value.mics6814.nh3 = MICS6814_1X_PPM_NH3 * powf(10, log10f(measure_value.mics6814.nh3 / MICS6814_BASE_RESISTANCE_NH3) / MICS6814_LOG_SLOPE_NH3);
    measure_value.mics6814.no2 = MICS6814_1X_PPM_NO2 * powf(10, log10f(measure_value.mics6814.no2 / MICS6814_BASE_RESISTANCE_NO2) / MICS6814_LOG_SLOPE_NO2);

    measure_value.valid.mics6814 = 1;
}

float voltage_to_resistor_value(uint32_t voltage, uint32_t resistor) {
    return 1.0 * resistor * voltage / (4096 - voltage);
}
