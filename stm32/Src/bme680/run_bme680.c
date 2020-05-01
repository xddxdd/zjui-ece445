#include "run_bme680.h"
#include "../run.h"

extern I2C_HandleTypeDef hi2c1;
#define BME680_I2C_INSTANCE hi2c1

static int8_t bme680_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);
static int8_t bme680_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);

void bme680_output_ready(
    float iaq, float temperature, float humidity,
    float pressure, float co2_equivalent, float breath_voc_equivalent
);

int8_t bme680_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    return HAL_I2C_Mem_Read(&BME680_I2C_INSTANCE, dev_id << 1, reg_addr, 1, data, len, 1000);
}

int8_t bme680_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    return HAL_I2C_Mem_Write(&BME680_I2C_INSTANCE, dev_id << 1, reg_addr, 1, data, len, 1000);
}

int bme680_my_init() {
    return_values_init ret;
    
    /* Call to the function which initializes the BSEC library 
     * Switch on low-power mode and provide no temperature offset */
    ret = bsec_iot_init(BSEC_SAMPLE_RATE_ULP, 0.0f, bme680_i2c_write, bme680_i2c_read, HAL_Delay);
    if (ret.bme680_status) {
        /* Could not intialize BME680 */
        return (int)ret.bme680_status;
    } else if (ret.bsec_status) {
        /* Could not intialize BSEC library */
        return (int)ret.bsec_status;
    }
    
    return 0;
}

int64_t bme680_fake_timestamp = 0;
void bme680_my_loop() {
    /* BSEC sensor settings struct */
    bsec_bme_settings_t sensor_settings;

    /* Retrieve sensor settings to be used in this time instant by calling bsec_sensor_control */
    bsec_sensor_control(bme680_fake_timestamp, &sensor_settings);
    if(bme680_fake_timestamp < sensor_settings.next_call) {
        bme680_fake_timestamp = sensor_settings.next_call;
    }

    /* Trigger a measurement if necessary */
    bme680_bsec_trigger_measurement(&sensor_settings, HAL_Delay);
    
    /* Read data from last measurement */
    bsec_input_t bsec_inputs[BSEC_MAX_PHYSICAL_SENSOR];
    uint8_t num_bsec_inputs = 0;
    bme680_bsec_read_data(bme680_fake_timestamp, bsec_inputs, &num_bsec_inputs, sensor_settings.process_data);
    
    /* Time to invoke BSEC to perform the actual processing */
    bme680_bsec_process_data(bsec_inputs, num_bsec_inputs, bme680_output_ready);
}

void bme680_output_ready(
    float iaq, float temperature, float humidity,
    float pressure, float co2_equivalent, float breath_voc_equivalent
) {
    measure_value.bme680.air_quality = iaq;
    measure_value.bme680.humidity = humidity;
    measure_value.bme680.pressure = pressure;
    measure_value.bme680.temperature = temperature;
    measure_value.bme680.tvoc = breath_voc_equivalent;
    measure_value.bme680.co2 = co2_equivalent;
}
