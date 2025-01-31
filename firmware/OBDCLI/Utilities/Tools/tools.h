/**
 * @file  tools.h
 * @brief Common utilities used in the project.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef TOOLS_H
#define TOOLS_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported defines ----------------------------------------------------------*/

#define BUFFER_SIZE         48U
#define RESPONSE_SOF        0x34U // '4'
#define RESPONSE_EOF        0x0DU // '\r'
#define RETRANSMIT_ATTEMPTS 0x03U

/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

#define ASCII_TO_HEX(_BYTE_) (uint8_t)((_BYTE_) - '0')

/* Exported variables --------------------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/

#ifdef __cplusplus
}
#endif
#endif  /* TOOLS_H */

/********************************* END OF FILE ********************************/
