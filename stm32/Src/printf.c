#include <errno.h>
#include <sys/unistd.h> // STDOUT_FILENO, STDERR_FILENO
#include <stdint.h>
#include <string.h>

#include "main.h"

extern UART_HandleTypeDef huart1;
#define PRINT_DEVICE huart1

int _write(int file, char *data, int len) {
    if ((file != STDOUT_FILENO) && (file != STDERR_FILENO)) {
        errno = EBADF;
        return -1;
    }

    HAL_StatusTypeDef status = HAL_UART_Transmit(&huart1, (uint8_t*)data, len, 1000);
    return (status == HAL_OK ? len : 0);
}

void print(char* str) {
    HAL_UART_Transmit(&huart1, (uint8_t*) str, strlen(str), 1000);
}
