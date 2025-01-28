/**
 * @file  obd.h
 * @brief OBD communications interface.
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

/* Exported defines ----------------------------------------------------------*/

#define OBD_BUFFER_SIZE        48U
#define OBD_QUERY_PAYLOAD_SIZE 5U

/* Exported types ------------------------------------------------------------*/

typedef enum __OBD_ERROR_e
{
  OBD_OK,                  // No error
  OBD_COMMAND_ERROR,       // Invalid OBD command or format
  OBD_UART_DMA_ERROR,      // UART DMA transaction failed
  OBD_UART_TX_ERROR,       // UART transmission failed
  OBD_UART_RX_ERROR,       // UART reception failed
  OBD_UART_INSTANCE_ERROR, // Invalid OBD UART peripheral handler instance
  CLI_UART_INSTANCE_ERROR  // Invalid CLI UART peripheral handler instance
} obd_error_t;

typedef struct __OBD_BUFFER_s
{
  uint16_t length;
  uint8_t data[OBD_BUFFER_SIZE];
} obd_buffer_t;

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/

extern obd_buffer_t obd_rx_buffer;
extern obd_buffer_t obd_tx_buffer;
extern obd_buffer_t cli_rx_buffer;
extern obd_buffer_t cli_tx_buffer;

/* Exported function prototypes ----------------------------------------------*/

obd_error_t obd_init(UART_HandleTypeDef *hobd, UART_HandleTypeDef *hcli);

#ifdef __cplusplus
}
#endif
#endif  /* OBD_H */

/********************************* END OF FILE ********************************/
