################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
/home/lucciu/Mongo-canfestival-3-asc-e07f8b94110e/drivers/timers_unix/timers_unix.o 

C_SRCS += \
/home/lucciu/Mongo-canfestival-3-asc-e07f8b94110e/drivers/timers_unix/timers_unix.c 

OBJS += \
./Mongo-canfestival-3-asc-e07f8b94110e/drivers/timers_unix/timers_unix.o 

C_DEPS += \
./Mongo-canfestival-3-asc-e07f8b94110e/drivers/timers_unix/timers_unix.d 


# Each subdirectory must supply rules for building sources it contributes
Mongo-canfestival-3-asc-e07f8b94110e/drivers/timers_unix/timers_unix.o: /home/lucciu/Mongo-canfestival-3-asc-e07f8b94110e/drivers/timers_unix/timers_unix.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -I/home/lucciu/Mongo-canfestival-3-asc-e07f8b94110e/include -I/home/lucciu/Mongo-canfestival-3-asc-e07f8b94110e/include/unix -I/home/lucciu/Mongo-canfestival-3-asc-e07f8b94110e/include/timers_unix -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


