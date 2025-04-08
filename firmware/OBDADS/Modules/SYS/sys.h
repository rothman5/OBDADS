/**
 * @file   sys.h
 * @brief  System state machine header file.
 * @author Rasheed Othman
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SYS_H
#define SYS_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Exported defines ----------------------------------------------------------*/

#define SYS_LOOP_DELAY_MS  50u   // System loop delay in milliseconds
#define SYS_ERROR_DELAY_MS 250u // Error delay in milliseconds
#define SYS_CSV_LINE_SIZE  1024u // Maximum CSV line size

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
  SYS_ERR_TIMER    = (1u << 13), // An error occured with the state machine execution timer
  SYS_ERR_PAUSED   = (1u << 14)  // The system is paused (timer has not elapsed)
} SysError_t;

/* Exported macro ------------------------------------------------------------*/

/**
 * @brief Convert two bytes to a 16-bit unsigned integer.
 * @param __MSB__ Most significant byte
 * @param __LSB__ Least significant byte
 */
#define BYTES_TO_U16(__MSB__, __LSB__) (uint16_t) ((__MSB__ << 8u) | (__LSB__ & 0xFFu))

/**
 * @brief Convert two bytes to a 16-bit signed integer.
 * @param __MSB__ Most significant byte
 * @param __LSB__ Least significant byte
 */
#define BYTES_TO_S16(__MSB__, __LSB__) (int16_t) (((int16_t) __MSB__ << 8u) | (__LSB__ & 0xFFu))

/**
 * @brief Check for an error and return it if one is found.
 * @param _FUNC_ Function to check for an error
 * @param _ERR_  Error to check for
 */
#define ERR_CHK(_FUNC_, _ERR_) \
  do {                         \
    int _err = (_FUNC_);       \
    if (_err != (_ERR_)) {     \
      return _err;             \
    }                          \
  } while (0)                  \

/* Exported variables --------------------------------------------------------*/

extern volatile FlagStatus ExecuteSystem;
extern volatile uint32_t ExecuteCounter;
extern volatile uint32_t ElapsedTimed_ms;

/* Exported function prototypes ----------------------------------------------*/

void SysInit(void);
void SysDeInit(void);
void SysExecute(void);

void DwtInit(void);
void DwtNoOpDelay(uint32_t ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* SYS_H */

/********************************* END OF FILE ********************************/
