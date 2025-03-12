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

static SysError_t SysRequestImu(void);
static SysError_t SysRequestObd(void);
static SysError_t SysRequest(void);
static SysError_t SysProcessImu(void);
static SysError_t SysProcessObd(void);
static SysError_t SysProcess(void);
static SysError_t SysForwardOs(void);
static SysError_t SysForwardSd(void);
static SysError_t SysForward(void);

/* Private variables ---------------------------------------------------------*/

static uint16_t SysCsvMsgSize = 0u;
static char SysCsvMsg[SYS_CSV_LINE_SIZE] = {'\0'};

/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Initializes the system state machine.
 * @retval SysError_t
 */
SysError_t SysInit(void) {
  // TODO: Initialize the IMU driver

  // Initialize the OBD driver
  if (ObdInit(&hfdcan2) != OBD_OK) {
    return SYS_ERR_CAN;
  }

  // TODO: Initialize the SD driver

  return SYS_OK;
}

/**
 * @brief  De-initializes the system state machine.
 * @retval SysError_t
 */
SysError_t SysDeInit(void) {
  // TODO: De-initialize the IMU driver

  // De-initialize the OBD driver
  if (ObdDeInit() != OBD_OK) {
    return SYS_ERR_CAN;
  }

  // TODO: De-initialize the SD driver
  return SYS_OK;
}

/**
 * @brief  Executes the system.
 * @retval SysError_t
 */
SysError_t SysExecute(void) {
  HAL_GPIO_TogglePin(LED_DBG_GPIO_Port, LED_DBG_Pin);

  SysError_t err = SYS_OK;

  err = SysRequest();
  if (err != SYS_OK) {
    SysDeInit();
    return err;
  }

  err = SysProcess();
  if (err != SYS_OK) {
    SysDeInit();
    return err;
  }

  err = SysForward();
  if (err != SYS_OK) {
    SysDeInit();
    return err;
  }

  return err;
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

/**
 * @brief  Requests IMU data.
 * @retval SysError_t
 */
static SysError_t SysRequestImu(void) {
  return SYS_OK;
}

/**
 * @brief  Requests OBD data.
 * @retval SysError_t
 */
static SysError_t SysRequestObd(void) {
  SysError_t err = SYS_OK;

  // Request OBD data for each PID
  const ObdPidDesc_t *obdPids = ObdGetPidDescs();
  for (uint8_t i = 0u; i < ObdGetNumPids(); i++) {
    // Send the OBD request
    if (ObdSend(i, obdPids[i].PID) != OBD_OK) {
      err = SYS_ERR_CAN_TX;
      break;
    }

    // Receive the OBD response
    if (ObdRecv(i) != OBD_OK) {
      err = SYS_ERR_CAN_RX;
      break;
    }
  }
  return err;
}

/**
 * @brief  Requests IMU and OBD data.
 * @retval SysError_t
 */
static SysError_t SysRequest(void) {
  SysError_t err = SYS_OK;

  err = SysRequestImu();
  if (err != SYS_OK) {
    return err;
  }

  return SysRequestObd();
}

/**
 * @brief  Processes IMU data.
 * @retval SysError_t
 */
static SysError_t SysProcessImu(void) {
  return SYS_OK;
}

/**
 * @brief  Processes OBD data.
 * @retval SysError_t
 */
static SysError_t SysProcessObd(void) {
  for (uint8_t i = 0u; i < ObdGetNumPids(); i++) {
    uint8_t *obdReq = ObdGetReq(i);
    uint8_t *obdRsp = ObdGetRsp(i);
    // const ObdPidDesc_t *reqPidDesc = ObdGetPidDesc(obdReq[OBD_PID_INDEX]);
    const ObdPidDesc_t *rspPidDesc = ObdGetPidDesc(obdRsp[OBD_PID_INDEX]);

    if ((rspPidDesc == NULL) || ((obdRsp[1] - obdReq[1]) != OBD_PID_MASK)) {
      // Unknown responses
      SysCsvMsgSize += snprintf(SysCsvMsg, sizeof(SysCsvMsg), ",");
    } else {
      // Expected responses
      SysCsvMsgSize += snprintf(SysCsvMsg, sizeof(SysCsvMsg), ", %.4f", rspPidDesc->processor(obdRsp, obdRsp[0]));
    }
  }

  return SYS_OK;
}

/**
 * @brief  Processes IMU and OBD data.
 * @retval SysError_t
 */
static SysError_t SysProcess(void) {
  SysError_t err = SYS_OK;

  // Reset the message size and buffer
  SysCsvMsgSize = 0u;
  memset(SysCsvMsg, '\0', sizeof(SysCsvMsg));
 
  err = SysProcessImu();
  if (err != SYS_OK) {
    return err;
  }

  err = SysProcessObd();
  if (err != SYS_OK) {
    return err;
  }

  // Add a newline to the message
  SysCsvMsgSize += snprintf(SysCsvMsg, sizeof(SysCsvMsg), "\r\n");

  return err;
}

/**
 * @brief  Forwards the CSV message to the operating system.
 * @retval SysError_t
 */
static SysError_t SysForwardOs(void) {
  SysError_t err = SYS_OK;

  // Send message to VCP
  // if (HAL_UART_Transmit_DMA(&huart4, (uint8_t *) SysCsvMsg, SysCsvMsgSize) != HAL_OK) {
  //   err = SYS_ERR_UART_TX | SYS_ERR_UART_DMA;
  // }

  return err;
}

/**
 * @brief  Forwards the CSV message to the SD card.
 * @retval SysError_t
 */
static SysError_t SysForwardSd(void) {
  // TODO: Open CSV file on SD card
  // TODO: Check if SD card would become full (pretend circular buffer)
  // TODO: Write CSV message to SD card
  return SYS_OK;
}

/**
 * @brief  Forwards the CSV message to the operating system and SD card.
 * @retval SysError_t
 */
static SysError_t SysForward(void) {
  SysError_t err = SYS_OK;

  err = SysForwardOs();
  if (err != SYS_OK) {
    return err;
  }

  err = SysForwardSd();
  if (err != SYS_OK) {
    return err;
  }

  // Send message to VCP
  if (HAL_UART_Transmit_DMA(&huart7, (uint8_t *) SysCsvMsg, SysCsvMsgSize) != HAL_OK) {
    err = SYS_ERR_UART_TX | SYS_ERR_UART_DMA;
  }

  return err;
}

/********************************* END OF FILE ********************************/
