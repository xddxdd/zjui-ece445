#include "run_bme680.h"

extern I2C_HandleTypeDef hi2c1;
#define BME680_I2C_INSTANCE hi2c1
extern UART_HandleTypeDef huart1;

// static int8_t bme680_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);
static int8_t bme680_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);
static void bme680_delay(uint32_t period);

static struct bme680_dev bme680;

int8_t bme680_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
  int8_t ret;
  if(HAL_OK != (ret = HAL_I2C_Master_Transmit(&BME680_I2C_INSTANCE, dev_id, &reg_addr, 1, HAL_MAX_DELAY))) {
      char buf[] = "I2C REC1 _\r\n";
      const char map[] = "0123456789ABCDEF";

      if(ret < 0) ret = -ret;
      buf[9] = map[ret];
      
      HAL_UART_Transmit(&huart1, buf, 12, HAL_MAX_DELAY);
      return ret;
  }
  if(HAL_OK != (ret = HAL_I2C_Master_Receive(&BME680_I2C_INSTANCE, dev_id, data, len, HAL_MAX_DELAY))) {
      char buf[] = "I2C REC2 _\r\n";
      const char map[] = "0123456789ABCDEF";

      if(ret < 0) ret = -ret;
      buf[9] = map[ret];
      
      HAL_UART_Transmit(&huart1, buf, 12, HAL_MAX_DELAY);
      return ret;
  }
  return 0;
}

int8_t bme680_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
  int8_t ret;
  for(uint16_t i = 0; i < len; i++) {
    char buf[2] = {reg_addr + i, data[i]};
    if(HAL_OK != (ret = HAL_I2C_Master_Transmit(&BME680_I2C_INSTANCE, dev_id, &buf, 2, HAL_MAX_DELAY))) {
      char buf[] = "I2C SEND _\r\n";
      const char map[] = "0123456789ABCDEF";

      if(ret < 0) ret = -ret;
      buf[9] = map[ret];
      
      HAL_UART_Transmit(&huart1, buf, 12, HAL_MAX_DELAY);
      return ret;
    }
  }
  // if(HAL_OK != (ret = HAL_I2C_Master_Transmit(&BME680_I2C_INSTANCE, dev_id, &reg_addr, 1, HAL_MAX_DELAY))) {
  //   return ret;
  // }
  // if(HAL_OK != (ret = HAL_I2C_Master_Transmit(&BME680_I2C_INSTANCE, dev_id, data, len, HAL_MAX_DELAY))) {
  //   return ret;
  // }
  return 0;
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

  bme680.tph_sett.os_temp = BME680_OS_8X;
  bme680.tph_sett.os_hum = BME680_OS_2X;
  bme680.tph_sett.os_pres = BME680_OS_4X;
  bme680.tph_sett.filter = BME680_FILTER_SIZE_3;
  bme680.gas_sett.heatr_temp = 320;
  bme680.gas_sett.heatr_dur = 150;
  bme680.gas_sett.heatr_ctrl = BME680_ENABLE_HEATER;
  bme680.gas_sett.run_gas = BME680_ENABLE_GAS_MEAS;
  bme680.power_mode = BME680_SLEEP_MODE;
  
  if(BME680_OK != (ret = bme680_set_sensor_settings(
    BME680_OST_SEL | BME680_OSH_SEL | BME680_OSP_SEL | BME680_FILTER_SEL | BME680_GAS_SENSOR_SEL,
    &bme680
  ))) return ret;
  if(BME680_OK != (ret = bme680_set_sensor_mode(&bme680))) return ret;

  return 0;
}

int32_t bme680_perform_measurement() {
  bme680.power_mode = BME680_SLEEP_MODE;
  if(BME680_OK != bme680_set_sensor_mode(&bme680)) return -1;

  uint16_t measure_duration;
  bme680_get_profile_dur(&measure_duration, &bme680);
  return measure_duration;
}

struct bme680_field_data bme680_get_measurements() {
  struct bme680_field_data data;
  if(BME680_OK != bme680_get_sensor_data(&data, &bme680)) {
    data.meas_index = -1;
    return data;
  }
  data.temperature /= 100;
  data.humidity /= 1000;
  return data;
}