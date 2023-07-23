#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../libc/lwmem.c \
../libc/putchar.c

OBJS += \
./libc/lwmem.o \
./libc/putchar.o

C_DEPS += \
./libc/lwmem.d \
./libc/putchar.d

# Each subdirectory must supply rules for building sources it contributes
libc/%.o: ../libc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MIPS SDE Lite C Compiler'
	D:/LoongIDE/mips-2011.03/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_FREERTOS  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../FreeRTOS/include" -I"../FreeRTOS/port/include" -I"../FreeRTOS/port/mips" -I"../ls1x-drv/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

