/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * File Name          : resmgr_utility.c
  * Description        : This file provides code for the configuration
  *                      of the resmgr_utility instances.
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
#include "resmgr_utility.h"

/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/* Global variables ---------------------------------------------------------*/

/* USER CODE BEGIN 1 */
/* USER CODE END 1 */

/* RESMGR_UTILITY init function */
void MX_RESMGR_UTILITY_Init(void)
{
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  if (ResMgr_Init(NULL, NULL) != RESMGR_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN 3 */
  /* USER CODE END 3 */

}

/**
  * @}
  */
