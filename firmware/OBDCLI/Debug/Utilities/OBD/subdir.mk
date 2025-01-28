################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Utilities/OBD/obd.c 

OBJS += \
./Utilities/OBD/obd.o 

C_DEPS += \
./Utilities/OBD/obd.d 


# Each subdirectory must supply rules for building sources it contributes
Utilities/OBD/%.o Utilities/OBD/%.su Utilities/OBD/%.cyclo: ../Utilities/OBD/%.c Utilities/OBD/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L412xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"D:/Development/OBDADS/firmware/OBDCLI/Utilities/OBD" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Utilities-2f-OBD

clean-Utilities-2f-OBD:
	-$(RM) ./Utilities/OBD/obd.cyclo ./Utilities/OBD/obd.d ./Utilities/OBD/obd.o ./Utilities/OBD/obd.su

.PHONY: clean-Utilities-2f-OBD

