#ifndef _GPS_H_
#define _GPS_H_

#include <stdint.h>

//##################################################################################################################

typedef struct
{
	float				UTC_Time;
	float				Latitude;
	float				Longitude;
	uint8_t			PositionFixIndicator;
}GPGGA_t;

typedef struct 
{
	uint8_t		rxBuffer[512];
	uint16_t	rxIndex;
	// uint8_t		rxTmp;	
	// uint32_t	LastTime;	
	
	GPGGA_t		GPGGA;
	
}GPS_t;

extern GPS_t GPS;
//##################################################################################################################
void	GPS_CallBack(void);
void	GPS_Process(void);
//##################################################################################################################

#endif
