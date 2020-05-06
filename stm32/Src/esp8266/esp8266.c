#include "main.h"
#include "run.h"
#include <string.h>

extern UART_HandleTypeDef huart3;
#define UART_ESP8266 huart3

#define RETRY_TIMES 2

#define ESP8266_WAIT_NORMAL 2000
#define ESP8266_WAIT_NETWORK 5000

#define ESP8266_CMD_WIFI "AT+CWJAP_CUR=\"" WIFI_SSID "\",\"" WIFI_PASSWORD "\"\r\n"
#define ESP8266_CMD_CONNECT "AT+CIPSTART=\"TCP\",\"" HTTP_INFLUXDB_IP "\"," HTTP_INFLUXDB_PORT "\r\n"

#define ESP8266_BUF_SIZE 64
char buf[ESP8266_BUF_SIZE];
char cmd[ESP8266_BUF_SIZE];

char tcp_send_buf[TCP_BUF_SIZE];
uint32_t tcp_send_len;

uint32_t print_esp8266(char* cmd) {
    return HAL_UART_Transmit(&UART_ESP8266, (uint8_t*) cmd, strlen(cmd), 100);
}

uint32_t readln_esp8266(char* buf, uint32_t timeout) {
    if(HAL_OK != HAL_UART_Receive(&UART_ESP8266, (uint8_t*) &buf[0], 1, timeout)) return -1;
    uint8_t i = 1;
    while(HAL_OK == HAL_UART_Receive(&UART_ESP8266, (uint8_t*) &buf[i], 1, 10)) {
        i = (i + 1) % ESP8266_BUF_SIZE;
        if(buf[i-1] == '\n') {
            // End of line
            while(i > 0 && (buf[i-1] == '\n' || buf[i-1] == '\r')) i--;
            break;
        }
    }
    buf[i] = '\0';
    return i;
}

uint32_t send_and_check_response(char* command, uint32_t wait) {
    for(uint8_t retry = 0; retry < RETRY_TIMES; retry++) {
        // Clear buffer
        uint8_t ch;
        while(HAL_OK == HAL_UART_Receive(&UART_ESP8266, &ch, 1, 0));
        // Send command
        print_esp8266(command);
        while(-1 != readln_esp8266(buf, wait)) {
            if(0 == strcmp("OK", buf)) return HAL_OK;
            if(0 == strncmp("ERROR", buf, 5)) break;
        }
    }
    return HAL_ERROR;
}

uint32_t detect_esp8266() {
    return send_and_check_response("ATE0\r\n", ESP8266_WAIT_NORMAL);
}

uint32_t set_station_mode_esp8266() {
    return send_and_check_response("AT+CWMODE=1\r\n", ESP8266_WAIT_NORMAL);
}

uint32_t connect_wifi_esp8266() {
    return send_and_check_response(ESP8266_CMD_WIFI, ESP8266_WAIT_NETWORK);
}

uint32_t tcp_connect_esp8266() {
    return send_and_check_response(ESP8266_CMD_CONNECT, ESP8266_WAIT_NETWORK);
}

uint32_t tcp_prepare_send_esp8266(uint32_t len) {
    snprintf(cmd, ESP8266_BUF_SIZE, "AT+CIPSEND=%ld\r\n", len);
    return send_and_check_response(cmd, ESP8266_WAIT_NORMAL);
}

uint32_t tcp_send_esp8266(char* data, uint32_t len) {
    for(uint8_t retry = 0; retry < RETRY_TIMES; retry++) {
        HAL_UART_Transmit(&UART_ESP8266, (uint8_t*) data, len, 1000);
        while(-1 != readln_esp8266(buf, ESP8266_WAIT_NETWORK)) {
            if(0 == strcmp("SEND OK", buf)) return HAL_OK;
            if(0 == strncmp("ERROR", buf, 5)) break;
        }
    }
    return HAL_ERROR;
}

uint32_t tcp_close_esp8266() {
    return send_and_check_response("AT+CIPCLOSE\r\n", ESP8266_WAIT_NORMAL);
}

uint32_t deep_sleep_esp8266() {
    return send_and_check_response("AT+GSLP=2147483647\r\n", ESP8266_WAIT_NORMAL);
}

void setup_esp8266() {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);

    if(HAL_OK != detect_esp8266()) blink(3);
    if(HAL_OK != deep_sleep_esp8266()) blink(3);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
}

#ifdef ZJUWLAN_USERNAME
uint32_t zjuwlan_login() {
    char tcp_content_buf[] = "action=login&username=" ZJUWLAN_USERNAME "&password=" ZJUWLAN_PASSWORD "&ac_id=12&ajax=1";
    uint32_t tcp_content_len = sizeof(tcp_content_buf) - 1;
    
    char tcp_send_buf[TCP_BUF_SIZE];
    uint32_t tcp_send_len;

    const char template[] = 
        "POST /include/auth_action.php HTTP/1.1\r\n"
        "Host: 10.105.1.35:803\r\n"
        "User-Agent: zjui-ece/4.4.5\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Connection: close\r\n"
        "Content-Length: %lu\r\n"
        "\r\n"
        "%s"
    ;
    tcp_send_len = snprintf(
        tcp_send_buf,
        TCP_BUF_SIZE,
        template,
        tcp_content_len,
        tcp_content_buf
    );

    #define ESP8266_CMD_ZJUWLAN_LOGIN "AT+CIPSTART=\"TCP\",\"10.105.1.35\",803\r\n"

    uint32_t ret;
    if(HAL_OK != (ret = send_and_check_response(ESP8266_CMD_ZJUWLAN_LOGIN, ESP8266_WAIT_NETWORK))) return ret;
    if(HAL_OK != (ret = tcp_prepare_send_esp8266(tcp_send_len))) return ret;
    if(HAL_OK != (ret = tcp_send_esp8266(tcp_send_buf, tcp_send_len))) return ret;
    tcp_close_esp8266();
    return HAL_OK;
}
#endif

void loop_esp8266() {
    // Reset ESP8266 module
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);

    do {
        if(HAL_OK != detect_esp8266()) {
            blink(2);
            break;
        }
        if(HAL_OK != set_station_mode_esp8266()) {
            blink(3);
            break;
        }
        if(HAL_OK != connect_wifi_esp8266()) {
            blink(4);
            break;
        }

        #ifdef ZJUWLAN_USERNAME
        // ZJUWLAN login sequence
        if(HAL_OK != zjuwlan_login()) {
            blink(5);
            break;
        }
        #endif

        // InfluxDB upload sequence
        if(HAL_OK != tcp_connect_esp8266()) {
            blink(6);
            break;
        }

        do {
            if(HAL_OK != tcp_prepare_send_esp8266(tcp_send_len)) {
                blink(7);
                break;
            }
            if(HAL_OK != tcp_send_esp8266(tcp_send_buf, tcp_send_len)) {
                blink(8);
                break;
            }
        } while(0);

        tcp_close_esp8266();
    } while(0);

    if(HAL_OK != deep_sleep_esp8266()) {
        blink(9);
    }

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
}
