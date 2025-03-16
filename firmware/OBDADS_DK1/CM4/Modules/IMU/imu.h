/**
 * @file   imu.h
 * @brief  IMU communications driver header file.
 * @author Rasheed Othman
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef IMU_H
#define IMU_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "spi.h"

/* Exported defines ----------------------------------------------------------*/

#define IMU_BUFFER_SIZE     8u
#define IMU_TIMEOUT_MS      1000u
#define IMU_SENSOR_ID       0x6Bu
#define IMU_MAX_ID_ATTEMPTS 10u
#define IMU_POLL_DELAY_MS   10u

#define IMU_XL_SENS_2G      0.061f // mg/LSB
#define IMU_XL_SENS_4G      0.122f // mg/LSB
#define IMU_XL_SENS_8G      0.244f // mg/LSB
#define IMU_XL_SENS_16G     0.488f // mg/LSB
#define IMU_GY_SENS_125DPS  4.37f  // mdps/LSB
#define IMU_GY_SENS_250DPS  8.75f  // mdps/LSB
#define IMU_GY_SENS_500DPS  17.5f  // mdps/LSB
#define IMU_GY_SENS_1000DPS 35.0f  // mdps/LSB
#define IMU_GY_SENS_2000DPS 70.0f  // mdps/LSB
#define IMU_GY_SENS_4000DPS 140.0f // mdps/LSB
#define IMU_TEMP_SENS       256.0f // LSB/degC

/* Exported types ------------------------------------------------------------*/

typedef struct __VECTOR_3D_s {
  float x;
  float y;
  float z;
} Vec3_t;

/**
 * @brief Error codes for the IMU communications driver.
 * @note  Each error code is a bit mask that can be combined with other error codes.
 */
typedef enum __IMU_ERROR_e {
  IMU_OK          = 0u,        // No error
  IMU_ERR_UNKNOWN = (1u << 0), // An unknown error occured
  IMU_ERR_CMD     = (1u << 1), // Invalid IMU command or command format
  IMU_ERR_DATA    = (1u << 2), // Invalid IMU data or data format
  IMU_ERR_SPI     = (1u << 3), // A SPI error occured
  IMU_ERR_SPI_TX  = (1u << 4), // A SPI TX error occured
  IMU_ERR_SPI_RX  = (1u << 5), // A SPI RX error occured
  IMU_ERR_TIMEOUT = (1u << 6), // Timed out waiting for requested data
  IMU_ERR_ID      = (1u << 7)  // Failed to identify the IMU
} ImuError_t;

typedef enum __IMU_XL_RANGE_e {
  IMU_XL_RANGE_2G  = 0x00, // +/- 2g
  IMU_XL_RANGE_4G  = 0x01, // +/- 4g
  IMU_XL_RANGE_8G  = 0x02, // +/- 8g
  IMU_XL_RANGE_16G = 0x03  // +/- 16g
} ImuXlRange_t;

typedef enum __IMU_GY_RANGE_e {
  IMU_GY_RANGE_125DPS  = 0x00, // +/- 125 dps
  IMU_GY_RANGE_250DPS  = 0x01, // +/- 250 dps
  IMU_GY_RANGE_500DPS  = 0x02, // +/- 500 dps
  IMU_GY_RANGE_1000DPS = 0x03, // +/- 1000 dps
  IMU_GY_RANGE_2000DPS = 0x04, // +/- 2000 dps
  IMU_GY_RANGE_4000DPS = 0x05  // +/- 4000 dps
} ImuGyRange_t;

typedef enum __IMU_OUTPUT_DATA_RATE_e {
  IMU_ODR_PWR_DWN = 0x00u, // Power-down mode
  IMU_ODR_12Hz5   = 0x01u, // 12.5 Hz
  IMU_ODR_26HZ    = 0x02u, // 26 Hz
  IMU_ODR_52HZ    = 0x03u, // 52 Hz
  IMU_ODR_104HZ   = 0x04u, // 104 Hz
  IMU_ODR_208HZ   = 0x05u, // 208 Hz
  IMU_ODR_416HZ   = 0x06u, // 416 Hz
  IMU_ODR_833HZ   = 0x07u, // 833 Hz
  IMU_ODR_1667HZ  = 0x08u, // 1667 Hz
  IMU_ODR_3333HZ  = 0x09u, // 3333 Hz
  IMU_ODR_6667HZ  = 0x0Au  // 6667 Hz
} ImuOdr_t;

/**
 * @brief IMU register addresses for the ASM330LHH.
 */
typedef enum __IMU_REGISTER_ADDRESS_e {
  IMU_ADDR_PIN_CTRL           = 0x02, // R/W
  IMU_ADDR_FIFO_CTRL1         = 0x07, // R/W
  IMU_ADDR_FIFO_CTRL2         = 0x08, // R/W
  IMU_ADDR_FIFO_CTRL3         = 0x09, // R/W
  IMU_ADDR_FIFO_CTRL4         = 0x0A, // R/W
  IMU_ADDR_COUNTER_BDR_REG1   = 0x0B, // R/W
  IMU_ADDR_COUNTER_BDR_REG2   = 0x0C, // R/W
  IMU_ADDR_INT1_CTRL          = 0x0D, // R/W
  IMU_ADDR_INT2_CTRL          = 0x0E, // R/W
  IMU_ADDR_WHO_AM_I           = 0x0F, // R
  IMU_ADDR_CTRL1_XL           = 0x10, // R/W
  IMU_ADDR_CTRL2_G            = 0x11, // R/W
  IMU_ADDR_CTRL3_C            = 0x12, // R/W
  IMU_ADDR_CTRL4_C            = 0x13, // R/W
  IMU_ADDR_CTRL5_C            = 0x14, // R/W
  IMU_ADDR_CTRL6_C            = 0x15, // R/W
  IMU_ADDR_CTRL7_G            = 0x16, // R/W
  IMU_ADDR_CTRL8_XL           = 0x17, // R/W
  IMU_ADDR_CTRL9_XL           = 0x18, // R/W
  IMU_ADDR_CTRL10_C           = 0x19, // R/W
  IMU_ADDR_ALL_INT_SRC        = 0x1A, // R
  IMU_ADDR_WAKE_UP_SRC        = 0x1B, // R
  IMU_ADDR_D6D_SRC            = 0x1D, // R
  IMU_ADDR_STATUS_REG         = 0x1E, // R
  IMU_ADDR_OUT_TEMP_L         = 0x20, // R
  IMU_ADDR_OUT_TEMP_H         = 0x21, // R
  IMU_ADDR_OUTX_L_G           = 0x22, // R
  IMU_ADDR_OUTX_H_G           = 0x23, // R
  IMU_ADDR_OUTY_L_G           = 0x24, // R
  IMU_ADDR_OUTY_H_G           = 0x25, // R
  IMU_ADDR_OUTZ_L_G           = 0x26, // R
  IMU_ADDR_OUTZ_H_G           = 0x27, // R
  IMU_ADDR_OUTX_L_A           = 0x28, // R
  IMU_ADDR_OUTX_H_A           = 0x29, // R
  IMU_ADDR_OUTY_L_A           = 0x2A, // R
  IMU_ADDR_OUTY_H_A           = 0x2B, // R
  IMU_ADDR_OUTZ_L_A           = 0x2C, // R
  IMU_ADDR_OUTZ_H_A           = 0x2D, // R
  IMU_ADDR_FIFO_STATUS1       = 0x3A, // R
  IMU_ADDR_FIFO_STATUS2       = 0x3B, // R
  IMU_ADDR_TIMESTAMP0_REG     = 0x40, // R
  IMU_ADDR_TIMESTAMP1_REG     = 0x41, // R
  IMU_ADDR_TIMESTAMP2_REG     = 0x42, // R
  IMU_ADDR_TIMESTAMP3_REG     = 0x43, // R
  IMU_ADDR_INT_CFG0           = 0x56, // R/W
  IMU_ADDR_INT_CFG1           = 0x58, // R/W
  IMU_ADDR_THS_6D             = 0x59, // R/W
  IMU_ADDR_WAKE_UP_THS        = 0x5B, // R/W
  IMU_ADDR_WAKE_UP_DUR        = 0x5C, // R/W
  IMU_ADDR_FREE_FALL          = 0x5D, // R/W
  IMU_ADDR_MD1_CFG            = 0x5E, // R/W
  IMU_ADDR_MD2_CFG            = 0x5F, // R/W
  IMU_ADDR_INTERNAL_FREQ_FINE = 0x63, // R
  IMU_ADDR_X_OFS_USR          = 0x73, // R/W
  IMU_ADDR_Y_OFS_USR          = 0x74, // R/W
  IMU_ADDR_Z_OFS_USR          = 0x75, // R/W
  IMU_ADDR_FIFO_DATA_OUT_TAG  = 0x78, // R
  IMU_ADDR_FIFO_DATA_OUT_X_L  = 0x79, // R
  IMU_ADDR_FIFO_DATA_OUT_X_H  = 0x7A, // R
  IMU_ADDR_FIFO_DATA_OUT_Y_L  = 0x7B, // R
  IMU_ADDR_FIFO_DATA_OUT_Y_H  = 0x7C, // R
  IMU_ADDR_FIFO_DATA_OUT_Z_L  = 0x7D, // R
  IMU_ADDR_FIFO_DATA_OUT_Z_H  = 0x7E  // R
} ImuRegAddress_t;

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/

ImuError_t ImuInit(SPI_HandleTypeDef *hspi);
ImuError_t ImuDeInit(void);
ImuError_t ImuReadReg(ImuRegAddress_t reg, uint8_t size);
ImuError_t ImuWriteReg(ImuRegAddress_t reg, uint8_t *data, uint8_t size);
ImuError_t ImuReadXl(void);
ImuError_t ImuReadGy(void);
ImuError_t ImuReadTemp(void);
Vec3_t *ImuGetXlData(void);
Vec3_t *ImuGetGyData(void);
float *ImuGetTemp(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* IMU_H */

/********************************* END OF FILE ********************************/
