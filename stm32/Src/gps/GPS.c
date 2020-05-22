#include "main.h"
#include "run.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Modified from https://github.com/nimaltd/GPS

extern UART_HandleTypeDef huart1;
#define	GPS_UART huart1

#define GPS_BUFFER_SIZE 2048
uint8_t rxBuffer[GPS_BUFFER_SIZE];
uint16_t rxIndex;

// float convertDegMinToDecDeg (float degMin) {
//   	return floorf(degMin / 100) + fmodf(degMin, 100) / 60;
// }

void GPS_Process(void) {
	// 30s timeout for GPS process
	extern volatile uint32_t uwTick;
	uint32_t timeout = uwTick + 30000;

	while(uwTick < timeout) {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_12);
		do {
			rxIndex=0;
			while((rxIndex < GPS_BUFFER_SIZE) && (HAL_OK == HAL_UART_Receive(&GPS_UART, &rxBuffer[rxIndex], 1, 50))) {
				rxIndex++;
			}
		} while(0);

		if(rxIndex > 0) {
			char* str = strstr((char*) rxBuffer,"$GNGGA,");
			if(str == NULL) continue;

			//                   1    1    2    2    3    3    4    4    5    5    6    6    7
			//         0    5    0    5    0    5    0    5    0    5    0    5    0    5    0
			// Format: $GNGGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh
			//         $GNGGA,074023.000,3005.84164,N,11955.87563,E,1,13,0.9,32.3,M,0.0,M,,*46

			// I don't have enough ROM space to use scanf with floating point
			// So I send the fields as is to InfluxDB, and let it parse them
			char latitude[GPS_FIELD_SIZE];
			char longitude[GPS_FIELD_SIZE];
			char positionFix;

			do {
				// Temp variables for parsing
				uint32_t len;
				char* next;

				// Field 1: GNGGA, ignore
				next = strstr(str, ",") + 1;
				len = next - str - 1;
				str = next;

				// Field 2: UTC time, ignore
				next = strstr(str, ",") + 1;
				len = next - str - 1;
				str = next;

				// Field 3: Latitude
				next = strstr(str, ",") + 1;
				len = next - str - 1;
				// offset by 1 to leave space for minus sign from next field
				strncpy(latitude + 1, str, len);
				latitude[len + 1] = '\0';
				str = next;

				// Field 4: Latitude N/S
				next = strstr(str, ",") + 1;
				// len = next - str - 1;
				latitude[0] = (str[0] == 'N') ? '0' : '-';
				str = next;

				// Field 5: Longitude
				next = strstr(str, ",") + 1;
				len = next - str - 1;
				// offset by 1 to leave space for minus sign from next field
				strncpy(longitude + 1, str, len);
				longitude[len + 1] = '\0';
				str = next;

				// Field 6: Longitude E/W
				next = strstr(str, ",") + 1;
				// len = next - str - 1;
				longitude[0] = (str[0] == 'E') ? '0' : '-';
				str = next;
				
				// Field 7: Fix status
				// next = strstr(str, ",") + 1;
				// len = next - str - 1;
				positionFix = str[0];
				// str = next;
			} while(0);

			// print(latitude);
			// print(", ");
			// print(longitude);
			// print(", ");
			// HAL_UART_Transmit(&huart1, (uint8_t*) &positionFix, 1, 1000);
			// print("\r\n");

			if((positionFix != '1') && (positionFix != '2')) continue;

			// TODO: Extra conversion needed
			// latitude = convertDegMinToDecDeg(latitude);
			// longitude = convertDegMinToDecDeg(longitude);

			// Done locating
			memcpy(measure_value.gps.latitude, latitude, GPS_FIELD_SIZE);
			memcpy(measure_value.gps.longitude, longitude, GPS_FIELD_SIZE);
			return;
		}
	}
}
