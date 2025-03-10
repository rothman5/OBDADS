/**
 * @file   system.h
 * @brief  System state machine header file.
 * @author Rasheed Othman
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SYSTEM_H
#define SYSTEM_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "tim.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/**
 * @brief Error codes for the OBD auto-dynamics real-time acquisition system.
 * @note  Each error code is a bit mask that can be combined with other error codes.
 */
typedef enum __SYSTEM_ERROR_e {
  SYS_OK           = 0u,         // No error    
  SYS_ERR_UNKNOWN  = (1u << 0),  // An unknown error occured
  SYS_ERR_SPI      = (1u << 1),  // A SPI error occured
  SYS_ERR_SPI_TX   = (1u << 2),  // A SPI TX error occured
  SYS_ERR_SPI_RX   = (1u << 3),  // A SPI RX error occured
  SYS_ERR_SPI_DMA  = (1u << 4),  // A SPI DMA error occured
  SYS_ERR_UART     = (1u << 5),  // A UART error occured
  SYS_ERR_UART_TX  = (1u << 6),  // A UART TX error occured
  SYS_ERR_UART_RX  = (1u << 7),  // A UART RX error occured
  SYS_ERR_UART_DMA = (1u << 8),  // A UART DMA error occured
  SYS_ERR_CAN      = (1u << 9),  // A CAN error occured
  SYS_ERR_CAN_TX   = (1u << 10), // A CAN TX error occured
  SYS_ERR_CAN_RX   = (1u << 11), // A CAN RX error occured
  SYS_ERR_TIMEOUT  = (1u << 12), // Timed out waiting for requested data
  SYS_ERR_TIMER    = (1u << 13)  // An error occured with the state machine execution timer
} SysError_t;

/**
 * @brief State machine states for the OBD auto-dynamics real-time acquisition system.
 */
typedef enum __SYSTEM_STATE_e {
  SYS_IDLE = 0u, // Initial state
  SYS_REQ_IMU,   // Request IMU data
  SYS_REQ_OBD,   // Request OBD data
  SYS_WAIT_IMU,  // Wait for requested IMU data
  SYS_PROCESS,   // Process data once requests are received
  SYS_ERROR      // An error occured during operation
} SysState_t;

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/

/**
 * @brief Tracks if an error occured during the operation of the system state machine.
 */
extern volatile bool SysError;

/**
 * @brief Tracks the current state of the system state machine.
 */
extern volatile SysState_t SysState;

/* Exported function prototypes ----------------------------------------------*/

SysError_t SysInit(TIM_HandleTypeDef *htim);
SysError_t SysDeInit(void);
SysError_t SysExec(void);

void DwtInit(void);
void DwtNoOpDelay(uint32_t ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* SYSTEM_H */

/********************************* END OF FILE ********************************/
