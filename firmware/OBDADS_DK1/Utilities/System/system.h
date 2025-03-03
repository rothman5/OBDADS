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
  SYS_INIT = 0u, // Initialize the acquisition system
  SYS_REQUEST,   // Request OBD and IMU data
  SYS_IDLE,      // Wait for requested data
  SYS_PROCESS,   // Process data once requests are received
  SYS_FORWARD    // Forward the processed data to the SD card and OS
} SystemState_t;

/**
 * @brief Error codes for the OBD auto-dynamics real-time acquisition system.
 */
typedef enum __SYSTEM_ERROR_e {
  SYS_ERROR_UNKNOWN = 0u, // An unknown error occured
  SYS_ERROR_SPI,          // A SPI error occured
  SYS_ERROR_SPI_TX,       // A SPI TX error occured
  SYS_ERROR_SPI_RX,       // A SPI RX error occured
  SYS_ERROR_SPI_DMA,      // A SPI DMA error occured
  SYS_ERROR_UART,         // A UART error occured
  SYS_ERROR_UART_TX,      // A UART TX error occured
  SYS_ERROR_UART_RX,      // A UART RX error occured
  SYS_ERROR_UART_DMA,     // A UART DMA error occured
  SYS_ERROR_TIMEOUT,      // Timed out waiting for requested data
  SYS_ERROR_TIMER,        // An error occured with the state machine execution timer
} SystemError_t;

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/

SystemError_t SystemInit(TIM_HandleTypeDef *htim);
SystemError_t SystemExec(void);

#ifdef __cplusplus
}
#endif
#endif  /* SYSTEM_H */

/********************************* END OF FILE ********************************/
