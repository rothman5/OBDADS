/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    sdmmc.c
  * @brief   This file provides code for the configuration
  *          of the SDMMC instances.
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
/* Includes ------------------------------------------------------------------*/
#include "sdmmc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

SD_HandleTypeDef hsd3;

/* SDMMC3 init function */

void MX_SDMMC3_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC3_Init 0 */

  /* USER CODE END SDMMC3_Init 0 */

  /* USER CODE BEGIN SDMMC3_Init 1 */

  /* USER CODE END SDMMC3_Init 1 */
  hsd3.Instance = SDMMC3;
  hsd3.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd3.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd3.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hsd3.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd3.Init.ClockDiv = 0;
  if (HAL_SD_Init(&hsd3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SDMMC3_Init 2 */

  /* USER CODE END SDMMC3_Init 2 */

}

void HAL_SD_MspInit(SD_HandleTypeDef* sdHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(sdHandle->Instance==SDMMC3)
  {
  /* USER CODE BEGIN SDMMC3_MspInit 0 */

  /* USER CODE END SDMMC3_MspInit 0 */
  if(IS_ENGINEERING_BOOT_MODE())
  {

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_SDMMC3;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

  }

    /* SDMMC3 clock enable */
    __HAL_RCC_SDMMC3_CLK_ENABLE();

    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**SDMMC3 GPIO Configuration
    PF1     ------> SDMMC3_CMD
    PG15     ------> SDMMC3_CK
    PF5     ------> SDMMC3_D2
    PF0     ------> SDMMC3_D0
    PF4     ------> SDMMC3_D1
    PD7     ------> SDMMC3_D3
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_5|GPIO_PIN_0|GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF9_SDIO3;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_SDIO3;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF10_SDIO3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* SDMMC3 interrupt Init */
    HAL_NVIC_SetPriority(SDMMC3_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SDMMC3_IRQn);
  /* USER CODE BEGIN SDMMC3_MspInit 1 */

  /* USER CODE END SDMMC3_MspInit 1 */
  }
}

void HAL_SD_MspDeInit(SD_HandleTypeDef* sdHandle)
{

  if(sdHandle->Instance==SDMMC3)
  {
  /* USER CODE BEGIN SDMMC3_MspDeInit 0 */

  /* USER CODE END SDMMC3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SDMMC3_CLK_DISABLE();

    /**SDMMC3 GPIO Configuration
    PF1     ------> SDMMC3_CMD
    PG15     ------> SDMMC3_CK
    PF5     ------> SDMMC3_D2
    PF0     ------> SDMMC3_D0
    PF4     ------> SDMMC3_D1
    PD7     ------> SDMMC3_D3
    */
    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_1|GPIO_PIN_5|GPIO_PIN_0|GPIO_PIN_4);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_7);

    /* SDMMC3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(SDMMC3_IRQn);
  /* USER CODE BEGIN SDMMC3_MspDeInit 1 */

  /* USER CODE END SDMMC3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
