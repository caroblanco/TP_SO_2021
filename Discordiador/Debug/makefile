################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: discordiador

dependents:
	-cd /home/utnso/tp-2021-1c-Los_Brogrammers/utiles/Debug && $(MAKE) all

# Tool invocations
discordiador: $(OBJS) $(USER_OBJS) /home/utnso/tp-2021-1c-Los_Brogrammers/utiles/Debug/libutiles.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/tp-2021-1c-Los_Brogrammers/utiles/Debug" -L/home/utnso/tp-2021-1c-Los_Brogrammers/utiles/Debug -L/home/utnso/tp-2021-1c-Los_Brogrammers/utiles -o "discordiador" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) discordiador
	-@echo ' '

.PHONY: all clean dependents
/home/utnso/tp-2021-1c-Los_Brogrammers/utiles/Debug/libutiles.so:

-include ../makefile.targets
