################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/Modules/eORB/eORB.cpp 

OBJS += \
./Core/Src/Modules/eORB/eORB.o 

CPP_DEPS += \
./Core/Src/Modules/eORB/eORB.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Modules/eORB/%.o: ../Core/Src/Modules/eORB/%.cpp Core/Src/Modules/eORB/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"C:/Users/timot/STM32CubeIDE/workspace_1.8.0/nucleo-f411re-rover/Core/Src/Modules/Blinky" -I"C:/Users/timot/STM32CubeIDE/workspace_1.8.0/nucleo-f411re-rover/Core/Src/Modules/eORB" -I"C:/Users/timot/STM32CubeIDE/workspace_1.8.0/nucleo-f411re-rover/Core/Src/Modules/Serial" -I"C:/Users/timot/STM32CubeIDE/workspace_1.8.0/nucleo-f411re-rover/Core/Src/Modules/TaskManager" -I"C:/Users/timot/STM32CubeIDE/workspace_1.8.0/nucleo-f411re-rover/Core/Src/Modules/BlinkSetter" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Modules-2f-eORB

clean-Core-2f-Src-2f-Modules-2f-eORB:
	-$(RM) ./Core/Src/Modules/eORB/eORB.d ./Core/Src/Modules/eORB/eORB.o

.PHONY: clean-Core-2f-Src-2f-Modules-2f-eORB

