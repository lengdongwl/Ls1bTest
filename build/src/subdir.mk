#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../src/install_3th_libraries.c \
../src/key.c \
../src/led.c \
../src/SMG.c \
../src/beep.c \
../src/pwm_ic.c \
../src/bh1750.c \
../src/bh1750_iic.c \
../src/hmc5883l_drv.c \
../src/ultrasonic_ranging_drv.c \
../src/fan_resistance_control_drv.c \
../src/uart.c \
../src/bkrc_voice.c \
../src/motor_drv.c \
../src/adc.c \
../src/mytask.c \
../src/MLX90614.c \
../src/rc522.c

OBJS += \
./src/install_3th_libraries.o \
./src/key.o \
./src/led.o \
./src/SMG.o \
./src/beep.o \
./src/pwm_ic.o \
./src/bh1750.o \
./src/bh1750_iic.o \
./src/hmc5883l_drv.o \
./src/ultrasonic_ranging_drv.o \
./src/fan_resistance_control_drv.o \
./src/uart.o \
./src/bkrc_voice.o \
./src/motor_drv.o \
./src/adc.o \
./src/mytask.o \
./src/MLX90614.o \
./src/rc522.o

C_DEPS += \
./src/install_3th_libraries.d \
./src/key.d \
./src/led.d \
./src/SMG.d \
./src/beep.d \
./src/pwm_ic.d \
./src/bh1750.d \
./src/bh1750_iic.d \
./src/hmc5883l_drv.d \
./src/ultrasonic_ranging_drv.d \
./src/fan_resistance_control_drv.d \
./src/uart.d \
./src/bkrc_voice.d \
./src/motor_drv.d \
./src/adc.d \
./src/mytask.d \
./src/MLX90614.d \
./src/rc522.d

# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MIPS SDE Lite C Compiler'
	D:/LoongIDE/mips-2011.03/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_FREERTOS  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../FreeRTOS/include" -I"../FreeRTOS/port/include" -I"../FreeRTOS/port/mips" -I"../ls1x-drv/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

