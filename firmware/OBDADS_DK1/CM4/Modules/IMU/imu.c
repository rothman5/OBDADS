/**
 * @file   imu.c
 * @brief  IMU communications driver source file.
 * @author Rasheed Othman
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "imu.h"
#include "system.h"

/* Private define ------------------------------------------------------------*/

#define WR_MASK 0x7Fu // MSB (first bit) is 0 for write operations
#define RD_MASK 0x80u // MSB (first bit) is 1 for read operations

/* Private types -------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static SPI_HandleTypeDef *ImuSpi = NULL;
static Vec3_t ImuVecXl = {0};
static Vec3_t ImuVecGy = {0};
static float ImuTemp = 0.0f;
static uint8_t ImuCmdBuffer[IMU_BUFFER_SIZE] = {0};
static uint8_t ImuRspBuffer[IMU_BUFFER_SIZE] = {0};

/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

ImuError_t ImuInit(SPI_HandleTypeDef *hspi) {
  ImuError_t err = IMU_OK;

  // Check if the SPI handle is valid
  if (hspi == NULL) {
    err = IMU_ERR_SPI;
  }

  // Initialize the IMU SPI handle
  ImuSpi = hspi;

  // Check the IMU sensor ID
  uint8_t attempt = 0u;
  while (attempt++ < IMU_MAX_ID_ATTEMPTS) {
    err = ImuReadReg(IMU_ADDR_WHO_AM_I, sizeof(uint8_t));
    if (err != IMU_OK) {
      err |= IMU_ERR_ID;
      continue;
    }

    if (ImuRspBuffer[1] == IMU_SENSOR_ID) {
      err = IMU_OK;
      break;
    } else {
      err |= IMU_ERR_ID;
    }

    HAL_Delay(IMU_POLL_DELAY_MS);
  }

  uint8_t reg = 0x00u;

  // Disable INT1 interrupts
  reg = 0b00000000;
  err = ImuWriteReg(IMU_ADDR_INT1_CTRL, &reg, sizeof(reg));
  if (err != IMU_OK) {
    return err;
  }

  // Enable temperature, accelerometer, and gyroscope data-ready interrupts on INT2
  reg = 0b00000111;
  err = ImuWriteReg(IMU_ADDR_INT2_CTRL, &reg, sizeof(reg));
  if (err != IMU_OK) {
    return err;
  }

  // Configure XL range and output data rate and filter
  reg = (IMU_ODR_1667HZ << 4) | (IMU_XL_RANGE_16G << 2) | 0x00u;
  err = ImuWriteReg(IMU_ADDR_CTRL1_XL, &reg, sizeof(reg));
  if (err != IMU_OK) {
    return err;
  }

  // Configure gyroscope range and output data rate
  reg = (IMU_ODR_1667HZ << 4) | (IMU_GY_RANGE_2000DPS << 2) | 0x00u;
  err = ImuWriteReg(IMU_ADDR_CTRL2_G, &reg, sizeof(reg));
  if (err != IMU_OK) {
    return err;
  }

  // Disable gyroscope sleep mode and filter, disable I2C
  reg = 0b00000000;
  err = ImuWriteReg(IMU_ADDR_CTRL4_C, &reg, sizeof(reg));
  if (err != IMU_OK) {
    return err;
  }

  // Configure interrupt active level, SPI mode, and reset device
  reg = 0b00000101;
  err = ImuWriteReg(IMU_ADDR_CTRL3_C, &reg, sizeof(reg));
  if (err != IMU_OK) {
    return err;
  }

  return err;
}

ImuError_t ImuDeInit(void) {
  ImuError_t err = IMU_OK;
  return err;
}

ImuError_t ImuReadReg(ImuRegAddress_t reg, uint8_t size) {
  ImuError_t err = IMU_OK;

  // Check if the data is valid
  if (size > IMU_BUFFER_SIZE) {
    err = IMU_ERR_CMD | IMU_ERR_DATA;
    return err;
  }

  // Set the register address in the command buffer
  ImuCmdBuffer[0] = (uint8_t) reg | RD_MASK;

  // Clear the command and response buffers
  memset(ImuCmdBuffer, 0x00u, IMU_BUFFER_SIZE);
  memset(ImuRspBuffer, 0x00u, IMU_BUFFER_SIZE);

  // Select the IMU using the SPI NSS pin
  HAL_GPIO_WritePin(IMU_SPI_NSS_GPIO_Port, IMU_SPI_NSS_Pin, GPIO_PIN_RESET);

  // Transmit the command buffer to the IMU
  // The plus one is to account for the register address
  if (HAL_SPI_TransmitReceive(ImuSpi, ImuCmdBuffer, ImuRspBuffer, size + 1, IMU_TIMEOUT_MS) != HAL_OK) {
    err = IMU_ERR_SPI_RX | IMU_ERR_SPI_TX;
  }

  // Deselect the IMU using the SPI NSS pin
  HAL_GPIO_WritePin(IMU_SPI_NSS_GPIO_Port, IMU_SPI_NSS_Pin, GPIO_PIN_SET);

  return err;
}

ImuError_t ImuWriteReg(ImuRegAddress_t reg, uint8_t *data, uint8_t size) {
  ImuError_t err = IMU_OK;

  // Check if the data is valid
  if ((data == NULL) || (size > IMU_BUFFER_SIZE) || (size < 1)) {
    err = IMU_ERR_CMD | IMU_ERR_DATA;
    return err;
  }

  // Set the register address in the command buffer
  ImuCmdBuffer[0] = (uint8_t) reg & WR_MASK;

  // Copy the data to the command buffer
  memset(ImuCmdBuffer, 0x00u, IMU_BUFFER_SIZE);
  memcpy(&ImuCmdBuffer[1], data, size);

  // Select the IMU using the SPI NSS pin
  HAL_GPIO_WritePin(IMU_SPI_NSS_GPIO_Port, IMU_SPI_NSS_Pin, GPIO_PIN_RESET);

  // Transmit the command buffer to the IMU
  if (HAL_SPI_Transmit(ImuSpi, ImuCmdBuffer, size + 1, IMU_TIMEOUT_MS) != HAL_OK) {
    err = IMU_ERR_SPI_TX;
  }

  // Deselect the IMU using the SPI NSS pin
  HAL_GPIO_WritePin(IMU_SPI_NSS_GPIO_Port, IMU_SPI_NSS_Pin, GPIO_PIN_SET);

  return err;
}

ImuError_t ImuReadXl(void) {
  ImuError_t err = IMU_OK;

  err = ImuReadReg(IMU_ADDR_OUTX_H_A, sizeof(uint16_t) * 3);
  if (err != IMU_OK) {
    return err;
  }

  int16_t x = BYTES_TO_S16(ImuRspBuffer[2], ImuRspBuffer[1]);
  int16_t y = BYTES_TO_S16(ImuRspBuffer[4], ImuRspBuffer[3]);
  int16_t z = BYTES_TO_S16(ImuRspBuffer[6], ImuRspBuffer[5]);

  ImuVecXl.x = (float) x * IMU_XL_SENS_16G;
  ImuVecXl.y = (float) y * IMU_XL_SENS_16G;
  ImuVecXl.z = (float) z * IMU_XL_SENS_16G;

  return err;
}

ImuError_t ImuReadGy(void) {
  ImuError_t err = IMU_OK;

  err = ImuReadReg(IMU_ADDR_OUTX_H_G, sizeof(uint16_t) * 3);
  if (err != IMU_OK) {
    return err;
  }

  int16_t x = BYTES_TO_S16(ImuRspBuffer[2], ImuRspBuffer[1]);
  int16_t y = BYTES_TO_S16(ImuRspBuffer[4], ImuRspBuffer[3]);
  int16_t z = BYTES_TO_S16(ImuRspBuffer[6], ImuRspBuffer[5]);

  ImuVecGy.x = (float) x * IMU_GY_SENS_2000DPS;
  ImuVecGy.y = (float) y * IMU_GY_SENS_2000DPS;
  ImuVecGy.z = (float) z * IMU_GY_SENS_2000DPS;

  return err;
}

ImuError_t ImuReadTemp(void) {
  ImuError_t err = IMU_OK;

  err = ImuReadReg(IMU_ADDR_OUT_TEMP_H, sizeof(uint16_t));
  if (err != IMU_OK) {
    return err;
  }

  int16_t temp = BYTES_TO_S16(ImuRspBuffer[2], ImuRspBuffer[1]);

  ImuTemp = (float) temp / IMU_TEMP_SENS;

  return err;
}

Vec3_t *ImuGetXlData(void) {
  return &ImuVecXl;
}

Vec3_t *ImuGetGyData(void) {
  return &ImuVecGy;
}

float *ImuGetTemp(void) {
  return &ImuTemp;
}

/* Private functions ---------------------------------------------------------*/

/********************************* END OF FILE ********************************/
