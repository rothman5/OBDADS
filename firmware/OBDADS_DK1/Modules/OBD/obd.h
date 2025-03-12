/**
 * @file   obd.h
 * @brief  OBD communications driver header file.
 * @author Rasheed Othman
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef OBD_H
#define OBD_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "fdcan.h"

/* Exported defines ----------------------------------------------------------*/

#define OBD_MAX_REQUESTS  10u   // Maximum number of OBD requests
#define OBD_BUFFER_SIZE   8u    // OBD request and response buffer size
#define OBD_TIMEOUT_MS    1000u // OBD request timeout in milliseconds
#define OBD_PID_MASK      0x40u // OBD PID mask
#define OBD_MAX_DESC_SIZE 32u   // Maximum OBD PID description size
#define OBD_PID_INDEX     2u    // OBD PID index in the request and response buffer
#define OBD_SVC_INDEX     1u    // OBD service index in the request and response buffer

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Error codes for the OBD communications driver.
 * @note  Each error code is a bit mask that can be combined with other error codes.
 */
typedef enum __OBD_ERROR_e {
  OBD_OK             = 0u,        // No error
  OBD_ERR_UNKNOWN    = (1u << 0), // An unknown error occured
  OBD_ERR_CMD        = (1u << 1), // Invalid OBD command or command format
  OBD_ERR_DATA       = (1u << 2), // Invalid OBD data or data format
  OBD_ERR_PID        = (1u << 3), // Invalid OBD PID
  OBD_ERR_CAN        = (1u << 4), // A CAN error occured
  OBD_ERR_CAN_TX     = (1u << 5), // A CAN TX error occured
  OBD_ERR_CAN_RX     = (1u << 6), // A CAN RX error occured
  OBD_ERR_CAN_FILTER = (1u << 7), // CAN filter configuration error
  OBD_ERR_BUFFER     = (1u << 8), // CAN bus buffer is full
  OBD_ERR_TIMEOUT    = (1u << 9)  // Timed out waiting for requested data
} ObdError_t;

/**
 * @brief OBD service codes.
 * @note  https://en.wikipedia.org/wiki/OBD-II_PIDs
 */
typedef enum __OBD_SERVICE_e {
  OBD_SERVICE_01 = 0x01, // Show current data
  OBD_SERVICE_02 = 0x02, // Show freeze frame data
  OBD_SERVICE_03 = 0x03, // Show stored Diagnostic Trouble Codes
  OBD_SERVICE_04 = 0x04, // Clear Diagnostic Trouble Codes and stored values
  OBD_SERVICE_05 = 0x05, // Test results, oxygen sensor monitoring (non CAN only)
  OBD_SERVICE_06 = 0x06, // Test results, other component/system monitoring (Test results, oxygen sensor monitoring for CAN only)
  OBD_SERVICE_07 = 0x07, // Show pending Diagnostic Trouble Codes (detected during current or last driving cycle)
  OBD_SERVICE_08 = 0x08, // Control operation of on-board component/system
  OBD_SERVICE_09 = 0x09, // Request vehicle information
  OBD_SERVICE_10 = 0x0A  // Permanent Diagnostic Trouble Codes (DTCs) (Cleared DTCs)
} ObdService_t;

/**
 * @brief OBD service 0x01 PIDs to be requested.
 * @note  https://en.wikipedia.org/wiki/OBD-II_PIDs
 */
typedef enum __OBD_SERVICE_0x01_PID_e {
  OBD_SVC_0x01_PID_ENG_LOAD   = 0x04u, // Calculated engine load
  OBD_SVC_0x01_PID_ENG_SPEED  = 0x0Cu, // Engine speed
  OBD_SVC_0x01_PID_VEH_SPEED  = 0x0Du, // Vehicle speed
  OBD_SVC_0x01_PID_THROTTLE   = 0x11u, // Throttle position
  OBD_SVC_0x01_PID_DIST_MIL   = 0x21u, // Distance traveled with MIL on
  OBD_SVC_0x01_PID_OXY_VOLT_1 = 0x24u, // Oxygen sensor 1 (Voltage)
  OBD_SVC_0x01_PID_OXY_VOLT_2 = 0x25u, // Oxygen sensor 2 (Voltage)
  OBD_SVC_0x01_PID_OXY_CURR_1 = 0x34u, // Oxygen sensor 1 (Current)
  OBD_SVC_0x01_PID_OXY_CURR_2 = 0x35u, // Oxygen sensor 2 (Current)
} ObdService_0x01_PID_t;

/**
 * @brief OBD response processor function pointer.
 */
typedef float (*ObdRspProcessor_t)(const uint8_t *rsp, uint8_t size);

/**
 * @brief OBD PID description structure.
 */
typedef struct __OBD_PID_DESCRIPTION_s {
    ObdService_0x01_PID_t PID;
    const char* name;
    const char* unit;
    ObdRspProcessor_t processor;
} ObdPidDesc_t;

/* Exported macro ------------------------------------------------------------*/

#define OBD_NUM_PIDS (uint8_t) (sizeof(ObdPidDescs) / sizeof(ObdPidDescs[0]))

/* Exported variables --------------------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/

ObdError_t ObdInit(FDCAN_HandleTypeDef *hfdcan);
ObdError_t ObdDeInit(void);
ObdError_t ObdSend(uint8_t index, ObdService_0x01_PID_t PID);
ObdError_t ObdRecv(uint8_t index);
uint8_t *ObdGetReq(uint8_t index);
uint8_t *ObdGetRsp(uint8_t index);
const ObdPidDesc_t *ObdGetPidDescs(void);
const ObdPidDesc_t *ObdGetPidDesc(ObdService_0x01_PID_t PID);
uint8_t ObdGetNumPids(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* OBD_H */

/********************************* END OF FILE ********************************/
