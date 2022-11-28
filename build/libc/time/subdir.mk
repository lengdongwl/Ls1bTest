#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../libc/time/gmtime.c \
../libc/time/localtime.c \
../libc/time/mktime.c \
../libc/time/strftime.c \
../libc/time/time.c \
../libc/time/tm_utils.c

OBJS += \
./libc/time/gmtime.o \
./libc/time/localtime.o \
./libc/time/mktime.o \
./libc/time/strftime.o \
./libc/time/time.o \
./libc/time/tm_utils.o

C_DEPS += \
./libc/time/gmtime.d \
./libc/time/localtime.d \
./libc/time/mktime.d \
./libc/time/strftime.d \
./libc/time/time.d \
./libc/time/tm_utils.d

# Each subdirectory must supply rules for building sources it contributes
libc/time/%.o: ../libc/time/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: SDE Lite C Compiler'
	D:/LoongIDE/mips-2011.03/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_FREERTOS  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../FreeRTOS/include" -I"../FreeRTOS/port/include" -I"../FreeRTOS/port/mips" -I"../ls1x-drv/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

