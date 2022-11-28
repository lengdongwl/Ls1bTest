#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../libc/string/memchr.c \
../libc/string/memcmp.c \
../libc/string/memcpy.c \
../libc/string/memmove.c \
../libc/string/memset.c \
../libc/string/strcasecmp.c \
../libc/string/strcat.c \
../libc/string/strchr.c \
../libc/string/strcmp.c \
../libc/string/strcpy.c \
../libc/string/strdup.c \
../libc/string/strerror.c \
../libc/string/strlen.c \
../libc/string/strncasecmp.c \
../libc/string/strncat.c \
../libc/string/strncmp.c \
../libc/string/strncpy.c \
../libc/string/strnlen.c

OBJS += \
./libc/string/memchr.o \
./libc/string/memcmp.o \
./libc/string/memcpy.o \
./libc/string/memmove.o \
./libc/string/memset.o \
./libc/string/strcasecmp.o \
./libc/string/strcat.o \
./libc/string/strchr.o \
./libc/string/strcmp.o \
./libc/string/strcpy.o \
./libc/string/strdup.o \
./libc/string/strerror.o \
./libc/string/strlen.o \
./libc/string/strncasecmp.o \
./libc/string/strncat.o \
./libc/string/strncmp.o \
./libc/string/strncpy.o \
./libc/string/strnlen.o

C_DEPS += \
./libc/string/memchr.d \
./libc/string/memcmp.d \
./libc/string/memcpy.d \
./libc/string/memmove.d \
./libc/string/memset.d \
./libc/string/strcasecmp.d \
./libc/string/strcat.d \
./libc/string/strchr.d \
./libc/string/strcmp.d \
./libc/string/strcpy.d \
./libc/string/strdup.d \
./libc/string/strerror.d \
./libc/string/strlen.d \
./libc/string/strncasecmp.d \
./libc/string/strncat.d \
./libc/string/strncmp.d \
./libc/string/strncpy.d \
./libc/string/strnlen.d

# Each subdirectory must supply rules for building sources it contributes
libc/string/%.o: ../libc/string/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: SDE Lite C Compiler'
	D:/LoongIDE/mips-2011.03/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_FREERTOS  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../FreeRTOS/include" -I"../FreeRTOS/port/include" -I"../FreeRTOS/port/mips" -I"../ls1x-drv/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

