################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/consola.c \
../src/discordiador.c \
../src/planificador.c \
../src/tripulante.c 

OBJS += \
./src/consola.o \
./src/discordiador.o \
./src/planificador.o \
./src/tripulante.o 

C_DEPS += \
./src/consola.d \
./src/discordiador.d \
./src/planificador.d \
./src/tripulante.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2021-1c-Los_Brogrammers/utiles" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


