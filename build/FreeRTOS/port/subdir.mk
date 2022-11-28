#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
ASM_SRCS += \
../FreeRTOS/port/cache.S \
../FreeRTOS/port/interrupt_s.S \
../FreeRTOS/port/mips_timer.S \
../FreeRTOS/port/port_asm.S \
../FreeRTOS/port/start.S \
../FreeRTOS/port/tlb.S

C_SRCS += \
../FreeRTOS/port/bsp_start.c \
../FreeRTOS/port/exception.c \
../FreeRTOS/port/inittlb.c \
../FreeRTOS/port/interrupt.c \
../FreeRTOS/port/port.c \
../FreeRTOS/port/port_timer.c

STARTO += ./FreeRTOS/port/start.o

OBJS += \
./FreeRTOS/port/bsp_start.o \
./FreeRTOS/port/cache.o \
./FreeRTOS/port/exception.o \
./FreeRTOS/port/inittlb.o \
./FreeRTOS/port/interrupt.o \
./FreeRTOS/port/interrupt_s.o \
./FreeRTOS/port/mips_timer.o \
./FreeRTOS/port/port.o \
./FreeRTOS/port/port_asm.o \
./FreeRTOS/port/port_timer.o \
./FreeRTOS/port/tlb.o

ASM_DEPS += \
./FreeRTOS/port/cache.d \
./FreeRTOS/port/interrupt_s.d \
./FreeRTOS/port/mips_timer.d \
./FreeRTOS/port/port_asm.d \
./FreeRTOS/port/start.d \
./FreeRTOS/port/tlb.d

C_DEPS += \
./FreeRTOS/port/bsp_start.d \
./FreeRTOS/port/exception.d \
./FreeRTOS/port/inittlb.d \
./FreeRTOS/port/interrupt.d \
./FreeRTOS/port/port.d \
./FreeRTOS/port/port_timer.d

# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/port/%.o: ../FreeRTOS/port/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: SDE Lite C Compiler'
	D:/LoongIDE/mips-2011.03/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_FREERTOS  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../FreeRTOS/port/mips" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

FreeRTOS/port/%.o: ../FreeRTOS/port/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: SDE Lite C Compiler'
	D:/LoongIDE/mips-2011.03/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_FREERTOS  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../FreeRTOS/include" -I"../FreeRTOS/port/include" -I"../FreeRTOS/port/mips" -I"../ls1x-drv/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

