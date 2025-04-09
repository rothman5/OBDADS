/**
 * @file   imu.c
 * @brief  IMU communications driver source file.
 * @author Rasheed Othman
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "imu.h"
#include "sys.h"

/* Private define ------------------------------------------------------------*/

#define WR_MASK 0x7Fu // MSB (first bit) is 0 for write operations
#define RD_MASK 0x80u // MSB (first bit) is 1 for read operations

/* Private types -------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

static ImuError_t CheckSensorStatus(ImuSensor_t sensor);

/* Private variables ---------------------------------------------------------*/

static SPI_HandleTypeDef *ImuSpi = NULL;

static uint8_t ImuCmdBuffer[IMU_BUFFER_SIZE] = {0};
static uint8_t ImuRspBuffer[IMU_BUFFER_SIZE] = {0};

static Vec3_t ImuVecXl = {0};
static Vec3_t ImuVecGy = {0};
static float ImuTemp = 0.0f;

static volatile FlagStatus ImuXlReady = RESET;
static volatile FlagStatus ImuGyReady = RESET;
static volatile FlagStatus ImuTempReady = RESET;

/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Initializes the IMU communications driver.
 * @param  hspi: Pointer to the SPI handle
 * @retval ImuError_t
 */
ImuError_t ImuInit(SPI_HandleTypeDef *hspi) {
  ImuError_t err = IMU_OK;

  // Check if the SPI handle is valid
  if (hspi == NULL) {
    err = IMU_ERR_SPI;
  }

  // Initialize the IMU SPI handle
  ImuSpi = hspi;

  // Configure interrupt active level, SPI mode, and reset device
  uint8_t reg = 0b00000101;
  err = ImuWriteReg(IMU_ADDR_CTRL3_C, &reg, sizeof(reg));
  if (err != IMU_OK) {
    return err;
  }

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

  if (err & IMU_ERR_ID) {
    return err;
  }

  // Enable configuration
  reg = 0b11100010;
  err = ImuWriteReg(IMU_ADDR_CTRL9_XL, &reg, sizeof(reg));
  if (err != IMU_OK) {
    return err;
  }

  // Enable XL data-ready interrupt on INT1
  reg = 0b00000001;
  err = ImuWriteReg(IMU_ADDR_INT1_CTRL, &reg, sizeof(reg));
  if (err != IMU_OK) {
    return err;
  }

  // Enable GY data-ready interrupt on INT2
  reg = 0b00000010;
  err = ImuWriteReg(IMU_ADDR_INT2_CTRL, &reg, sizeof(reg));
  if (err != IMU_OK) {
    return err;
  }

  // Configure XL range and output data rate and filter
  reg = (IMU_ODR_416HZ << 4) | (IMU_XL_RANGE_8G << 2) | 0x00u;
  err = ImuWriteReg(IMU_ADDR_CTRL1_XL, &reg, sizeof(reg));
  if (err != IMU_OK) {
    return err;
  }

  // Configure gyroscope range and output data rate
  reg = (IMU_ODR_416HZ << 4) | (IMU_GY_RANGE_1000DPS << 2) | 0x00u;
  err = ImuWriteReg(IMU_ADDR_CTRL2_G, &reg, sizeof(reg));
  if (err != IMU_OK) {
    return err;
  }

  // Configure interrupt active level, SPI mode, and block data updates
  reg = 0b01000100;
  err = ImuWriteReg(IMU_ADDR_CTRL3_C, &reg, sizeof(reg));
  if (err != IMU_OK) {
    return err;
  }

  // Disable gyroscope sleep mode and filter, disable I2C
  reg = 0b00000100;
  err = ImuWriteReg(IMU_ADDR_CTRL4_C, &reg, sizeof(reg));
  if (err != IMU_OK) {
    return err;
  }

  // Clear latched interrupts immediately after reading
  reg = 0b01000001;
  err = ImuWriteReg(IMU_ADDR_INT_CFG0, &reg, sizeof(reg));
  if (err != IMU_OK) {
    return err;
  }

  // Enable interrupts
  reg = 0b10000000;
  err = ImuWriteReg(IMU_ADDR_INT_CFG1, &reg, sizeof(reg));
  if (err != IMU_OK) {
    return err;
  }

  // Disable configuration
  reg = 0b11100000;
  err = ImuWriteReg(IMU_ADDR_CTRL9_XL, &reg, sizeof(reg));
  if (err != IMU_OK) {
    return err;
  }

  HAL_Delay(IMU_POLL_DELAY_MS);

  ImuReadXlBlocking();
  ImuReadGyBlocking();
  ImuReadTempBlocking();

  return err;
}

/**
 * @brief  De-initialize the IMU communications driver.
 * @retval ImuError_t
 */
ImuError_t ImuDeInit(void) {
  ImuError_t err = IMU_OK;
  return err;
}

/**
 * @brief  Reads register(s) from the IMU.
 * @param  reg:  Register address to read
 * @param  size: Number of registers to read, each register is 1 byte
 * @retval ImuError_t
 */
ImuError_t ImuReadReg(ImuRegAddress_t reg, uint8_t size) {
  ImuError_t err = IMU_OK;

  // Check if the data is valid
  if (size > IMU_BUFFER_SIZE) {
    err = IMU_ERR_CMD | IMU_ERR_DATA;
    return err;
  }

  // Clear the command and response buffers
  memset(ImuCmdBuffer, 0x00u, IMU_BUFFER_SIZE);
  memset(ImuRspBuffer, 0x00u, IMU_BUFFER_SIZE);

  // Set the register address in the command buffer
  ImuCmdBuffer[0] = (uint8_t) reg | RD_MASK;

  // Select the IMU using the SPI NSS pin
  HAL_GPIO_WritePin(IMU_NSS_GPIO_Port, IMU_NSS_Pin, GPIO_PIN_RESET);

  // Transmit the command buffer to the IMU
  // The plus one is to account for the register address
  if (HAL_SPI_TransmitReceive(ImuSpi, ImuCmdBuffer, ImuRspBuffer, size + 1, IMU_TIMEOUT_MS) != HAL_OK) {
    err = IMU_ERR_SPI_RX | IMU_ERR_SPI_TX;
  }

  // Deselect the IMU using the SPI NSS pin
  HAL_GPIO_WritePin(IMU_NSS_GPIO_Port, IMU_NSS_Pin, GPIO_PIN_SET);

  return err;
}

/**
 * @brief  Writes register(s) to the IMU.
 * @param  reg:  Register address to write
 * @param  data: Pointer to the data to write
 * @param  size: Number of registers to write, each register is 1 byte
 * @retval ImuError_t
 */
ImuError_t ImuWriteReg(ImuRegAddress_t reg, uint8_t *data, uint8_t size) {
  ImuError_t err = IMU_OK;

  // Check if the data is valid
  if ((data == NULL) || (size > IMU_BUFFER_SIZE) || (size < 1)) {
    err = IMU_ERR_CMD | IMU_ERR_DATA;
    return err;
  }

  // Copy the data to the command buffer
  memset(ImuCmdBuffer, 0x00u, IMU_BUFFER_SIZE);
  memcpy(&ImuCmdBuffer[1], data, size);

  // Set the register address in the command buffer
  ImuCmdBuffer[0] = (uint8_t) reg & WR_MASK;

  // Select the IMU using the SPI NSS pin
  HAL_GPIO_WritePin(IMU_NSS_GPIO_Port, IMU_NSS_Pin, GPIO_PIN_RESET);

  // Transmit the command buffer to the IMU
  if (HAL_SPI_Transmit(ImuSpi, ImuCmdBuffer, size + 1, IMU_TIMEOUT_MS) != HAL_OK) {
    err = IMU_ERR_SPI_TX;
  }

  // Deselect the IMU using the SPI NSS pin
  HAL_GPIO_WritePin(IMU_NSS_GPIO_Port, IMU_NSS_Pin, GPIO_PIN_SET);

  return err;
}

/**
 * @brief  Reads accelerometer data from the IMU.
 * @retval ImuError_t
 */
ImuError_t ImuReadXl(void) {
  ImuError_t err = IMU_OK;

  if (ImuXlIsReady()) {
    ImuXlSetStatus(RESET);

    // Read the accelerometer data
    err = ImuReadReg(IMU_ADDR_OUTX_L_A, sizeof(uint16_t) * 3);
    if (err != IMU_OK) {
      return err;
    }

    // Convert the data to floating point values
    int16_t x = BYTES_TO_S16(ImuRspBuffer[2], ImuRspBuffer[1]);
    int16_t y = BYTES_TO_S16(ImuRspBuffer[4], ImuRspBuffer[3]);
    int16_t z = BYTES_TO_S16(ImuRspBuffer[6], ImuRspBuffer[5]);
    ImuVecXl.x = (float) x * IMU_XL_SENS_8G / 1000.0f;
    ImuVecXl.y = (float) y * IMU_XL_SENS_8G / 1000.0f;
    ImuVecXl.z = (float) z * IMU_XL_SENS_8G / 1000.0f;
  }

  return err;
}

/**
 * @brief  Reads accelerometer data from the IMU.
 * @retval ImuError_t
 */
ImuError_t ImuReadXlBlocking(void) {
  ImuError_t err = IMU_OK;

  // Wait for the IMU data to be available
  err = CheckSensorStatus(IMU_SENSOR_XL);
  if (err != IMU_OK) {
    return err;
  }

  // Read the accelerometer data
  err = ImuReadReg(IMU_ADDR_OUTX_L_A, sizeof(uint16_t) * 3);
  if (err != IMU_OK) {
    return err;
  }

  // Convert the data to floating point values
  int16_t x = BYTES_TO_S16(ImuRspBuffer[2], ImuRspBuffer[1]);
  int16_t y = BYTES_TO_S16(ImuRspBuffer[4], ImuRspBuffer[3]);
  int16_t z = BYTES_TO_S16(ImuRspBuffer[6], ImuRspBuffer[5]);
  ImuVecXl.x = (float) x * IMU_XL_SENS_8G / 1000.0f;
  ImuVecXl.y = (float) y * IMU_XL_SENS_8G / 1000.0f;
  ImuVecXl.z = (float) z * IMU_XL_SENS_8G / 1000.0f;

  return err;
}

/**
 * @brief  Reads gyroscope data from the IMU.
 * @retval ImuError_t
 */
ImuError_t ImuReadGy(void) {
  ImuError_t err = IMU_OK;

  if (ImuGyIsReady()) {
    ImuGySetStatus(RESET);

    // Read the gyroscope data
    err = ImuReadReg(IMU_ADDR_OUTX_L_G, sizeof(uint16_t) * 3);
    if (err != IMU_OK) {
      return err;
    }

    // Convert the data to floating point values
    int16_t x = BYTES_TO_S16(ImuRspBuffer[2], ImuRspBuffer[1]);
    int16_t y = BYTES_TO_S16(ImuRspBuffer[4], ImuRspBuffer[3]);
    int16_t z = BYTES_TO_S16(ImuRspBuffer[6], ImuRspBuffer[5]);
    ImuVecGy.x = (float) x * IMU_GY_SENS_1000DPS / 1000.0f;
    ImuVecGy.y = (float) y * IMU_GY_SENS_1000DPS / 1000.0f;
    ImuVecGy.z = (float) z * IMU_GY_SENS_1000DPS / 1000.0f;
  }

  return err;
}

/**
 * @brief  Reads gyroscope data from the IMU.
 * @retval ImuError_t
 */
ImuError_t ImuReadGyBlocking(void) {
  ImuError_t err = IMU_OK;

  // Wait for the IMU data to be available
  err = CheckSensorStatus(IMU_SENSOR_GY);
  if (err != IMU_OK) {
    return err;
  }

  // Read the gyroscope data
  err = ImuReadReg(IMU_ADDR_OUTX_L_G, sizeof(uint16_t) * 3);
  if (err != IMU_OK) {
    return err;
  }

  // Convert the data to floating point values
  int16_t x = BYTES_TO_S16(ImuRspBuffer[2], ImuRspBuffer[1]);
  int16_t y = BYTES_TO_S16(ImuRspBuffer[4], ImuRspBuffer[3]);
  int16_t z = BYTES_TO_S16(ImuRspBuffer[6], ImuRspBuffer[5]);
  ImuVecGy.x = (float) x * IMU_GY_SENS_1000DPS / 1000.0f;
  ImuVecGy.y = (float) y * IMU_GY_SENS_1000DPS / 1000.0f;
  ImuVecGy.z = (float) z * IMU_GY_SENS_1000DPS / 1000.0f;

  return err;
}

/**
 * @brief  Reads temperature data from the IMU.
 * @retval ImuError_t
 */
ImuError_t ImuReadTemp(void) {
  ImuError_t err = IMU_OK;

  if (ImuTempIsReady()) {
    ImuTempSetStatus(RESET);

    // Read the temperature data
    err = ImuReadReg(IMU_ADDR_OUT_TEMP_L, sizeof(uint16_t));
    if (err != IMU_OK) {
      return err;
    }

    // Convert the data to floating point values
    int16_t temp = BYTES_TO_S16(ImuRspBuffer[2], ImuRspBuffer[1]);
    ImuTemp = (float) temp / IMU_TEMP_SENS + 25.0f;
  }

  return err;
}

/**
 * @brief  Reads temperature data from the IMU.
 * @retval ImuError_t
 */
ImuError_t ImuReadTempBlocking(void) {
  ImuError_t err = IMU_OK;

  // Wait for the IMU data to be available
  err = CheckSensorStatus(IMU_SENSOR_TEMP);
  if (err != IMU_OK) {
    return err;
  }

  // Read the temperature data
  err = ImuReadReg(IMU_ADDR_OUT_TEMP_L, sizeof(uint16_t));
  if (err != IMU_OK) {
    return err;
  }

  // Convert the data to floating point values
  int16_t temp = BYTES_TO_S16(ImuRspBuffer[2], ImuRspBuffer[1]);
  ImuTemp = (float) temp / IMU_TEMP_SENS + 25.0f;

  return err;
}

/**
 * @brief  Gets the accelerometer data from the IMU.
 * @retval Vec3_t pointer
 */
Vec3_t *ImuGetXlData(void) {
  return &ImuVecXl;
}

/**
 * @brief  Gets the gyroscope data from the IMU.
 * @retval Vec3_t pointer
 */
Vec3_t *ImuGetGyData(void) {
  return &ImuVecGy;
}

/**
 * @brief  Gets the temperature data from the IMU.
 * @retval float pointer
 */
float *ImuGetTemp(void) {
  return &ImuTemp;
}

void ImuXlSetStatus(FlagStatus status) {
  ImuXlReady = status;
}

void ImuGySetStatus(FlagStatus status) {
  ImuGyReady = status;
}

void ImuTempSetStatus(FlagStatus status) {
  ImuTempReady = status;
}

FlagStatus ImuXlIsReady(void) {
  return ImuXlReady;
}

FlagStatus ImuGyIsReady(void) {
  return ImuGyReady;
}

FlagStatus ImuTempIsReady(void) {
  return ImuTempReady;
}

/**
 * @brief IMU INT1 interrupt callback.
 * @note  IMU XL data-ready interrupt.
 */
void ImuInt1Callback(void) {
  ImuXlSetStatus(SET);
}

/**
 * @brief IMU INT2 interrupt callback.
 * @note  IMU GY data-ready and temperature data-ready interrupts.
 */
void ImuInt2Callback(void) {
  ImuGySetStatus(SET);
  ImuTempSetStatus(SET);
}

/* Private functions ---------------------------------------------------------*/

static ImuError_t CheckSensorStatus(ImuSensor_t sensor) {
  ImuError_t err = IMU_OK;

  uint32_t messageStartTick = HAL_GetTick();
  while ((HAL_GetTick() - messageStartTick) < IMU_TIMEOUT_MS) {
    err = ImuReadReg(IMU_ADDR_STATUS_REG, sizeof(uint8_t));
    if (err != IMU_OK) {
      err |= IMU_ERR_TIMEOUT;
      return err;
    }

    if (ImuRspBuffer[1] & sensor) {
      break;
    } else {
      HAL_Delay(1u);
    }
  }

  return err;
}

/********************************* END OF FILE ********************************/
