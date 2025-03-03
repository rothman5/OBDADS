/**
 * @file  system.c
 * @brief System state machine source file.
 */

/* Includes ------------------------------------------------------------------*/
#include "system.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Public variables ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Initialize the system state machine's required peripherals and start the
 *         state machine execution timer.
 * @param  htim: Reference to the timer handler.
 * @return SystemError_t
 */
SystemError_t SystemInit(TIM_HandleTypeDef *htim) {
  if (htim == NULL) {
    return SYS_ERROR_TIMER;
  }

  // Start the state machine execution timer
  if (HAL_TIM_Base_Start_IT(htim) != HAL_OK) {
    return SYS_ERROR_TIMER;
  }
}

/**
 * @brief  Execute the system state machine and configure the next state.
 * @note   Executed every 250ms.
 * @return SystemError_t
 */
SystemError_t SystemExec(void) {
  HAL_GPIO_TogglePin(SME_GPIO_Port, SME_Pin);
}

/* Private functions ---------------------------------------------------------*/

/********************************* END OF FILE ********************************/
