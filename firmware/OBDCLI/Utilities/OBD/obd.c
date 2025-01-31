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

static uint16_t obd_decode_response(const uint8_t *rsp, uint16_t rsp_size, char *decoded, uint16_t decoded_size);

/* Private variables ---------------------------------------------------------*/

static UART_HandleTypeDef *OBD_UART = NULL;
static UART_HandleTypeDef *CLI_UART = NULL;

/* Public variables ---------------------------------------------------------*/

static uint8_t obd_tx_buffer[BUFFER_SIZE] = {0};
static uint8_t obd_rx_buffer[BUFFER_SIZE] = {0};

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Initialize the OBD emulator to listen for requests.
 * @param  hobd: Reference to the OBD UART peripheral handler instance.
 * @param  hcli: Reference to the CLI UART peripheral handler instance.
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
  err = obd_write((uint8_t *) "ATR\r", 4U);
  if (err != OBD_OK) {
    return err;
  }

  // After some testing, the emulator takes ~3 seconds to restart
  HAL_Delay(OBD_RESET_DELAY_MS);
#endif /* OBD_RESET_ON_INIT */

  // Initialize the emulator
  return obd_write((uint8_t *) "ATZ\r", 4U);
}

/**
 * @brief  Use UART to send a response to the OBD.
 * @param  data: Response data.
 * @param  size: Response size.
 * @retval obd_error_t: OBD_OK                  - Indicates a successful operation.
 * @retval obd_error_t: OBD_DATA_ERROR          - Indicates the data buffer doesn't exist.
 * @retval obd_error_t: OBD_UART_TX_ERROR       - Indicates the UART transmission failed.
 * @retval obd_error_t: OBD_UART_INSTANCE_ERROR - Indicates the UART peripheral failed.
 */
obd_error_t obd_write(uint8_t *data, uint16_t size) {
  if (OBD_UART == NULL) {
    return OBD_UART_INSTANCE_ERROR;
  }

  if ((data == NULL) || (size == 0)) {
    return OBD_DATA_ERROR;
  }

  // Copy the response into the OBD transmission buffer
  memcpy(obd_tx_buffer, data, size);

  // Transmit the response to the OBD
  if (HAL_UART_Transmit(OBD_UART, obd_tx_buffer, size, HAL_MAX_DELAY) != HAL_OK) {
    return OBD_UART_TX_ERROR;
  }

  return OBD_OK;
}

/**
 * @brief  Use UART DMA to send a response to the OBD.
 * @param  data: Response data.
 * @param  size: Response size.
 * @retval obd_error_t: OBD_OK                  - Indicates a successful operation.
 * @retval obd_error_t: OBD_DATA_ERROR          - Indicates the data buffer doesn't exist.
 * @retval obd_error_t: OBD_UART_DMA_ERROR      - Indicates the UART DMA transaction failed.
 * @retval obd_error_t: OBD_UART_TX_ERROR       - Indicates the UART transmission failed.
 * @retval obd_error_t: OBD_UART_INSTANCE_ERROR - Indicates the UART peripheral failed.
 */
obd_error_t obd_write_dma(uint8_t *data, uint16_t size) {
  if (OBD_UART == NULL) {
    return OBD_UART_INSTANCE_ERROR;
  }

  if ((data == NULL) || (size == 0)) {
    return OBD_DATA_ERROR;
  }

  // Copy the response into the OBD transmission buffer
  memcpy(obd_tx_buffer, data, size);

  // Transmit the response to the OBD
  if (HAL_UART_Transmit_DMA(OBD_UART, obd_tx_buffer, size) != HAL_OK) {
    return OBD_UART_TX_ERROR | OBD_UART_DMA_ERROR;
  }

  return OBD_OK;
}

/**
 * @brief  Start listening for a response from the emulator.
 * @retval obd_error_t: OBD_OK             - Indicates a successful operation.
 * @retval obd_error_t: OBD_UART_DMA_ERROR - Indicates the UART DMA transaction failed.
 * @retval obd_error_t: OBD_UART_RX_ERROR  - Indicates the UART reception failed.
 */
obd_error_t obd_listen_for_response(void) {
  // Start listening for responses from the emulator
  if (HAL_UARTEx_ReceiveToIdle_DMA(OBD_UART, obd_rx_buffer, BUFFER_SIZE) != HAL_OK) {
    return OBD_UART_RX_ERROR | OBD_UART_DMA_ERROR;
  }

  return OBD_OK;
}

/**
 * @brief Process the OBD response and send it to the CLI.
 * @param size: Size of the received response segment.
 */
void obd_process_response(uint16_t size) {
  // Accumulate response segments and decode until a response is detected, then send to CLI
  static uint16_t index = 0;
  static uint8_t accumulated[BUFFER_SIZE];

  // Check if the accumulation response buffer has enough space
  if ((index + size) > BUFFER_SIZE) {
    // Safe to clear since the entire buffer filled before decoding a response SOF
    index = 0;
    memset(accumulated, 0x00U, BUFFER_SIZE);
  }

  // Accumulate the new response segment
  memcpy(&accumulated[index], obd_rx_buffer, size);
  index += size;

  // Attempt to decode the current accumulated response
  char decoded[BUFFER_SIZE] = {'\0'};
  uint16_t decoded_size = obd_decode_response(accumulated, index, decoded, sizeof(decoded));
  if (decoded_size > 0) {
    // Successfully decoded a response, send to the CLI
    cli_error_t err = CLI_OK;
    for (int i = 0; i < RETRANSMIT_ATTEMPTS; i++) {
      err = cli_write((uint8_t *) decoded, decoded_size);
      if (err == CLI_OK) {
        // Successfully sent the response to the CLI, reset the accumulated response
        index = 0;
        memset(accumulated, 0x00U, BUFFER_SIZE);
        break;
      } else if (err == CLI_UART_TX_ERROR) {
        // Abort any transmission attempt since there was a failure
        HAL_UART_AbortTransmit(CLI_UART);
      }
    }
  }
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Check if a byte represents a valid OBD service.
 * @param  byte: The byte to check.
 * @retval bool: True if the byte is a valid OBD service, false otherwise.
 */
static inline bool is_valid_obd_service(uint8_t byte) {
  return ((ASCII_TO_HEX(byte) >= OBD_SERVICE_01) && (ASCII_TO_HEX(byte) <= OBD_SERVICE_10));
}

/**
 * @brief  Decode the OBD response and extract a valid sequence.
 * @param  rsp: Pointer to the response buffer.
 * @param  rsp_size: Size of the response buffer.
 * @param  decoded: Pointer to the decoded buffer.
 * @param  decoded_size: Size of the decoded buffer.
 * @retval uint16_t: Length of the decoded sequence, or 0 if no valid sequence is found.
 */
static uint16_t obd_decode_response(const uint8_t *rsp, uint16_t rsp_size, char *decoded, uint16_t decoded_size) {
  int si = -1; // Start index of the sequence
  int ei = -1; // End index of the sequence

  // Find the first sequence of RESPONSE_SOF followed by a valid OBD service
  for (int i = 0; i < rsp_size - 1; i++) {
    if (rsp[i] == RESPONSE_SOF && is_valid_obd_service(rsp[i + 1])) {
      // Found a RESPONSE_SOF followed by a valid OBD service, mark the starting index
      si = i;

      // Find the first sequence of RESPONSE_EOF
      for (int j = i + 1; j < rsp_size; j++) {
        if (rsp[j] == RESPONSE_EOF) {
          // Found a RESPONSE_EOF, mark the ending index
          ei = j;
          break;
        }
      }

      // Stop after finding the first valid sequence
      break;
    }
  }

  // Check if a valid sequence was found
  if ((si != -1) && (ei != -1)) {
    // Calculate the sequence length
    uint16_t sequence_length = ei - si + 1;

    // Ensure the decoded buffer is large enough to hold the sequence
    if (sequence_length > decoded_size) {
      return 0; // Decoded buffer is too small
    }

    // Copy the sequence into the decoded buffer
    memcpy(decoded, &rsp[si], sequence_length);

    // Ensure the string is null-terminated
    decoded[sequence_length] = '\0';

    // Return the length of the decoded sequence
    return sequence_length;
  }

  // No valid sequence found
  return 0;
}

/********************************* END OF FILE ********************************/
