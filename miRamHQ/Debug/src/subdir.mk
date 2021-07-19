################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/mPaginacion.c \
../src/mSegmentacion.c \
../src/mapa.c \
../src/memoria.c \
../src/mi-ram.c \
../src/opComunes.c 

OBJS += \
./src/mPaginacion.o \
./src/mSegmentacion.o \
./src/mapa.o \
./src/memoria.o \
./src/mi-ram.o \
./src/opComunes.o 

C_DEPS += \
./src/mPaginacion.d \
./src/mSegmentacion.d \
./src/mapa.d \
./src/memoria.d \
./src/mi-ram.d \
./src/opComunes.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2021-1c-Los_Brogrammers/utiles" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


