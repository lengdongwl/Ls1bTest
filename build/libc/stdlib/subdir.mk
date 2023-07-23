#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../libc/stdlib/aligned_free.c \
../libc/stdlib/aligned_malloc.c \
../libc/stdlib/atoi.c \
../libc/stdlib/atol.c \
../libc/stdlib/calloc.c \
../libc/stdlib/div.c \
../libc/stdlib/free.c \
../libc/stdlib/malloc.c \
../libc/stdlib/realloc.c \
../libc/stdlib/strtod.c \
../libc/stdlib/strtol.c \
../libc/stdlib/strtoul.c

OBJS += \
./libc/stdlib/aligned_free.o \
./libc/stdlib/aligned_malloc.o \
./libc/stdlib/atoi.o \
./libc/stdlib/atol.o \
./libc/stdlib/calloc.o \
./libc/stdlib/div.o \
./libc/stdlib/free.o \
./libc/stdlib/malloc.o \
./libc/stdlib/realloc.o \
./libc/stdlib/strtod.o \
./libc/stdlib/strtol.o \
./libc/stdlib/strtoul.o

C_DEPS += \
./libc/stdlib/aligned_free.d \
./libc/stdlib/aligned_malloc.d \
./libc/stdlib/atoi.d \
./libc/stdlib/atol.d \
./libc/stdlib/calloc.d \
./libc/stdlib/div.d \
./libc/stdlib/free.d \
./libc/stdlib/malloc.d \
./libc/stdlib/realloc.d \
./libc/stdlib/strtod.d \
./libc/stdlib/strtol.d \
./libc/stdlib/strtoul.d

# Each subdirectory must supply rules for building sources it contributes
libc/stdlib/%.o: ../libc/stdlib/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MIPS SDE Lite C Compiler'
	D:/LoongIDE/mips-2011.03/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_FREERTOS  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../FreeRTOS/include" -I"../FreeRTOS/port/include" -I"../FreeRTOS/port/mips" -I"../ls1x-drv/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

