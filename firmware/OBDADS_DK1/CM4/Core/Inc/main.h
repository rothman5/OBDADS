/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32mp1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define IMU_NSS_Pin GPIO_PIN_3
#define IMU_NSS_GPIO_Port GPIOF
#define LED_Pin GPIO_PIN_2
#define LED_GPIO_Port GPIOG
#define OBD_TX_Pin GPIO_PIN_10
#define OBD_TX_GPIO_Port GPIOB
#define SME_Pin GPIO_PIN_6
#define SME_GPIO_Port GPIOF
#define IMU_MOSI_Pin GPIO_PIN_9
#define IMU_MOSI_GPIO_Port GPIOF
#define IMU_CLK_Pin GPIO_PIN_7
#define IMU_CLK_GPIO_Port GPIOF
#define OBD_RX_Pin GPIO_PIN_12
#define OBD_RX_GPIO_Port GPIOB
#define IMU_MISO_Pin GPIO_PIN_8
#define IMU_MISO_GPIO_Port GPIOF

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
