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
#include "stm32f4xx_hal.h"

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
#define BUTTON_BLUE_Pin GPIO_PIN_13
#define BUTTON_BLUE_GPIO_Port GPIOC
#define BUTTON_BLUE_EXTI_IRQn EXTI15_10_IRQn
#define LED_ERR_Pin GPIO_PIN_0
#define LED_ERR_GPIO_Port GPIOC
#define LED_DBG_Pin GPIO_PIN_1
#define LED_DBG_GPIO_Port GPIOC
#define VCP_TX_Pin GPIO_PIN_0
#define VCP_TX_GPIO_Port GPIOA
#define VCP_RX_Pin GPIO_PIN_1
#define VCP_RX_GPIO_Port GPIOA
#define STLINK_TX_Pin GPIO_PIN_2
#define STLINK_TX_GPIO_Port GPIOA
#define STLINK_RX_Pin GPIO_PIN_3
#define STLINK_RX_GPIO_Port GPIOA
#define LED_GREEN_Pin GPIO_PIN_5
#define LED_GREEN_GPIO_Port GPIOA
#define LED_RUN_Pin GPIO_PIN_0
#define LED_RUN_GPIO_Port GPIOB
#define IMU_INT2_Pin GPIO_PIN_10
#define IMU_INT2_GPIO_Port GPIOB
#define IMU_INT2_EXTI_IRQn EXTI15_10_IRQn
#define IMU_INT1_Pin GPIO_PIN_8
#define IMU_INT1_GPIO_Port GPIOA
#define IMU_INT1_EXTI_IRQn EXTI9_5_IRQn
#define OBD_RX_Pin GPIO_PIN_11
#define OBD_RX_GPIO_Port GPIOA
#define OBD_TX_Pin GPIO_PIN_12
#define OBD_TX_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define IMU_CLK_Pin GPIO_PIN_10
#define IMU_CLK_GPIO_Port GPIOC
#define IMU_MISO_Pin GPIO_PIN_11
#define IMU_MISO_GPIO_Port GPIOC
#define IMU_MOSI_Pin GPIO_PIN_12
#define IMU_MOSI_GPIO_Port GPIOC
#define IMU_NSS_Pin GPIO_PIN_2
#define IMU_NSS_GPIO_Port GPIOD
#define PWM_MS_Pin GPIO_PIN_8
#define PWM_MS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
