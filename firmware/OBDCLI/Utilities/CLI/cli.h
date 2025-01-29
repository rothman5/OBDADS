/**
 * @file  cli.h
 * @brief CLI communications.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CLI_H
#define CLI_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "tools.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef enum __CLI_ERROR_e {
  CLI_OK,                 // No error
  CLI_COMMAND_ERROR,      // Invalid CLI command or format
  CLI_RESPONSE_ERROR,     // Invalid request response
  CLI_UART_DMA_ERROR,     // UART DMA transaction failed
  CLI_UART_TX_ERROR,      // UART transmission failed
  CLI_UART_RX_ERROR,      // UART reception failed
  CLI_UART_INSTANCE_ERROR // Invalid CLI UART peripheral handler instance
} cli_error_t;

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/

extern buffer_t cli_rx_buffer;
extern buffer_t cli_tx_buffer;

/* Exported function prototypes ----------------------------------------------*/

cli_error_t cli_init(UART_HandleTypeDef *hobd, UART_HandleTypeDef *hcli);
cli_error_t cli_write(uint8_t *rsp, uint16_t len);
cli_error_t cli_write_dma(uint8_t *rsp, uint16_t len);
cli_error_t cli_listen_for_response(void);

#ifdef __cplusplus
}
#endif
#endif  /* CLI_H */

/********************************* END OF FILE ********************************/
