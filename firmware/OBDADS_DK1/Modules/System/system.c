/**
 * @file   system.c
 * @brief  System state machine source file.
 * @author Rasheed Othman
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "system.h"
#include "obd.h"
#include "usart.h"
#include "fdcan.h"

/* Private define ------------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static TIM_HandleTypeDef *SysTimer = NULL;

/* Public variables ----------------------------------------------------------*/

volatile bool SysError = false;
volatile SysState_t SysState = SYS_IDLE;

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Initializes the system state machine.
 * @param  htim TIM handle
 * @retval SysError_t
 */
SysError_t SysInit(TIM_HandleTypeDef *htim) {
  // Check if the TIM handle is valid
  if (htim == NULL) {
    return SYS_ERR_TIMER;
  }

  // Set the system timer handle
  SysTimer = htim;

  // Initialize the IMU driver
  // Initialize the OBD driver
  if (ObdInit(&hfdcan2) != OBD_OK) {
    return SYS_ERR_CAN;
  }
  // Initialize the SD driver
  // Initialize the IPC driver
  
  // if (HAL_TIM_Base_Start_IT(SysTimer) != HAL_OK) {
  //   return SYS_ERR_TIMER;
  // }

  // Set the system state to begin requesting IMU data after the timer expires
  SysState = SYS_REQ_IMU;

  HAL_GPIO_TogglePin(LED_DBG_GPIO_Port, LED_DBG_Pin);

  return SYS_OK;
}

/**
 * @brief  De-initializes the system state machine.
 * @retval SysError_t
 */
SysError_t SysDeInit(void) {
  // Stop the system timer
  if (HAL_TIM_Base_Stop_IT(SysTimer) != HAL_OK) {
    return SYS_ERR_TIMER;
  }

  // De-initialize the IMU driver
  // De-initialize the OBD driver
  if (ObdDeInit() != OBD_OK) {
    return SYS_ERR_CAN;
  }

  // De-initialize the SD driver
  // De-initialize the IPC driver
  return SYS_OK;
}

/**
 * @brief  Executes the system state machine based on the current state.
 * @retval SysError_t
 */
SysError_t SysExec(void) {
  if (SysError) {
    return SYS_ERR_UNKNOWN;
  }

  HAL_GPIO_TogglePin(LED_DBG_GPIO_Port, LED_DBG_Pin);

  static uint16_t MessageSize = 0u;
  static char Message[128] = {'\0'};

  switch (SysState) {
    case SYS_IDLE: {
      break;
    }

    case SYS_REQ_IMU: {
      // For now just skip to requesting OBD data
      SysState = SYS_REQ_OBD;
      break;
    }

    case SYS_REQ_OBD: {
      const ObdPidDesc_t *obdPids = ObdGetPidDescs();
      if (obdPids == NULL) {
        SysState = SYS_ERROR;
        break;
      }

      // Request OBD data for each PID
      for (uint8_t i = 0u; i < ObdGetNumPids(); i++) {
        // Check if the OBD request was successful
        if (ObdSend(i, obdPids[i].PID) != OBD_OK) {
          SysState = SYS_ERROR;
          break;
        }

        // Check if the OBD response was received
        if (ObdRecv(i) != OBD_OK) {
          SysState = SYS_ERROR;
        } else {
          // Set the system state to process the IMU and OBD data
          // TODO: add check to see if IMU data was received, if not set state to SYS_WAIT_IMU
          SysState = SYS_PROCESS;
        }
      }
      break;
    }

    case SYS_WAIT_IMU: {
      // TODO: Check if the IMU data was received and if not chill till bill, otherwise set state to SYS_PROCESS
      SysState = SYS_PROCESS;
      break;
    }

    case SYS_PROCESS: {
      for (uint8_t i = 0u; i < ObdGetNumPids(); i++) {
        memset(Message, 0x00u, sizeof(Message));

        uint8_t *obdReq = ObdGetReq(i);
        uint8_t *obdRsp = ObdGetRsp(i);

        const ObdPidDesc_t *reqPidDesc = ObdGetPidDesc(obdReq[2]);
        const ObdPidDesc_t *rspPidDesc = ObdGetPidDesc(obdRsp[2]);

        if ((rspPidDesc == NULL) || ((obdRsp[1] - obdReq[1]) != OBD_PID_MASK)) {
          MessageSize = snprintf(Message, sizeof(Message),
                                "Unknown PID response detected for requested PID (0x%02X): %s \r\n",
                                reqPidDesc->PID, reqPidDesc->name);
        } else {
          float value = rspPidDesc->processor(obdRsp, obdRsp[0]);
          MessageSize = snprintf(Message, sizeof(Message),
                                "PID (0x%02X): %.4f %s [ %s ]\r\n",
                                rspPidDesc->PID, value, rspPidDesc->unit, rspPidDesc->name);
        }

        HAL_UART_Transmit(&huart7, (uint8_t *) Message, MessageSize, HAL_MAX_DELAY);
        HAL_Delay(10u);
      }
      SysState = SYS_REQ_OBD;
      break;
    }

    case SYS_ERROR: {
      SysDeInit();
      SysError = true;
      break;
    }
  }
  return SYS_OK;
}

/**
 * @brief Initializes the DWT cycle counter for use with delay functions.
 */
void DwtInit(void) {
  // Enable core debug timers
  SET_BIT(CoreDebug->DEMCR, CoreDebug_DEMCR_TRCENA_Msk);

  // Reset cycle counter
  DWT->CYCCNT = 0;

  // Enable cycle counter
  SET_BIT(DWT->CTRL, DWT_CTRL_CYCCNTENA_Msk);
}

/**
 * @brief Delay for a specified number of milliseconds.
 * @note  This function is not accurate for very short delays since it uses the DWT cycle counter.
 * @param ms Number of milliseconds to delay.
 */
void DwtNoOpDelay(uint32_t ms) {
  uint32_t start = DWT->CYCCNT;
  uint32_t delay = ms * (HAL_RCC_GetSystemCoreClockFreq() / 1000u);
  while ((DWT->CYCCNT - start) < delay) {
    __NOP();
  }
}

/* Private functions ---------------------------------------------------------*/

/********************************* END OF FILE ********************************/
