#pragma once

#include "bme680.h"
#include "main.h"

int8_t bme680_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);

int32_t bme680_create_structure();
int32_t bme680_perform_measurement();
struct bme680_field_data bme680_get_measurements();