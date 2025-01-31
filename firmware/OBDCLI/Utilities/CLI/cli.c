/**
 * @file  cli.c
 * @brief CLI communications.
 */

/* Includes ------------------------------------------------------------------*/
#include "cli.h"
#include "obd.h"
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static UART_HandleTypeDef *OBD_UART = NULL;
static UART_HandleTypeDef *CLI_UART = NULL;

/* Public variables ---------------------------------------------------------*/

static uint8_t cli_tx_buffer[BUFFER_SIZE] = {0};
static uint8_t cli_rx_buffer[BUFFER_SIZE] = {0};

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Initialize the CLI UART interfaces.
 * @param  hobd: Reference to the OBD UART peripheral handler instance.
 * @param  hcli: Reference to the CLI UART peripheral handler instance.
 * @retval cli_error_t: CLI_OK                  - Indicates a successful operation.
 * @retval cli_error_t: CLI_UART_DMA_ERROR      - Indicates the UART DMA transaction failed.
 * @retval cli_error_t: CLI_UART_RX_ERROR       - Indicates the UART reception failed.
 * @retval cli_error_t: CLI_UART_INSTANCE_ERROR - Indicates the UART peripheral failed.
 */
cli_error_t cli_init(UART_HandleTypeDef *hobd, UART_HandleTypeDef *hcli) {
  if ((hobd == NULL) || (hcli == NULL)) {
    return CLI_UART_INSTANCE_ERROR;
  }

  OBD_UART = hobd;
  CLI_UART = hcli;

  // Start listening for requests from the CLI
  return cli_listen_for_response();
}

/**
 * @brief  Use UART to send a response to the CLI.
 * @param  data: Response data.
 * @param  size: Response size.
 * @retval cli_error_t: CLI_OK                  - Indicates a successful operation.
 * @retval cli_error_t: CLI_DATA_ERROR          - Indicates the data buffer doesn't exist.
 * @retval cli_error_t: CLI_UART_TX_ERROR       - Indicates the UART transmission failed.
 * @retval cli_error_t: CLI_UART_INSTANCE_ERROR - Indicates the UART peripheral failed.
 */
cli_error_t cli_write(uint8_t *data, uint16_t size) {
  if (CLI_UART == NULL) {
    return CLI_UART_INSTANCE_ERROR;
  }

  if ((data == NULL) || (size == 0)) {
    return CLI_DATA_ERROR;
  }

  // Copy the response into the CLI transmission buffer
  memcpy(cli_tx_buffer, data, size);

  // Transmit the response to the CLI
  if (HAL_UART_Transmit(CLI_UART, cli_tx_buffer, size, HAL_MAX_DELAY) != HAL_OK) {
    return CLI_UART_TX_ERROR;
  }

  return CLI_OK;
}

/**
 * @brief  Use UART DMA to send a response to the CLI.
 * @param  data: Response data.
 * @param  size: Response size.
 * @retval cli_error_t: CLI_OK                  - Indicates a successful operation.
 * @retval cli_error_t: CLI_DATA_ERROR          - Indicates the data buffer doesn't exist.
 * @retval cli_error_t: CLI_UART_DMA_ERROR      - Indicates the UART DMA transaction failed.
 * @retval cli_error_t: CLI_UART_TX_ERROR       - Indicates the UART transmission failed.
 * @retval cli_error_t: CLI_UART_INSTANCE_ERROR - Indicates the UART peripheral failed.
 */
cli_error_t cli_write_dma(uint8_t *data, uint16_t size) {
  if (CLI_UART == NULL) {
    return CLI_UART_INSTANCE_ERROR;
  }

  if ((data == NULL) || (size == 0)) {
    return CLI_DATA_ERROR;
  }

  // Copy the response into the CLI transmission buffer
  memcpy(cli_tx_buffer, data, size);

  // Transmit the response to the CLI
  if (HAL_UART_Transmit_DMA(CLI_UART, cli_tx_buffer, size) != HAL_OK) {
    return CLI_UART_TX_ERROR | CLI_UART_DMA_ERROR;
  }

  return CLI_OK;
}

/**
 * @brief  Start listening for a response from the CLI.
 * @retval cli_error_t: CLI_OK             - Indicates a successful operation.
 * @retval cli_error_t: CLI_UART_DMA_ERROR - Indicates the UART DMA transaction failed.
 * @retval cli_error_t: CLI_UART_RX_ERROR  - Indicates the UART reception failed.
 */
cli_error_t cli_listen_for_response(void) {
  // Start listening for responses from the CLI
  if (HAL_UARTEx_ReceiveToIdle_DMA(CLI_UART, cli_rx_buffer, BUFFER_SIZE) != HAL_OK) {
    return CLI_UART_RX_ERROR | CLI_UART_DMA_ERROR;
  }

  return CLI_OK;
}

/**
 * @brief Process the CLI request and send it to the OBD emulator.
 * @param size: Size of the received request segment.
 */
void cli_process_request(uint16_t size) {
  // Send the request directly to the emulator for now
  obd_error_t err = OBD_OK;
  for (int i = 0; i < RETRANSMIT_ATTEMPTS; i++) {
    err = obd_write(cli_rx_buffer, size);
    if (err == OBD_OK) {
      break;
    } else if (err == OBD_UART_TX_ERROR) {
      // Abort any transmission attempt since there was a failure
      HAL_UART_AbortTransmit(OBD_UART);
    }
  }
}

/* Private functions ---------------------------------------------------------*/

/********************************* END OF FILE ********************************/

