#include <stdint.h>
#include <string.h>

#include "main.h"

extern UART_HandleTypeDef huart1;
#define PRINT_DEVICE huart1

int _write(int file, char *data, int len) {
    HAL_UART_Transmit(&huart1, (uint8_t*)data, len, 1000);
    return len;
}
