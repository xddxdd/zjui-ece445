/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_cortex.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_pwr.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EN_MICS6814_Pin GPIO_PIN_1
#define EN_MICS6814_GPIO_Port GPIOA
#define USART2_TX_PMS5003_Pin GPIO_PIN_2
#define USART2_TX_PMS5003_GPIO_Port GPIOA
#define USART2_RX_PMS5003_Pin GPIO_PIN_3
#define USART2_RX_PMS5003_GPIO_Port GPIOA
#define ADC1_IN4_MICS6814_CO_Pin GPIO_PIN_4
#define ADC1_IN4_MICS6814_CO_GPIO_Port GPIOA
#define ADC1_IN4_MICS6814_NH3_Pin GPIO_PIN_5
#define ADC1_IN4_MICS6814_NH3_GPIO_Port GPIOA
#define ADC1_IN4_MICS6814_NO2_Pin GPIO_PIN_6
#define ADC1_IN4_MICS6814_NO2_GPIO_Port GPIOA
#define USART3_TX_ESP8266_Pin GPIO_PIN_10
#define USART3_TX_ESP8266_GPIO_Port GPIOB
#define USART3_RX_ESP8266_Pin GPIO_PIN_11
#define USART3_RX_ESP8266_GPIO_Port GPIOB
#define EN_LED_Pin GPIO_PIN_12
#define EN_LED_GPIO_Port GPIOB
#define EN_GPS_Pin GPIO_PIN_14
#define EN_GPS_GPIO_Port GPIOB
#define EN_PMS5003_Pin GPIO_PIN_15
#define EN_PMS5003_GPIO_Port GPIOB
#define EN_ESP8266_Pin GPIO_PIN_8
#define EN_ESP8266_GPIO_Port GPIOA
#define USART1_TX_GPS_Pin GPIO_PIN_9
#define USART1_TX_GPS_GPIO_Port GPIOA
#define USART1_RX_GPS_Pin GPIO_PIN_10
#define USART1_RX_GPS_GPIO_Port GPIOA
#define TOGGLE_BAT_Pin GPIO_PIN_5
#define TOGGLE_BAT_GPIO_Port GPIOB
#define I2C1_SCL_BME680_Pin GPIO_PIN_6
#define I2C1_SCL_BME680_GPIO_Port GPIOB
#define I2C1_SDA_BME680_Pin GPIO_PIN_7
#define I2C1_SDA_BME680_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
