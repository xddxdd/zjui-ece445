#include "run_bme680.h"
#include "../run.h"

extern I2C_HandleTypeDef hi2c1;
#define BME680_I2C_INSTANCE hi2c1

static int8_t bme680_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);
static int8_t bme680_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);
static void bme680_delay(uint32_t period);
static uint32_t bme680_state_load(uint8_t *state_buffer, uint32_t n_buffer);
static uint32_t bme680_config_load(uint8_t *config_buffer, uint32_t n_buffer);

void bme680_output_ready(
    int64_t timestamp, float iaq, uint8_t iaq_accuracy, float temperature, float humidity,
    float pressure, float raw_temperature, float raw_humidity, float gas, bsec_library_return_t bsec_status,
    float static_iaq, float co2_equivalent, float breath_voc_equivalen
);

int8_t bme680_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    return HAL_I2C_Mem_Read(&BME680_I2C_INSTANCE, dev_id << 1, reg_addr, 1, data, len, 1000);
}

int8_t bme680_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    return HAL_I2C_Mem_Write(&BME680_I2C_INSTANCE, dev_id << 1, reg_addr, 1, data, len, 1000);
}

void bme680_delay(uint32_t period) {
    HAL_Delay(period);
}

uint32_t bme680_state_load(uint8_t *state_buffer, uint32_t n_buffer) { return 0; }
uint32_t bme680_config_load(uint8_t *config_buffer, uint32_t n_buffer) { return 0; }

int bme680_my_init() {
    return_values_init ret;
    
    /* Call to the function which initializes the BSEC library 
     * Switch on low-power mode and provide no temperature offset */
    ret = bsec_iot_init(BSEC_SAMPLE_RATE_LP, 0.0f, bme680_i2c_write, bme680_i2c_read, bme680_delay, bme680_state_load, bme680_config_load);
    if (ret.bme680_status) {
        /* Could not intialize BME680 */
        return (int)ret.bme680_status;
    } else if (ret.bsec_status) {
        /* Could not intialize BSEC library */
        return (int)ret.bsec_status;
    }
    
    return 0;
}

void bme680_my_loop() {
    /* BSEC sensor settings struct */
    bsec_bme_settings_t sensor_settings;

    /* get the timestamp in nanoseconds before calling bsec_sensor_control() */
    int64_t time_stamp = HAL_GetTick() * 1000000;
    
    /* Retrieve sensor settings to be used in this time instant by calling bsec_sensor_control */
    bsec_sensor_control(time_stamp, &sensor_settings);
    
    /* Trigger a measurement if necessary */
    bme680_bsec_trigger_measurement(&sensor_settings, bme680_delay);
    
    /* Read data from last measurement */
    bsec_input_t bsec_inputs[BSEC_MAX_PHYSICAL_SENSOR];
    uint8_t num_bsec_inputs = 0;
    bme680_bsec_read_data(time_stamp, bsec_inputs, &num_bsec_inputs, sensor_settings.process_data);
    
    /* Time to invoke BSEC to perform the actual processing */
    bme680_bsec_process_data(bsec_inputs, num_bsec_inputs, bme680_output_ready);
}

void bme680_output_ready(
    int64_t timestamp, float iaq, uint8_t iaq_accuracy, float temperature, float humidity,
    float pressure, float raw_temperature, float raw_humidity, float gas, bsec_library_return_t bsec_status,
    float static_iaq, float co2_equivalent, float breath_voc_equivalen
) {
    measure_value.bme680.air_quality = iaq;
    measure_value.bme680.gas_resistance = gas;
    measure_value.bme680.humidity = humidity;
    measure_value.bme680.pressure = pressure;
    measure_value.bme680.temperature = temperature;

    measure_value.valid.bme680 = 1;
}
