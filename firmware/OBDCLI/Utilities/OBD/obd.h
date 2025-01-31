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

#define OBD_RESET_ON_INIT
#define OBD_QUERY_PAYLOAD_SIZE 5U
#define OBD_RESET_DELAY_MS     3000U

/* Exported types ------------------------------------------------------------*/

typedef enum __OBD_ERROR_e {
  OBD_OK,                 // No error
  OBD_COMMAND_ERROR,      // Invalid OBD command or format
  OBD_UART_DMA_ERROR,     // UART DMA transaction failed
  OBD_UART_TX_ERROR,      // UART transmission failed
  OBD_UART_RX_ERROR,      // UART reception failed
  OBD_UART_INSTANCE_ERROR // Invalid OBD UART peripheral handler instance
} obd_error_t;

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/

extern buffer_t obd_rx_buffer;
extern buffer_t obd_tx_buffer;

/* Exported function prototypes ----------------------------------------------*/

obd_error_t obd_init(UART_HandleTypeDef *hobd, UART_HandleTypeDef *hcli);
obd_error_t obd_write(char *cmd, bool eol);
obd_error_t obd_write_dma(char *cmd, bool eol);
obd_error_t obd_listen_for_response(void);

#ifdef __cplusplus
}
#endif
#endif  /* OBD_H */

/********************************* END OF FILE ********************************/
