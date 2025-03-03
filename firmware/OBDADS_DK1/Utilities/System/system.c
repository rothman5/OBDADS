/**
 * @file  system.c
 * @brief System state machine source file.
 */

/* Includes ------------------------------------------------------------------*/
#include "system.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Public variables ---------------------------------------------------------*/

volatile SysState_t SysState = SYS_IDLE;

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Initialize the system state machine's required peripherals and start the
 *         state machine execution timer.
 * @param  htim: Reference to the timer handler.
 * @return SysError_t
 */
SysError_t SysInit(TIM_HandleTypeDef *htim) {
  if (htim == NULL) {
    return SYS_ERR_TIMER;
  }
  
  // Initialize OBD driver
  // Initialize IMU driver
  // Initialize SD card driver

  // Start the state machine execution timer
  if (HAL_TIM_Base_Start_IT(htim) != HAL_OK) {
    return SYS_ERR_TIMER;
  }

  SysState = SYS_REQ_IMU;

  return SYS_ERR_OK;
}

/**
 * @brief  Execute the system state machine and configure the next state.
 * @note   Executed every 250ms.
 * @return SysError_t
 */
SysError_t SysExec(void) {
  HAL_GPIO_TogglePin(SME_GPIO_Port, SME_Pin);

  switch (SysState) {
    case SYS_REQ_IMU: {
      // Request IMU data using DMA
      break;
    }
    case SYS_REQ_OBD: {
      break;
    }
    case SYS_PENDING: {
      break;
    }
    case SYS_PROCESS: {
      break;
    }
    case SYS_FORWARD: {
      break;
    }
  }

  return SYS_ERR_OK;
}

/* Private functions ---------------------------------------------------------*/

/********************************* END OF FILE ********************************/
