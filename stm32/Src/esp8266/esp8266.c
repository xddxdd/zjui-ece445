#include "main.h"
#include "run.h"
#include <string.h>

extern UART_HandleTypeDef huart3;
#define UART_ESP8266 huart3

#define RETRY_TIMES 3

char buf[256];
char cmd[256];

char tcp_send_buf[1024];
uint32_t tcp_send_len;

void clear_buffer_esp8266() {
    uint8_t ch;
    while(HAL_OK == HAL_UART_Receive(&UART_ESP8266, &ch, 1, 0));
}

uint32_t print_esp8266(char* cmd) {
    return HAL_UART_Transmit(&UART_ESP8266, (uint8_t*) cmd, strlen(cmd), 100);
}

uint32_t readln_esp8266(char* buf, uint32_t timeout) {
    if(HAL_OK != HAL_UART_Receive(&UART_ESP8266, (uint8_t*) &buf[0], 1, timeout)) return -1;
    uint8_t i = 1;
    while(HAL_OK == HAL_UART_Receive(&UART_ESP8266, (uint8_t*) &buf[i], 1, 10)) {
        i++;
        if(buf[i-1] == '\n') {
            // End of line
            while(i > 0 && (buf[i-1] == '\n' || buf[i-1] == '\r')) i--;
            break;
        }
    }
    buf[i] = '\0';
    return i;
}

uint32_t detect_esp8266() {
    clear_buffer_esp8266();

    for(uint8_t retry = 0; retry < RETRY_TIMES; retry++) {
        print_esp8266("ATE0\r\n");
        while(-1 != readln_esp8266(buf, 2000)) {
            if(0 == strcmp("OK", buf)) return HAL_OK;
        }
    }
    return HAL_ERROR;
}

uint32_t set_station_mode_esp8266() {
    for(uint8_t retry = 0; retry < RETRY_TIMES; retry++) {
        print_esp8266("AT+CWMODE=1\r\n");
        while(-1 != readln_esp8266(buf, 2000)) {
            if(0 == strcmp("OK", buf)) return HAL_OK;
        }
    }
    return HAL_ERROR;
}

uint32_t connect_wifi_esp8266(char* ssid, char* password) {
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);

    for(uint8_t retry = 0; retry < RETRY_TIMES; retry++) {
        print_esp8266(cmd);

        uint8_t recv_ok = 0, recv_wifi_connected = 0, recv_wifi_got_ip = 0;
        while(-1 != readln_esp8266(buf, 5000)) {
            if(0 == strcmp("OK", buf)) recv_ok = 1;
            if(0 == strcmp("WIFI CONNECTED", buf)) recv_wifi_connected = 1;
            if(0 == strcmp("WIFI GOT IP", buf)) recv_wifi_got_ip = 1;
            if(recv_ok && recv_wifi_connected && recv_wifi_got_ip) return HAL_OK;
        }
    }
    return HAL_ERROR;
}

uint32_t tcp_connect_esp8266(char* addr, uint32_t port) {
    sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%lu\r\n", addr, port);
    for(uint8_t retry = 0; retry < RETRY_TIMES; retry++) {
        print_esp8266(cmd);
        while(-1 != readln_esp8266(buf, 5000)) {
            if(0 == strcmp("OK", buf)) return HAL_OK;
            if(0 == strncmp("ERROR", buf, 5)) break;
        }
    }
    return HAL_ERROR;
}

uint32_t tcp_prepare_send_esp8266(uint32_t len) {
    sprintf(cmd, "AT+CIPSEND=%ld\r\n", len);
    for(uint8_t retry = 0; retry < RETRY_TIMES; retry++) {
        print_esp8266(cmd);
        while(-1 != readln_esp8266(buf, 2000)) {
            if(0 == strcmp("OK", buf)) return HAL_OK;
            if(0 == strncmp("ERROR", buf, 5)) break;
        }
    }
    return HAL_ERROR;
}

uint32_t tcp_send_esp8266(char* data, uint32_t len) {
    for(uint8_t retry = 0; retry < RETRY_TIMES; retry++) {
        HAL_UART_Transmit(&UART_ESP8266, (uint8_t*) data, len, 1000);
        while(-1 != readln_esp8266(buf, 5000)) {
            if(0 == strcmp("SEND OK", buf)) return HAL_OK;
            if(0 == strncmp("ERROR", buf, 5)) break;
        }
    }
    return HAL_ERROR;
}

uint32_t tcp_close_esp8266() {
    for(uint8_t retry = 0; retry < RETRY_TIMES; retry++) {
        print_esp8266("AT+CIPCLOSE\r\n");
        while(-1 != readln_esp8266(buf, 5000)) {
            if(0 == strcmp("OK", buf)) return HAL_OK;
            if(0 == strncmp("ERROR", buf, 5)) break;
        }
    }
    return HAL_ERROR;
}

void loop_esp8266() {
    // Enable ESP8266
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);

    do {
        if(HAL_OK != detect_esp8266()) {
            printf("ESP8266 not detected\r\n");
            break;
        }
        if(HAL_OK != set_station_mode_esp8266()) {
            printf("ESP8266 cannot enter station mode\r\n");
            break;
        }
        if(HAL_OK != connect_wifi_esp8266("xqm2", "98.01.20")) {
            printf("ESP8266 cannot connect to wifi\r\n");
            break;
        }
        if(HAL_OK != tcp_connect_esp8266("192.168.0.254", 8000)) {
            printf("ESP8266 cannot create TCP connection\r\n");
            break;
        }

        do {
            if(HAL_OK != tcp_prepare_send_esp8266(tcp_send_len)) {
                printf("ESP8266 cannot begin send data\r\n");
                break;
            }
            if(HAL_OK != tcp_send_esp8266(tcp_send_buf, tcp_send_len)) {
                printf("ESP8266 cannot send data\r\n");
                break;
            }
        } while(0);

        if(HAL_OK != tcp_close_esp8266()) {
            printf("ESP8266 cannot close connection\r\n");
            break;
        }
    } while(0);

    // Disable ESP8266 module
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    printf("ESP8266 off\r\n");
}
