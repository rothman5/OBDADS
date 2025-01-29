/**
 * @file  obd.c
 * @brief OBD communications.
 */

/* Includes ------------------------------------------------------------------*/
#include "obd.h"
#include "cli.h"
#include <stdio.h>
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static UART_HandleTypeDef *OBD_UART = NULL;
static UART_HandleTypeDef *CLI_UART = NULL;

static char *CMD_LUT[] = {
  "ATR",  // Resetting the emulator
  "ATZ",  // Initializing the emulator
  "010C", // Engine speed
  "010D", // Vehicle speed
};

/* Public variables ---------------------------------------------------------*/

buffer_t obd_rx_buffer = {0};
buffer_t obd_tx_buffer = {0};

/* Public functions ----------------------------------------------------------*/

/*
* SERVUCE(HEX BYTE) + PID(HEX BYTE) + \r
* 
* 010C\r -> engine speed
* 
* 010D\r -> vehicle speed
* SEARCHING...\r41 0d 00\r\r> (25 bytes)
* or
* 41 0d 00\r\r> (12 bytes)
* 
* 30 31 30 64 0D (ME)
* 53 45 41 52 43 48 49 4E 47 2E 2E 2E 0D
* 34 31 20 30 44 20 30 30 20 0D
* 0D
* 3E
* 0D (ME)
* 30 31 30 64 0D (ME)
* 34 31 20 30 44 20 30 30 20 0D
* 0D
* 3E
* 0D (ME)
* */

/**
 * @brief  Initialize the OBD emulator to listen for requests.
 * @param  hobd: Reference to the OBD UART peripheral handler instance.
 * @param  hcli: Reference to the OBD UART peripheral handler instance.
 * @retval obd_error_t: OBD_OK                  - Indicates a successful operation.
 * @retval obd_error_t: OBD_UART_DMA_ERROR      - Indicates the UART DMA transaction failed.
 * @retval obd_error_t: OBD_UART_RX_ERROR       - Indicates the UART reception failed.
 * @retval obd_error_t: OBD_UART_TX_ERROR       - Indicates the UART transmission failed.
 * @retval obd_error_t: OBD_UART_INSTANCE_ERROR - Indicates the UART peripheral failed.
 */
obd_error_t obd_init(UART_HandleTypeDef *hobd, UART_HandleTypeDef *hcli) {
  obd_error_t err = OBD_OK;

  if ((hobd == NULL) || (hcli == NULL)) {
    return (err = OBD_UART_INSTANCE_ERROR);
  }

  OBD_UART = hobd;
  CLI_UART = hcli;

  // Start listening for a response from the emulator.
  err = obd_listen_for_response();
  if (err != OBD_OK) {
    return err;
  }

// Reset the emulator if so desired (we need to decide if this is necessary)
#ifdef OBD_RESET_ON_INIT
  // Reset the emulator
  err = obd_write(CMD_LUT[0], true);
  if (err != OBD_OK) {
    return err;
  }

  // After some testing, the emulator takes ~3 seconds to restart
  HAL_Delay(OBD_RESET_DELAY_MS);
#endif /* OBD_RESET_ON_INIT */

  // Initialize the emulator
  err = obd_write(CMD_LUT[1], true);
  if (err != OBD_OK) {
    return err;
  }

  // Start listening for requests from the CLI
  if (cli_listen_for_response() != CLI_OK) {
    return OBD_UART_DMA_ERROR;
  }

  return err;
}

/**
 * @brief  Use UART to send request to the emulator.
 * @param  cmd: Command string.
 * @param  eol: Append end of line character.
 * @retval obd_error_t: OBD_OK            - Indicates a successful operation.
 * @retval obd_error_t: OBD_UART_TX_ERROR - Indicates the UART transmission failed.
 */
obd_error_t obd_write(char *cmd, bool eol) {
  if (eol) {
    // Append \r
    obd_tx_buffer.length = snprintf((char *) obd_tx_buffer.data, sizeof(obd_tx_buffer.data), "%s%c", cmd, EOL);
  } else {
    // Already has \r
    obd_tx_buffer.length = snprintf((char *) obd_tx_buffer.data, sizeof(obd_tx_buffer.data), "%s", cmd);
  }

  if (HAL_UART_Transmit(OBD_UART, obd_tx_buffer.data, obd_tx_buffer.length, HAL_MAX_DELAY) != HAL_OK) {
    return OBD_UART_TX_ERROR;
  }

  return OBD_OK;
}

/**
 * @brief  Use UART DMA to send request to the emulator.
 * @param  cmd: Command string.
 * @param  eol: Append end of line character.
 * @retval obd_error_t: OBD_OK            - Indicates a successful operation.
 * @retval obd_error_t: OBD_UART_TX_ERROR - Indicates the UART transmission failed.
 */
obd_error_t obd_write_dma(char *cmd, bool eol) {
  if (eol) {
    // Append \r
    obd_tx_buffer.length = snprintf((char *) obd_tx_buffer.data, sizeof(obd_tx_buffer.data), "%s%c", cmd, EOL);
  } else {
    // Already has \r
    obd_tx_buffer.length = snprintf((char *) obd_tx_buffer.data, sizeof(obd_tx_buffer.data), "%s", cmd);
  }

  if (HAL_UART_Transmit_DMA(OBD_UART, obd_tx_buffer.data, obd_tx_buffer.length) != HAL_OK) {
    return OBD_UART_TX_ERROR;
  }

  return OBD_OK;
}

/**
 * @brief  Start listening for a response from the emulator.
 * @retval obd_error_t: OBD_OK             - Indicates a successful operation.
 * @retval obd_error_t: OBD_UART_DMA_ERROR - Indicates the UART DMA transaction failed.
 * @retval obd_error_t: OBD_UART_RX_ERROR  - Indicates the UART reception failed.
 * @retval obd_error_t: OBD_UART_TX_ERROR  - Indicates the UART transmission failed.
 */
obd_error_t obd_listen_for_response(void) {
  // Start listening for responses from the emulator
  if (HAL_UARTEx_ReceiveToIdle_DMA(OBD_UART, obd_rx_buffer.data, BUFFER_SIZE) != HAL_OK) {
    return OBD_UART_RX_ERROR | OBD_UART_DMA_ERROR;
  }

  return OBD_OK;
}

/* Private functions ---------------------------------------------------------*/

/********************************* END OF FILE ********************************/
