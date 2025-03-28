/**
 * @file   ipc.h
 * @brief  Inter-process communications driver source file.
 * @author Rasheed Othman
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ipc.h"
#include "imu.h"

/* Private define ------------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

static void IpcRxCpltCallback(VIRT_UART_HandleTypeDef *huart);

/* Private variables ---------------------------------------------------------*/

static VIRT_UART_HandleTypeDef IpcUart;

static volatile FlagStatus IpcHandshake = RESET;
static volatile FlagStatus IpcMsgReceived = RESET;

static uint16_t IpcTxBufferSize = 0;
static uint16_t IpcRxBufferSize = 0;

static uint8_t IpcTxBuffer[RPMSG_BUFFER_SIZE] = {0};
static uint8_t IpcRxBuffer[RPMSG_BUFFER_SIZE] = {0};

/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

void IpcInit(void) {
  // Initialize the IPC driver
  if (VIRT_UART_Init(&IpcUart) != VIRT_UART_OK) {
    Error_Handler();
  }

  if (VIRT_UART_RegisterCallback(&IpcUart, VIRT_UART_RXCPLT_CB_ID, IpcRxCpltCallback) != VIRT_UART_OK) {
    Error_Handler();
  }

  log_info("IPC UART initialized\r\n");
}

void IpcService(void) {
  // Check for IPC message
  OPENAMP_check_for_message();

  // Message received
  if (IpcMsgReceived == SET) {
    IpcMsgReceived = RESET;

    // Check for initialization message
    if (memcmp(IpcRxBuffer, "OBDADS-IPC", 10) == 0) {
      IpcHandshake = SET;
      HAL_GPIO_WritePin(LED_ERR_GPIO_Port, LED_ERR_Pin, GPIO_PIN_RESET);
    }

    // Transmit the message back
    IpcWrite(IpcRxBuffer, IpcRxBufferSize);
  }
}

void IpcWrite(uint8_t *data, uint16_t size) {
  IpcTxBufferSize = size < sizeof(IpcTxBuffer) ? size : sizeof(IpcTxBuffer);
  memcpy(IpcTxBuffer, data, IpcTxBufferSize);

  // Transmit the message
  VIRT_UART_Transmit(&IpcUart, IpcTxBuffer, IpcTxBufferSize);
}

FlagStatus IpcGetHandshake(void) {
  return IpcHandshake;
}

/* Private functions ---------------------------------------------------------*/

static void IpcRxCpltCallback(VIRT_UART_HandleTypeDef *huart)
{
  log_info("IPC RX: %s\r\n", (char *) huart->pRxBuffPtr);
  IpcRxBufferSize = huart->RxXferSize < sizeof(IpcRxBuffer) ? huart->RxXferSize : (sizeof(IpcRxBuffer) - 1);
  memcpy(IpcRxBuffer, huart->pRxBuffPtr, IpcRxBufferSize);
  IpcMsgReceived = SET;
}

/********************************* END OF FILE ********************************/
