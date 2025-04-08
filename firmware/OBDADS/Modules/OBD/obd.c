/**
 * @file   obd.c
 * @brief  OBD communications driver source file.
 * @author Rasheed Othman
 */

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <string.h>
#include "obd.h"

/* Private define ------------------------------------------------------------*/

#define OBD_CAN_REQ_ID   0x07DFu
#define OBD_CAN_RSP_ID_1 0x07E8u
#define OBD_CAN_RSP_ID_2 0x07EFu
#define OBD_DUMMY_BYTE   0xCCu

/* Private types -------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

static bool IsByteInEnum(uint8_t byte);
static float EngLoadProcessor(const uint8_t *rsp, uint8_t size);
static float EndSpeedProcessor(const uint8_t *rsp, uint8_t size);
static float VehSpeedProcessor(const uint8_t *rsp, uint8_t size);
static float ThrottleProcessor(const uint8_t *rsp, uint8_t size);
static float DistMilProcessor(const uint8_t *rsp, uint8_t size);
static float OxyVoltProcessor(const uint8_t *rsp, uint8_t size);
static float OxyCurrProcessor(const uint8_t *rsp, uint8_t size);

/* Private variables ---------------------------------------------------------*/

static CAN_HandleTypeDef *ObdCan = NULL;
static CAN_FilterTypeDef ObdFilter = {0};
static CAN_TxHeaderTypeDef ObdTxHeader = {0};
static CAN_RxHeaderTypeDef ObdRxHeader = {0};

// Array of OBD service 0x01 PID descriptions
static const ObdService_0x01_PidDesc_t ObdPidDescs[] = {
  { OBD_SVC_0x01_PID_ENG_SPEED,  "Engine speed",                  "rpm",  EndSpeedProcessor },
  { OBD_SVC_0x01_PID_VEH_SPEED,  "Vehicle speed",                 "km/h", VehSpeedProcessor },
  { OBD_SVC_0x01_PID_ENG_LOAD,   "Calculated engine load",        "%",    EngLoadProcessor  },
  { OBD_SVC_0x01_PID_THROTTLE,   "Throttle position",             "%",    ThrottleProcessor },
  { OBD_SVC_0x01_PID_DIST_MIL,   "Distance traveled with MIL on", "km",   DistMilProcessor  },
  { OBD_SVC_0x01_PID_OXY_VOLT_1, "Oxygen sensor 1 (Voltage)",     "V",    OxyVoltProcessor  },
  { OBD_SVC_0x01_PID_OXY_VOLT_2, "Oxygen sensor 2 (Voltage)",     "V",    OxyVoltProcessor  },
  { OBD_SVC_0x01_PID_OXY_CURR_1, "Oxygen sensor 1 (Current)",     "mA",   OxyCurrProcessor  },
  { OBD_SVC_0x01_PID_OXY_CURR_2, "Oxygen sensor 2 (Current)",     "mA",   OxyCurrProcessor  }
};

static uint8_t ObdReqBuffer[OBD_NUM_PIDS][OBD_BUFFER_SIZE] = {0};
static uint8_t ObdRspBuffer[OBD_NUM_PIDS][OBD_BUFFER_SIZE] = {0};

/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/


/**
 * @brief  Initialize the OBD driver.
 * @param  hcan Pointer to the FDCAN handle
 * @retval ObdError_t
 */
ObdError_t ObdInit(CAN_HandleTypeDef *hcan) {
  // Check if the FDCAN handle is valid
  if (hcan == NULL) {
    return OBD_ERR_CAN;
  }

  // Set the FDCAN handle
  ObdCan = hcan;

  // Configure the CAN filter
  ObdFilter.FilterActivation = CAN_FILTER_ENABLE;
  ObdFilter.FilterMode = CAN_FILTERMODE_IDMASK;
  ObdFilter.FilterScale = CAN_FILTERSCALE_32BIT;
  ObdFilter.FilterIdHigh = (0x7E8 << 5u);
  ObdFilter.FilterMaskIdHigh = (0x7F8 << 5u);
  ObdFilter.FilterIdLow = 0u;
  ObdFilter.FilterMaskIdLow = 0u;
  ObdFilter.FilterFIFOAssignment = CAN_RX_FIFO0;
  ObdFilter.FilterBank = 0u;
  ObdFilter.SlaveStartFilterBank = 0u;
  if (HAL_CAN_ConfigFilter(ObdCan, &ObdFilter) != HAL_OK) {
    return OBD_ERR_CAN_FILTER;
  }

  // Start the CAN peripheral
  if (HAL_CAN_Start(ObdCan) != HAL_OK) {
    return OBD_ERR_CAN;
  }

  // Configure the CAN request message header
  ObdTxHeader.StdId = OBD_CAN_REQ_ID;
  ObdTxHeader.ExtId = 0u;
  ObdTxHeader.IDE = CAN_ID_STD;
  ObdTxHeader.RTR = CAN_RTR_DATA;
  ObdTxHeader.DLC = OBD_BUFFER_SIZE;
  ObdTxHeader.TransmitGlobalTime = DISABLE;

  // Configure the FDCAN request and response buffers
  memset(ObdReqBuffer, OBD_DUMMY_BYTE, sizeof(ObdReqBuffer));
  memset(ObdRspBuffer, 0x00u, sizeof(ObdRspBuffer));

  return OBD_OK;
}

/**
 * @brief  De-initialize the OBD driver.
 * @retval ObdError_t
 */
ObdError_t ObdDeInit(void) {
  // Stop the CAN peripheral
  if (HAL_CAN_Stop(ObdCan) != HAL_OK) {
    return OBD_ERR_CAN;
  }

  return OBD_OK;
}

/**
 * @brief  Send an OBD request.
 * @param  index Index of the OBD request buffer.
 * @param  PID OBD service 0x01 PID to send.
 * @retval ObdError_t
 */
ObdError_t ObdSend(uint8_t index, ObdService_0x01_PID_t PID) {
  uint32_t mailbox = 0u;

  // Clear the OBD request buffer
  memset(ObdReqBuffer[index], OBD_DUMMY_BYTE, OBD_BUFFER_SIZE);

  // Check if the PID is in the OBD service 0x01 PID enum
  if (!IsByteInEnum(PID)) {
    return OBD_ERR_CMD | OBD_ERR_PID;
  }

  // Set the OBD request buffer
  ObdReqBuffer[index][0] = 0x02u; // Number of bytes in the request not include this one
  ObdReqBuffer[index][1] = 0x01u; // OBD service 0x01
  ObdReqBuffer[index][2] = PID;   // OBD PID

  // Add the OBD request
  if (HAL_CAN_AddTxMessage(ObdCan, &ObdTxHeader, (uint8_t *) ObdReqBuffer[index], &mailbox) != HAL_OK) {
    return OBD_ERR_CAN_TX | OBD_ERR_BUFFER;
  }

  // Wait for the TX buffer request to be sent
  uint32_t messageStartTick = HAL_GetTick();
  do {
    if ((HAL_GetTick() - messageStartTick) > OBD_TIMEOUT_MS) {
      return OBD_ERR_CAN_TX | OBD_ERR_TIMEOUT;
    }
  } while (HAL_CAN_IsTxMessagePending(ObdCan, mailbox) == SET);

  return OBD_OK;
}

/**
 * @brief  Receive an OBD response.
 * @param  index Index of the OBD response buffer.
 * @retval ObdError_t
 */
ObdError_t ObdRecv(uint8_t index) {
  // Clear the OBD response buffer
  memset(ObdRspBuffer[index], 0x00u, OBD_BUFFER_SIZE);

  // Wait for the OBD response to be received
  uint32_t messageStartTick = HAL_GetTick();
  do {
    if ((HAL_GetTick() - messageStartTick) > OBD_TIMEOUT_MS) {
      return OBD_ERR_CAN_RX | OBD_ERR_TIMEOUT;
    }
  } while (HAL_CAN_GetRxFifoFillLevel(ObdCan, CAN_RX_FIFO0) == !SET);

  // Get the OBD response
  if (HAL_CAN_GetRxMessage(ObdCan, CAN_RX_FIFO0, &ObdRxHeader, (uint8_t *) ObdRspBuffer[index]) != HAL_OK) {
    return OBD_ERR_CAN_RX | OBD_ERR_BUFFER;
  }

  return OBD_OK;
}

/**
 * @brief  Get the OBD request buffer.
 * @param  index Index of the OBD request buffer.
 * @retval Pointer to the OBD request buffer.
 */
uint8_t *ObdGetReq(uint8_t index) {
  return ObdReqBuffer[index];
}

/**
 * @brief  Get the OBD response buffer.
 * @param  index Index of the OBD response buffer.
 * @retval Pointer to the OBD response buffer.
 */
uint8_t *ObdGetRsp(uint8_t index) {
  return ObdRspBuffer[index];
}

/**
 * @brief  Get the OBD service 0x01 PID descriptions.
 * @retval Pointer to the OBD service 0x01 PID descriptions.
 */
const ObdService_0x01_PidDesc_t *ObdGetPidDescs(void) {
  return ObdPidDescs;
}

/**
 * @brief  Get the description of an OBD service 0x01 PID.
 * @param  PID OBD service 0x01 PID.
 * @retval ObdService_0x01_PidDesc_t of the PID or NULL if not found.
 */
const ObdService_0x01_PidDesc_t *ObdGetPidDesc(ObdService_0x01_PID_t PID) {
  for (uint32_t i = 0u; i < OBD_NUM_PIDS; i++) {
    if (ObdPidDescs[i].PID == PID) {
      return &ObdPidDescs[i];
    }
  }

  // Return NULL if the PID is not found
  return NULL;
}

/**
 * @brief  Get the number of OBD service 0x01 PIDs.
 * @retval Number of OBD service 0x01 PIDs.
 */
uint8_t ObdGetNumPids(void) {
  return OBD_NUM_PIDS;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Check if a byte is in the OBD service 0x01 PID enum.
 * @param  byte Byte to check.
 * @retval true if the byte is in the enum, false otherwise.
 */
static bool IsByteInEnum(uint8_t byte) {
  // Check if the byte matches any of the valid PIDs
  for (uint16_t i = 0; i < (sizeof(ObdPidDescs) / sizeof(ObdPidDescs[0])); i++) {
    if (byte == (uint8_t) ObdPidDescs[i].PID) {
      return true; // Byte is in the enum
    }
  }
  return false; // Byte is not in the enum
}

/**
 * @brief  OBD response processor for the calculated engine load.
 * @param  rsp OBD response.
 * @param  size Size of the OBD response.
 * @retval Calculated engine load.
 */
static float EngLoadProcessor(const uint8_t *rsp, uint8_t size) {
  return (float) (rsp[3] / 2.55f);
}

/**
 * @brief  OBD response processor for the engine speed.
 * @param  rsp OBD response.
 * @param  size Size of the OBD response.
 * @retval Engine speed.
 */
static float EndSpeedProcessor(const uint8_t *rsp, uint8_t size) {
  return (float) ((256 * rsp[3]) + rsp[4]) / 4.0f;
}

/**
 * @brief  OBD response processor for the vehicle speed.
 * @param  rsp OBD response.
 * @param  size Size of the OBD response.
 * @retval Vehicle speed.
 */
static float VehSpeedProcessor(const uint8_t *rsp, uint8_t size) {
  return (float) rsp[3];
}

/**
 * @brief  OBD response processor for the throttle position.
 * @param  rsp OBD response.
 * @param  size Size of the OBD response.
 * @retval Throttle position.
 */
static float ThrottleProcessor(const uint8_t *rsp, uint8_t size) {
  return (float) (rsp[3] / 2.55f);
}

/**
 * @brief  OBD response processor for the distance traveled with MIL on.
 * @param  rsp OBD response.
 * @param  size Size of the OBD response.
 * @retval Distance traveled with MIL on.
 */
static float DistMilProcessor(const uint8_t *rsp, uint8_t size) {
  return (float) ((256 * rsp[3]) + rsp[4]);
}

/**
 * @brief  OBD response processor for the oxygen sensor voltage.
 * @param  rsp OBD response.
 * @param  size Size of the OBD response.
 * @retval Oxygen sensor voltage.
 */
static float OxyVoltProcessor(const uint8_t *rsp, uint8_t size) {
  return (float) (rsp[3] / 200.0f);
}

/**
 * @brief  OBD response processor for the oxygen sensor current.
 * @param  rsp OBD response.
 * @param  size Size of the OBD response.
 * @retval Oxygen sensor current.
 */
static float OxyCurrProcessor(const uint8_t *rsp, uint8_t size) {
  return (float) (((256 * rsp[5]) + rsp[6]) / 256.0f) - 128.0f;
}

/********************************* END OF FILE ********************************/
