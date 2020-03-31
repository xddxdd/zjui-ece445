#pragma once

#include "bme680.h"
#include "main.h"

int32_t bme680_create_structure();
int32_t bme680_perform_measurement();
int32_t bme680_get_measurements(struct bme680_field_data* data);