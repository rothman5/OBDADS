/**
 * @file  obd.h
 * @brief OBD communications.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef OBD_H
#define OBD_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "tools.h"

/* Exported defines ----------------------------------------------------------*/

// Enable OBD emulator reset during initialization
#define OBD_RESET_ON_INIT
#define OBD_RESET_DELAY_MS 3000U

/* Exported types ------------------------------------------------------------*/

typedef enum __OBD_ERROR_e {
  OBD_OK,                 // No error
  OBD_COMMAND_ERROR,      // Invalid OBD command or format
  OBD_DATA_ERROR,         // Invalid data buffer
  OBD_UART_DMA_ERROR,     // UART DMA transaction failed
  OBD_UART_TX_ERROR,      // UART transmission failed
  OBD_UART_RX_ERROR,      // UART reception failed
  OBD_UART_INSTANCE_ERROR // Invalid OBD UART peripheral handler instance
} obd_error_t;

// https://en.wikipedia.org/wiki/OBD-II_PIDs
typedef enum __OBD_SERVICE_e {
  OBD_SERVICE_01 = 0x01, // Show current data
  OBD_SERVICE_02 = 0x02, // Show freeze frame data
  OBD_SERVICE_03 = 0x03, // Show stored Diagnostic Trouble Codes
  OBD_SERVICE_04 = 0x04, // Clear Diagnostic Trouble Codes and stored values
  OBD_SERVICE_05 = 0x05, // Test results, oxygen sensor monitoring (non CAN only)
  OBD_SERVICE_06 = 0x06, // Test results, other component/system monitoring (Test results, oxygen sensor monitoring for CAN only)
  OBD_SERVICE_07 = 0x07, // Show pending Diagnostic Trouble Codes (detected during current or last driving cycle)
  OBD_SERVICE_08 = 0x08, // Control operation of on-board component/system
  OBD_SERVICE_09 = 0x09, // Request vehicle information
  OBD_SERVICE_10 = 0x0A  // Permanent Diagnostic Trouble Codes (DTCs) (Cleared DTCs)
} obd_service_t;

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/

obd_error_t obd_init(UART_HandleTypeDef *hobd, UART_HandleTypeDef *hcli);
obd_error_t obd_write(uint8_t *data, uint16_t size);
obd_error_t obd_write_dma(uint8_t *data, uint16_t size);
obd_error_t obd_listen_for_response(void);
void obd_process_response(uint16_t size);

#ifdef __cplusplus
}
#endif
#endif  /* OBD_H */

/********************************* END OF FILE ********************************/
