# OBD-based Auto Dynamics System Firmware
> This repository contains resources, documentation, and project files for the project firmware.

## Top Pins

### CN2 Odd Pins (Pins closest to the center of the board)
| Pin # | STM32 Pin | Signal             | Notes | Used | Usage    |
|:-----:|:---------:|:------------------:|:-----:|:----:|:--------:|
|     1 | -         | 3V3                |       | -    |          |
|     3 | PA12      | GPIO2, I2C5_SDA    |       | N    |          |
|     5 | PA11      | GPIO3, I2C5_SCL    |       | N    |          |
|     7 | PA8       | GPIO4, MCO1        |       | N    |          |
|     9 | -         | GND                |       | -    |          |
|    11 | PG8       | GPIO17, USART3_RTS |       | N    |          |
|    13 | PD7       | GPIO27, SDMMC3_D3  |       | Y    | SD Card  |
|    15 | PG15      | GPIO2, SDMMC3_CK   |       | Y    | SD Card  |
|    17 | -         | 3V3                |       | -    |          |
|    19 | PF9       | GPIO10, SPI5_MOSI  |       | Y    | IMU      |
|    21 | PF8       | GPIO9, SPI5_MISO   |       | Y    | IMU      |
|    23 | PF7       | GPIO11, SPI5_SCK   |       | Y    | IMU      |
|    25 | -         | GND                |       | -    |          |
|    27 | PF15      | I2C1_SDA           |       | N    |          |
|    29 | PG2       | GPIO5, MCO2        |       | Y    | IMU      |
|    31 | PH11      | GPIO6, TIM5_CH2    |       | N    |          |
|    33 | PC7       | GPIO13, TIM3_CH2   |       | N    |          |
|    35 | PI7       | GPIO19, SAI2_FSA   |       | N    |          |
|    37 | PF5       | GPIO26, SDMMC3_D2  |       | Y    | SD Card  |
|    39 | -         | GND                |       | -    |          |

### CN2 Even Pins (Pins closest to the edge of the board)
| Pin # | STM32 Pin | Signal                        | Notes | Used | Usage    |
|:-----:|:---------:|:-----------------------------:|:-----:|:----:|:--------:|
|     2 | -         | 5V                            |       | -    |          |
|     4 | -         | 5V                            |       | -    |          |
|     6 | -         | GND                           |       | -    |          |
|     8 | PB10      | GPIO14, USART3_TX             |       | N    |          |
|    10 | PB12      | GPIO15, USART3_RX, FDCAN2_RX  |       | Y    | OBD Port |
|    12 | PI5       | GPIO18, SAI2_SCKA             |       | N    |          |
|    14 | -         | GND                           |       | -    |          |
|    16 | PF1       | GPIO23, SDMMC3_CMD            |       | Y    | SD Card  |
|    18 | PF0       | GPIO24, SDMMC3_D0             |       | Y    | SD Card  |
|    20 | -         | GND                           |       | -    |          |
|    22 | PF4       | GPIO25, SDMMC3_D1             |       | Y    | SD Card  |
|    24 | PF6       | GPIO8, SPI5_NSS               |       | Y    | IMU      |
|    26 | PF3       | GPIO7                         |       | Y    | IMU      |
|    28 | PD12      | I2C1_SCL                      |       | N    |          |
|    30 | -         | GND                           |       | -    |          |
|    32 | PD13      | GPIO12, TIM4_CH2              |       | N    |          |
|    34 | -         | GND                           |       | -    |          |
|    36 | PB13      | GPIO16, USART3_CTS, FDCAN2_TX |       | Y    | OBD Port |
|    38 | PI6       | GPIO20, SAI2_SDA              |       | N    |          |
|    40 | PF11      | GPIO21, SAI2_SDB              |       | N    |          |

## Bottom Pins

### CN16
| Pin # | STM32 Pin | Signal | Notes | Used | Usage |
| ----- | --------- | ------ | ----- |:----:|:-----:|
| 1     | -         | NC     | NC    | N    |       |
| 2     | -         | 3.3v   | IOREF | N    |       |
| 3     | NRST      | NRST   |       | N    |       |
| 4     | 3.3v      | -      |       | -    |       |
| 5     | 5v        | -      |       | -    |       |
| 6     | GND       | -      |       | -    |       |
| 7     | GND       | -      |       | -    |       |
| 8     | VIN       | -      | NC    | N    |       |

### CN17
| Pin # | STM32 Pin  | Signal | Notes              | Used | Usage |
|:-----:|:----------:|:------:|:------------------:|:----:|:-----:|
| 1     | PF14       | A0     | ADC2_IN6           | N    |       |
| 2     | PF13       | A1     | ADC2_IN2           | N    |       |
| 3     | ANA0       | A2     | ADC1_IN0, ADC2_IN0 | N    |       |
| 4     | ANA1       | A3     | ADC1_IN1, ADC2_IN1 | N    |       |
| 5     | PC3, PA12  | A4     | ADC1_IN13 (PC3)    | N    |       |
| 6     | PF12, PA11 | A5     | ADC1_IN6 (PF12)    | N    |       |

### CN14
| Pin # | STM32 Pin | Signal | Notes     | Used | Usage            |
|:-----:|:---------:|:------:|:---------:|:----:|:----------------:|
| 1     | PE7       | ARD_D0 | USART7_RX | Y    | Virtual COM Port |
| 2     | PE8       | ARD_D1 | USART7_TX | Y    | Virtual COM Port |
| 3     | PE1       | ARD_D2 | GPIO      | N    |                  |
| 4     | PD14      | ARD_D3 | TIM4_CH3  | N    |                  |
| 5     | PE10      | ARD_D4 | GPIO      | N    |                  |
| 6     | PD15      | ARD_D5 | TIM4_CH4  | N    |                  |
| 7     | PE9       | ARD_D6 | TIM1_CH1  | N    |                  |
| 8     | PD1       | ARD_D7 | GPIO      | Y    | Debug LED        |

### CN13
| Pin # | STM32 Pin | Signal  | Notes               | Used | Usage             |
|:-----:|:---------:|:-------:|:-------------------:|:----:|:-----------------:|
| 1     | PG3       | ARD_D8  | GPIO                | Y    | Error LED         |
| 2     | PH6       | ARD_D9  | TIM12_CH1           | Y    | State machine LED |
| 3     | PE11      | ARD_D10 | SPI4_NSS, TIM1_CH2  | N    |                   |
| 4     | PE14      | ARD_D11 | SPI4_MOSI, TIM1_CH4 | N    |                   |
| 5     | PE13      | ARD_D12 | SPI4_MISO           | N    |                   |
| 6     | PE12      | ARD_D13 | SPI4_CLK            | N    |                   |
| 7     | -         | GND     | GND                 | N    |                   |
| 8     | -         | VREFP   | VREF+               | N    |                   |
| 9     | PA12      | ARD_D14 | I2C5_SDA            | N    |                   |
| 10    | PA11      | ARD_D15 | I2C5_CLK            | N    |                   |
