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

#define SYS_LOOP_DELAY_MS 100u // System loop delay in milliseconds
#define SYS_CSV_LINE_SIZE 512u // Maximum CSV line size

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

/* Exported macro ------------------------------------------------------------*/

#define ERR_CHK(_FUNC_, _ERR_) \
  do {                         \
    int _err = (_FUNC_);       \
    if (_err != (_ERR_)) {     \
      return _err;             \
    }                          \
  } while (0)                  \

/* Exported variables --------------------------------------------------------*/

/* Exported function prototypes ----------------------------------------------*/

SysError_t SysInit(void);
SysError_t SysDeInit(void);
SysError_t SysExecute(void);

void DwtInit(void);
void DwtNoOpDelay(uint32_t ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* SYSTEM_H */

/********************************* END OF FILE ********************************/
