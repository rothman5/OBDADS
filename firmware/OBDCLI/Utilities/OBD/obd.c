/**
 * @file  obd.c
 * @brief OBD communications interface.
 */

/* Includes ------------------------------------------------------------------*/
#include "obd.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static UART_HandleTypeDef *OBD_UART = NULL;
static UART_HandleTypeDef *CLI_UART = NULL;

/* Public variables ---------------------------------------------------------*/
obd_buffer_t obd_rx_buffer = {0};
obd_buffer_t obd_tx_buffer = {0};
obd_buffer_t cli_rx_buffer = {0};
obd_buffer_t cli_tx_buffer = {0};

/* Public functions ----------------------------------------------------------*/

/*
* SERVUCE(HEX BYTE) + PID(HEX BYTE) + \r
* 
* 010C\r -> engine speed
* 010D\r -> vehicle speed
* 
* */

/**
 * @brief  Initialize the OBD emulator to listen for requests.
 * @param  hobd: Reference to the OBD UART peripheral handler instance.
 * @param  hcli: Reference to the OBD UART peripheral handler instance.
 * @retval obd_error_t: OBD_OK                  - Indicates a successful operation.
 * @retval obd_error_t: OBD_UART_DMA_ERROR      - Indicates the UART DMA transaction failed.
 * @retval obd_error_t: OBD_UART_TX_ERROR       - Indicates the UART transmission failed.
 * @retval obd_error_t: OBD_UART_INSTANCE_ERROR - Indicates the UART peripheral failed.
 */
obd_error_t obd_init(UART_HandleTypeDef *hobd, UART_HandleTypeDef *hcli)
{
  obd_error_t err = OBD_OK;

  if (hobd == NULL)
  {
    return (err = OBD_UART_INSTANCE_ERROR);
  }

  if (hcli == NULL)
  {
    return (err = CLI_UART_INSTANCE_ERROR);
  }

  OBD_UART = hobd;
  CLI_UART = hcli;

  if (HAL_UARTEx_ReceiveToIdle_DMA(CLI_UART, cli_rx_buffer.data, OBD_BUFFER_SIZE) != HAL_OK)
  {
    return (err = OBD_UART_DMA_ERROR);
  }

  return err;
}

/* Private functions ---------------------------------------------------------*/

/********************************* END OF FILE ********************************/

