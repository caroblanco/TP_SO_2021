################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/gestion_bloques.c \
../src/imongo.c \
../src/init_file-system.c \
../src/mmap.c \
../src/pruebas.c \
../src/sabotajes.c 

OBJS += \
./src/gestion_bloques.o \
./src/imongo.o \
./src/init_file-system.o \
./src/mmap.o \
./src/pruebas.o \
./src/sabotajes.o 

C_DEPS += \
./src/gestion_bloques.d \
./src/imongo.d \
./src/init_file-system.d \
./src/mmap.d \
./src/pruebas.d \
./src/sabotajes.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2021-1c-Los_Brogrammers/utiles" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


