/**
 * @file   system.c
 * @brief  System state machine source file.
 * @author Rasheed Othman
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "usart.h"
#include "tim.h"
#include "spi.h"
#include "can.h"
#include "sys.h"
#include "imu.h"
#include "obd.h"

/* Private define ------------------------------------------------------------*/

#define SYS_INIT_LED_BLINK_COUNT 10u // Number of times to blink the LED during initialization

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

volatile FlagStatus ExecuteSystem = RESET;
volatile uint32_t ExecuteCounter = 0u;
volatile uint32_t ElapsedTimed_ms = 0u;

/* Public functions ----------------------------------------------------------*/

/**
 * @brief Initializes the system state machine.
 */
void SysInit(void) {
  // Initialize the IMU driver
  if (ImuInit(&hspi3) != IMU_OK) {
    Error_Handler();
  }

  // Initialize the OBD driver
  if (ObdInit(&hcan1) != OBD_OK) {
    Error_Handler();
  }

  // TODO: Initialize the SD driver

  if (HAL_TIM_Base_Start_IT(&htim10) != HAL_OK) {
    Error_Handler();
  }

  for (uint8_t i = 0u; i < SYS_INIT_LED_BLINK_COUNT; i++) {
    HAL_GPIO_TogglePin(LED_RUN_GPIO_Port, LED_RUN_Pin);
    HAL_Delay(50u);
    HAL_GPIO_TogglePin(LED_DBG_GPIO_Port, LED_DBG_Pin);
    HAL_Delay(50u);
    HAL_GPIO_TogglePin(LED_ERR_GPIO_Port, LED_ERR_Pin);
    HAL_Delay(50u);
  }
}

/**
 * @brief De-initializes the system state machine.
 */
void SysDeInit(void) {
  // TODO: De-initialize the IMU driver
  if (ImuDeInit() != IMU_OK) {
    Error_Handler();
  }

  // De-initialize the OBD driver
  if (ObdDeInit() != OBD_OK) {
    Error_Handler();
  }

  // TODO: De-initialize the SD driver

  if (HAL_TIM_Base_Stop_IT(&htim10) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief Executes the system.
 */
void SysExecute(void) {
  HAL_GPIO_TogglePin(LED_RUN_GPIO_Port, LED_RUN_Pin);

  if (SysRequest() != SYS_OK) {
    SysDeInit();
    Error_Handler();
  }

  if (SysProcess() != SYS_OK) {
    SysDeInit();
    Error_Handler();
  }

  if (SysForward() != SYS_OK) {
    SysDeInit();
    Error_Handler();
  }
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
  uint32_t delay = ms * (HAL_RCC_GetSysClockFreq() / 1000u);
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
  SysError_t err = SYS_OK;

  if (ImuReadXl() != IMU_OK) {
    err = SYS_ERR_SPI_TX | SYS_ERR_SPI_RX;
  }

  if (ImuReadGy() != IMU_OK) {
    err = SYS_ERR_SPI_TX | SYS_ERR_SPI_RX;
  }

  if (ImuReadTemp() != IMU_OK) {
    err = SYS_ERR_SPI_TX | SYS_ERR_SPI_RX;
  }

  return err;
}

/**
 * @brief  Requests OBD data.
 * @retval SysError_t
 */
static SysError_t SysRequestObd(void) {
  SysError_t err = SYS_OK;

  // Request OBD data for each PID
  const ObdService_0x01_PidDesc_t *obdPids = ObdGetPidDescs();
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
  Vec3_t *imuXl = ImuGetXlData();
  Vec3_t *imuGy = ImuGetGyData();
  float *imuTemp = ImuGetTemp();

  if ((imuXl == NULL) || (imuGy == NULL) || (imuTemp == NULL)) {
    return SYS_ERR_SPI_TX | SYS_ERR_SPI_RX;
  }

  SysCsvMsgSize += snprintf(&SysCsvMsg[SysCsvMsgSize], sizeof(SysCsvMsg), ", %.4f, %.4f, %.4f", imuXl->x, imuXl->y, imuXl->z);
  SysCsvMsgSize += snprintf(&SysCsvMsg[SysCsvMsgSize], sizeof(SysCsvMsg), ", %.4f, %.4f, %.4f", imuGy->x, imuGy->y, imuGy->z);
  SysCsvMsgSize += snprintf(&SysCsvMsg[SysCsvMsgSize], sizeof(SysCsvMsg), ", %.2f", *imuTemp);

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
    const ObdService_0x01_PidDesc_t *rspPidDesc = ObdGetPidDesc(obdRsp[OBD_PID_INDEX]);

    if ((rspPidDesc == NULL) || ((obdRsp[1] - obdReq[1]) != OBD_PID_MASK)) {
      // Unknown responses
      continue;
    }

    // Expected responses
    SysCsvMsgSize += snprintf(&SysCsvMsg[SysCsvMsgSize], sizeof(SysCsvMsg), ", %.2f", rspPidDesc->processor(obdRsp, obdRsp[0]));
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

  // Add the system timestamp to the message (ms since boot)
  SysCsvMsgSize += snprintf(SysCsvMsg, sizeof(SysCsvMsg), "%lu", ElapsedTimed_ms);
 
  err = SysProcessImu();
  if (err != SYS_OK) {
    return err;
  }

  err = SysProcessObd();
  if (err != SYS_OK) {
    return err;
  }

  // Add a newline to the message
  SysCsvMsg[SysCsvMsgSize++] = '\n';

  return err;
}

/**
 * @brief  Forwards the CSV message to the operating system.
 * @retval SysError_t
 */
static SysError_t SysForwardOs(void) {
  SysError_t err = SYS_OK;

  // Send message to OS
  if (HAL_UART_Transmit(&huart4, (uint8_t *) SysCsvMsg, SysCsvMsgSize, HAL_MAX_DELAY) != HAL_OK) {
    err = SYS_ERR_UART_TX | SYS_ERR_UART_DMA;
  }

  if (HAL_UART_Transmit(&huart2, (uint8_t *) SysCsvMsg, SysCsvMsgSize, HAL_MAX_DELAY) != HAL_OK) {
    err = SYS_ERR_UART_TX | SYS_ERR_UART_DMA;
  }

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

  return err;
}

/********************************* END OF FILE ********************************/
