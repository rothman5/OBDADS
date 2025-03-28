/**
 * @file   ipc.h
 * @brief  Inter-process communications driver header file.
 * @author Rasheed Othman
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef IPC_H
#define IPC_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "virt_uart.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/

void IpcInit(void);
void IpcService(void);
void IpcWrite(uint8_t *data, uint16_t size);
FlagStatus IpcGetHandshake(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* IPC_H */

/********************************* END OF FILE ********************************/
