#include "GPS.h"
#include "main.h"
#include "run.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Modified from https://github.com/nimaltd/GPS

extern UART_HandleTypeDef huart3;
#define	_GPS_USART huart3
GPS_t GPS;

float convertDegMinToDecDeg (float degMin) {
  	return floorf(degMin / 100) + fmodf(degMin, 100) / 60;
}

void GPS_Process(void) {
	while(1) {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_12);
		do {
			int ret;
			GPS.rxIndex=0;
			while(HAL_TIMEOUT != (ret = HAL_UART_Receive(&_GPS_USART, &GPS.rxBuffer[GPS.rxIndex], 1, 50))) {
				GPS.rxIndex++;
			}
		} while(0);

		if(GPS.rxIndex>0) {
			char* str = strstr((char*) GPS.rxBuffer,"$GPGGA,");
			if(str == NULL) continue;

			//                   1    1    2    2    3    3    4    4    5    5    6    6    7
			//         0    5    0    5    0    5    0    5    0    5    0    5    0    5    0
			// Format: $GPGGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh
			//         $GPGGA,120558.916,5058.7457,N,00647.0514,E,2,06,1.7,109.0,M,47.6,M,1.5,0000*71
			char latitudeNS, longitudeEW, positionFix;
			print(str);
			sscanf(str,"$GPGGA,%f,%f,%c,%f,%c,%c,",
				&GPS.GPGGA.UTC_Time,
				&GPS.GPGGA.Latitude,
				&latitudeNS,
				&GPS.GPGGA.Longitude,
				&longitudeEW,
				&positionFix
			);
			if(positionFix == '0') continue;

			GPS.GPGGA.Latitude=convertDegMinToDecDeg(GPS.GPGGA.Latitude);
			GPS.GPGGA.Longitude=convertDegMinToDecDeg(GPS.GPGGA.Longitude);

			if(latitudeNS == 'S') GPS.GPGGA.Latitude = -GPS.GPGGA.Latitude;
			if(longitudeEW == 'W') GPS.GPGGA.Longitude = -GPS.GPGGA.Longitude;
				
			// Done locating
			return;
		}
	}
}
