################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/lucciu/canfestival-3-asc/drivers/AVR/can_AVR.c \
/home/lucciu/canfestival-3-asc/drivers/AVR/timer_AVR.c 

OBJS += \
./canfestival-3-asc/drivers/AVR/can_AVR.o \
./canfestival-3-asc/drivers/AVR/timer_AVR.o 

C_DEPS += \
./canfestival-3-asc/drivers/AVR/can_AVR.d \
./canfestival-3-asc/drivers/AVR/timer_AVR.d 


# Each subdirectory must supply rules for building sources it contributes
canfestival-3-asc/drivers/AVR/can_AVR.o: /home/lucciu/canfestival-3-asc/drivers/AVR/can_AVR.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -I/home/lucciu/canfestival-3-asc/include -I/usr/src/linux-headers-4.4.0-45-generic/include -I/usr/src/linux-headers-4.4.0-45-generic/include/linux -I/usr/src/linux-headers-4.4.0-45-generic/include/asm -I/home/lucciu/canfestival-3-asc/include/unix -I/home/lucciu/canfestival-3-asc/include/timers_unix -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

canfestival-3-asc/drivers/AVR/timer_AVR.o: /home/lucciu/canfestival-3-asc/drivers/AVR/timer_AVR.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -I/home/lucciu/canfestival-3-asc/include -I/usr/src/linux-headers-4.4.0-45-generic/include -I/usr/src/linux-headers-4.4.0-45-generic/include/linux -I/usr/src/linux-headers-4.4.0-45-generic/include/asm -I/home/lucciu/canfestival-3-asc/include/unix -I/home/lucciu/canfestival-3-asc/include/timers_unix -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


