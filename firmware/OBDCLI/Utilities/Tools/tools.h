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

#define BUFFER_SIZE 48U
#define EOL         '\r'

/* Exported types ------------------------------------------------------------*/

typedef struct __BUFFER_s {
  uint16_t length;
  uint8_t data[BUFFER_SIZE];
} buffer_t;

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/

#ifdef __cplusplus
}
#endif
#endif  /* TOOLS_H */

/********************************* END OF FILE ********************************/
