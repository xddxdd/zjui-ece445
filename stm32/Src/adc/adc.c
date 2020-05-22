#include "main.h"
#include "run.h"
// #include <math.h>

// Include this to prevent compile error
uint32_t __errno;

#define ADC_DMA_CHANNEL_COUNT 6

uint32_t adc_dma[ADC_DMA_CHANNEL_COUNT];
volatile uint32_t adc_dma_finished;

#define RESISTOR_VALUE_CO               47000
#define RESISTOR_VALUE_NH3              47000
#define RESISTOR_VALUE_NO2              47000

#define MICS6814_1X_PPM_CO              2.0
#define MICS6814_1X_PPM_NH3             20.0
#define MICS6814_1X_PPM_NO2             15.0
#define MICS6814_SLOPE_CO               -0.8
#define MICS6814_SLOPE_NH3              -0.55
#define MICS6814_SLOPE_NO2              1.0

#define STM32_VREFINT_VALUE             1.20f
#define STM32_TEMPERATURE_VOLTAGE_SLOPE 0.0043f
#define STM32_TEMPERATURE_VOLTAGE_25C   1.43f
#define STM32_VBAT_RESISTOR_LOW         100000
#define STM32_VBAT_RESISTOR_HIGH        100000

#define AVERAGE_SAMPLES                 5

void loop_adc() {
    extern ADC_HandleTypeDef hadc1;
    
    for(size_t i = 0; i < AVERAGE_SAMPLES; i++) {
        HAL_Delay(100);
        adc_dma_finished = 0;
        HAL_ADC_Start_DMA(&hadc1, adc_dma, ADC_DMA_CHANNEL_COUNT);
        while(!adc_dma_finished);
        HAL_Delay(100);
        HAL_ADC_Stop_DMA(&hadc1);

        measure_value.mics6814.co   += adc_dma[0];
        measure_value.mics6814.nh3  += adc_dma[1];
        measure_value.mics6814.no2  += adc_dma[2];
        measure_value.stm32.vbat    += adc_dma[3];
        measure_value.stm32.temp    += adc_dma[4];
        measure_value.stm32.vrefint += adc_dma[5];
    }

    measure_value.mics6814.co   /= AVERAGE_SAMPLES;
    measure_value.mics6814.nh3  /= AVERAGE_SAMPLES;
    measure_value.mics6814.no2  /= AVERAGE_SAMPLES;
    measure_value.stm32.vbat    /= AVERAGE_SAMPLES;
    measure_value.stm32.temp    /= AVERAGE_SAMPLES;
    measure_value.stm32.vrefint /= AVERAGE_SAMPLES;


    // See __LL_ADC_CALC_TEMPERATURE_TYP_PARAMS for temperature formula.
    measure_value.stm32.vrefint = 4096.0 / measure_value.stm32.vrefint * STM32_VREFINT_VALUE;
    measure_value.stm32.temp = (STM32_TEMPERATURE_VOLTAGE_25C - measure_value.stm32.temp * measure_value.stm32.vrefint / 4096) / STM32_TEMPERATURE_VOLTAGE_SLOPE + 25;
    measure_value.stm32.vbat = (measure_value.stm32.vbat * measure_value.stm32.vrefint / 4096.0) / (STM32_VBAT_RESISTOR_LOW) * (STM32_VBAT_RESISTOR_LOW + STM32_VBAT_RESISTOR_HIGH);

    measure_value.mics6814.co  = MICS6814_1X_PPM_CO  + 1.0 * (4096 - measure_value.mics6814.co ) / (1 + measure_value.mics6814.co ) / MICS6814_SLOPE_CO;
    measure_value.mics6814.nh3 = MICS6814_1X_PPM_NH3 + 1.0 * (4096 - measure_value.mics6814.nh3) / (1 + measure_value.mics6814.nh3) / MICS6814_SLOPE_NH3;
    measure_value.mics6814.no2 = MICS6814_1X_PPM_NO2 + 1.0 * (4096 - measure_value.mics6814.no2) / (1 + measure_value.mics6814.no2) / MICS6814_SLOPE_NO2;
}
