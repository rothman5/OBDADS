/**
 * @file  cli.c
 * @brief CLI communications.
 */

/* Includes ------------------------------------------------------------------*/
#include "cli.h"
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static UART_HandleTypeDef *OBD_UART = NULL;
static UART_HandleTypeDef *CLI_UART = NULL;

/* Public variables ---------------------------------------------------------*/

buffer_t cli_rx_buffer = {0};
buffer_t cli_tx_buffer = {0};

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Initialize the CLI UART interfaces.
 * @param  hobd: Reference to the OBD UART peripheral handler instance.
 * @param  hcli: Reference to the OBD UART peripheral handler instance.
 * @retval obd_error_t: OBD_OK                  - Indicates a successful operation.
 * @retval obd_error_t: OBD_UART_INSTANCE_ERROR - Indicates the UART peripheral failed.
 */
cli_error_t cli_init(UART_HandleTypeDef *hobd, UART_HandleTypeDef *hcli) {
  if ((hobd == NULL) || (hcli == NULL)) {
    return CLI_UART_INSTANCE_ERROR;
  }

  OBD_UART = hobd;
  CLI_UART = hcli;

  return CLI_OK;
}

/**
 * @brief  Use UART to send a response to the CLI.
 * @param  rsp: Response data.
 * @param  len: Response length.
 * @retval cli_error_t: CLI_OK             - Indicates a successful operation.
 * @retval cli_error_t: CLI_UART_TX_ERROR  - Indicates the UART transmission failed.
 * @retval cli_error_t: CLI_RESPONSE_ERROR - Indicates the UART reception or emulator failed.
 */
cli_error_t cli_write(uint8_t *rsp, uint16_t len) {
  if ((rsp == NULL) || (len == 0))
  {
    return CLI_RESPONSE_ERROR;
  }

  // Copy the response into the CLI transmission buffer
  cli_tx_buffer.length = len;
  memcpy(cli_tx_buffer.data, rsp, len);

  if (HAL_UART_Transmit(CLI_UART, cli_tx_buffer.data, cli_tx_buffer.length, HAL_MAX_DELAY) != HAL_OK) {
    return CLI_UART_TX_ERROR;
  }

  return CLI_OK;
}

/**
 * @brief  Use UART DMA to send a response to the CLI.
 * @param  rsp: Response data.
 * @param  len: Response length.
 * @retval cli_error_t: CLI_OK             - Indicates a successful operation.
 * @retval cli_error_t: CLI_UART_TX_ERROR  - Indicates the UART transmission failed.
 * @retval cli_error_t: CLI_RESPONSE_ERROR - Indicates the UART reception or emulator failed.
 */
cli_error_t cli_write_dma(uint8_t *rsp, uint16_t len) {
  if ((rsp == NULL) || (len == 0))
  {
    return CLI_RESPONSE_ERROR;
  }

  // Copy the response into the CLI transmission buffer
  cli_tx_buffer.length = len;
  memcpy(cli_tx_buffer.data, rsp, len);

  if (HAL_UART_Transmit_DMA(CLI_UART, cli_tx_buffer.data, cli_tx_buffer.length) != HAL_OK) {
    return CLI_UART_TX_ERROR;
  }

  return CLI_OK;
}

/**
 * @brief  Start listening for a response from the emulator.
 * @retval cli_error_t: CLI_OK             - Indicates a successful operation.
 * @retval cli_error_t: CLI_UART_DMA_ERROR - Indicates the UART DMA transaction failed.
 * @retval cli_error_t: CLI_UART_RX_ERROR  - Indicates the UART reception failed.
 * @retval cli_error_t: CLI_UART_TX_ERROR  - Indicates the UART transmission failed.
 */
cli_error_t cli_listen_for_response(void) {
  // Start listening for responses from the emulator
  if (HAL_UARTEx_ReceiveToIdle_DMA(CLI_UART, cli_rx_buffer.data, BUFFER_SIZE) != HAL_OK) {
    return CLI_UART_RX_ERROR | CLI_UART_DMA_ERROR;
  }

  return CLI_OK;
}

/* Private functions ---------------------------------------------------------*/

/********************************* END OF FILE ********************************/

