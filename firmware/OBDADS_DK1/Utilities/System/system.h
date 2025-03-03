/**
 * @file  system.h
 * @brief System state machine header file.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SYSTEM_H
#define SYSTEM_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/**
 * @brief State machine states for the OBD auto-dynamics real-time acquisition system.
 */
typedef enum __SYSTEM_STATE_e {
  SYS_IDLE = 0u, // Initial state
  SYS_REQ_IMU,   // Request IMU data
  SYS_REQ_OBD,   // Request OBD data
  SYS_PENDING,   // Wait for requested data
  SYS_PROCESS,   // Process data once requests are received
  SYS_FORWARD,   // Forward the processed data to the SD card and OS
  SYS_ERROR      // An error occured during operation
} SysState_t;

/**
 * @brief Error codes for the OBD auto-dynamics real-time acquisition system.
 */
typedef enum __SYSTEM_ERROR_e {
  SYS_ERR_OK = 0u,  // No error    
  SYS_ERR_UNKNOWN,  // An unknown error occured
  SYS_ERR_SPI,      // A SPI error occured
  SYS_ERR_SPI_TX,   // A SPI TX error occured
  SYS_ERR_SPI_RX,   // A SPI RX error occured
  SYS_ERR_SPI_DMA,  // A SPI DMA error occured
  SYS_ERR_UART,     // A UART error occured
  SYS_ERR_UART_TX,  // A UART TX error occured
  SYS_ERR_UART_RX,  // A UART RX error occured
  SYS_ERR_UART_DMA, // A UART DMA error occured
  SYS_ERR_TIMEOUT,  // Timed out waiting for requested data
  SYS_ERR_TIMER,    // An error occured with the state machine execution timer
} SysError_t;

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/

extern volatile SysState_t SysState;

/* Exported function prototypes ----------------------------------------------*/

SysError_t SysInit(TIM_HandleTypeDef *htim);
SysError_t SysExec(void);

#ifdef __cplusplus
}
#endif
#endif  /* SYSTEM_H */

/********************************* END OF FILE ********************************/
