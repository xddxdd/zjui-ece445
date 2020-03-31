#include "run_bme680.h"

extern I2C_HandleTypeDef hi2c1;
#define BME680_I2C_INSTANCE hi2c1

static int8_t bme680_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);
static int8_t bme680_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);
static void bme680_delay(uint32_t period);

static struct bme680_dev bme680;

int8_t bme680_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    return HAL_I2C_Mem_Read(&BME680_I2C_INSTANCE, dev_id << 1, reg_addr, 1, data, len, 1000);
}

int8_t bme680_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    return HAL_I2C_Mem_Write(&BME680_I2C_INSTANCE, dev_id << 1, reg_addr, 1, data, len, 1000);
}

void bme680_delay(uint32_t period) {
    HAL_Delay(period);
}

int32_t bme680_create_structure() {
    int8_t ret;

    bme680.dev_id = 0x77;
    bme680.intf = BME680_I2C_INTF;
    bme680.read = bme680_i2c_read;
    bme680.write = bme680_i2c_write;
    bme680.delay_ms = bme680_delay;
    if(BME680_OK != (ret = bme680_init(&bme680))) return ret;

    bme680.tph_sett.os_temp = BME680_OS_16X;
    bme680.tph_sett.os_hum = BME680_OS_16X;
    bme680.tph_sett.os_pres = BME680_OS_16X;
    bme680.tph_sett.filter = BME680_FILTER_SIZE_3;
    bme680.gas_sett.heatr_temp = 320;
    bme680.gas_sett.heatr_dur = 150;
    bme680.gas_sett.heatr_ctrl = BME680_ENABLE_HEATER;
    bme680.gas_sett.run_gas = BME680_ENABLE_GAS_MEAS;
    
    return 0;
}

int32_t bme680_perform_measurement() {
    bme680.power_mode = BME680_FORCED_MODE;

    uint8_t bme680_target_settings = 0;
    bme680_target_settings |= BME680_FILTER_SEL;    // IIR filter
    bme680_target_settings |= BME680_HCNTRL_SEL;    // Heater
    bme680_target_settings |= BME680_OST_SEL;       // Temperature oversampling
    bme680_target_settings |= BME680_OSP_SEL;       // Pressure oversampling
    bme680_target_settings |= BME680_OSH_SEL;       // Humidity oversampling
    bme680_target_settings |= BME680_GAS_SENSOR_SEL;// Gas measurement

    if(BME680_OK != bme680_set_sensor_settings(bme680_target_settings, &bme680)) return -1;
    if(BME680_OK != bme680_set_sensor_mode(&bme680)) return -1;

    uint16_t measure_duration;
    bme680_get_profile_dur(&measure_duration, &bme680);
    return measure_duration;
}

int32_t bme680_get_measurements(struct bme680_field_data* data) {
    if(BME680_OK != bme680_get_sensor_data(data, &bme680)) return -1;
    bme680.power_mode = BME680_SLEEP_MODE;
    if(BME680_OK != bme680_set_sensor_mode(&bme680)) return -1;
    if(!(data->status & BME680_HEAT_STAB_MSK)) {
        data->gas_resistance = 0;
    }
    return 0;
}