#pragma once

#include "bme680.h"
#include "main.h"

int32_t bme680_create_structure();
int32_t bme680_perform_measurement();
struct bme680_field_data bme680_get_measurements();