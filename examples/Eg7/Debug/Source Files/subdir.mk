################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source\ Files/lpc17xx_clkpwr.c \
../Source\ Files/lpc17xx_gpio.c \
../Source\ Files/lpc17xx_nvic.c \
../Source\ Files/lpc17xx_pinsel.c \
../Source\ Files/lpc17xx_systick.c \
../Source\ Files/lpc17xx_wdt.c \
../Source\ Files/lpc_global.c \
../Source\ Files/lpc_system_init.c \
../Source\ Files/main.c 

OBJS += \
./Source\ Files/lpc17xx_clkpwr.o \
./Source\ Files/lpc17xx_gpio.o \
./Source\ Files/lpc17xx_nvic.o \
./Source\ Files/lpc17xx_pinsel.o \
./Source\ Files/lpc17xx_systick.o \
./Source\ Files/lpc17xx_wdt.o \
./Source\ Files/lpc_global.o \
./Source\ Files/lpc_system_init.o \
./Source\ Files/main.o 

C_DEPS += \
./Source\ Files/lpc17xx_clkpwr.d \
./Source\ Files/lpc17xx_gpio.d \
./Source\ Files/lpc17xx_nvic.d \
./Source\ Files/lpc17xx_pinsel.d \
./Source\ Files/lpc17xx_systick.d \
./Source\ Files/lpc17xx_wdt.d \
./Source\ Files/lpc_global.d \
./Source\ Files/lpc_system_init.d \
./Source\ Files/main.d 


# Each subdirectory must supply rules for building sources it contributes
Source\ Files/lpc17xx_clkpwr.o: ../Source\ Files/lpc17xx_clkpwr.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Windows GCC C Compiler'
	arm-none-eabi-gcc -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\arm-none-eabi\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include-fixed" -I"C:\E_Workspace\LPC1768\Eg7\Header Files" -I"C:\E_Workspace\LPC1768\Eg7\CM3 Core" -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"Source Files/lpc17xx_clkpwr.d" -MT"Source\ Files/lpc17xx_clkpwr.d" -mcpu=cortex-m3 -mthumb -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Source\ Files/lpc17xx_gpio.o: ../Source\ Files/lpc17xx_gpio.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Windows GCC C Compiler'
	arm-none-eabi-gcc -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\arm-none-eabi\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include-fixed" -I"C:\E_Workspace\LPC1768\Eg7\Header Files" -I"C:\E_Workspace\LPC1768\Eg7\CM3 Core" -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"Source Files/lpc17xx_gpio.d" -MT"Source\ Files/lpc17xx_gpio.d" -mcpu=cortex-m3 -mthumb -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Source\ Files/lpc17xx_nvic.o: ../Source\ Files/lpc17xx_nvic.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Windows GCC C Compiler'
	arm-none-eabi-gcc -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\arm-none-eabi\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include-fixed" -I"C:\E_Workspace\LPC1768\Eg7\Header Files" -I"C:\E_Workspace\LPC1768\Eg7\CM3 Core" -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"Source Files/lpc17xx_nvic.d" -MT"Source\ Files/lpc17xx_nvic.d" -mcpu=cortex-m3 -mthumb -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Source\ Files/lpc17xx_pinsel.o: ../Source\ Files/lpc17xx_pinsel.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Windows GCC C Compiler'
	arm-none-eabi-gcc -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\arm-none-eabi\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include-fixed" -I"C:\E_Workspace\LPC1768\Eg7\Header Files" -I"C:\E_Workspace\LPC1768\Eg7\CM3 Core" -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"Source Files/lpc17xx_pinsel.d" -MT"Source\ Files/lpc17xx_pinsel.d" -mcpu=cortex-m3 -mthumb -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Source\ Files/lpc17xx_systick.o: ../Source\ Files/lpc17xx_systick.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Windows GCC C Compiler'
	arm-none-eabi-gcc -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\arm-none-eabi\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include-fixed" -I"C:\E_Workspace\LPC1768\Eg7\Header Files" -I"C:\E_Workspace\LPC1768\Eg7\CM3 Core" -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"Source Files/lpc17xx_systick.d" -MT"Source\ Files/lpc17xx_systick.d" -mcpu=cortex-m3 -mthumb -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Source\ Files/lpc17xx_wdt.o: ../Source\ Files/lpc17xx_wdt.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Windows GCC C Compiler'
	arm-none-eabi-gcc -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\arm-none-eabi\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include-fixed" -I"C:\E_Workspace\LPC1768\Eg7\Header Files" -I"C:\E_Workspace\LPC1768\Eg7\CM3 Core" -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"Source Files/lpc17xx_wdt.d" -MT"Source\ Files/lpc17xx_wdt.d" -mcpu=cortex-m3 -mthumb -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Source\ Files/lpc_global.o: ../Source\ Files/lpc_global.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Windows GCC C Compiler'
	arm-none-eabi-gcc -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\arm-none-eabi\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include-fixed" -I"C:\E_Workspace\LPC1768\Eg7\Header Files" -I"C:\E_Workspace\LPC1768\Eg7\CM3 Core" -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"Source Files/lpc_global.d" -MT"Source\ Files/lpc_global.d" -mcpu=cortex-m3 -mthumb -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Source\ Files/lpc_system_init.o: ../Source\ Files/lpc_system_init.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Windows GCC C Compiler'
	arm-none-eabi-gcc -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\arm-none-eabi\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include-fixed" -I"C:\E_Workspace\LPC1768\Eg7\Header Files" -I"C:\E_Workspace\LPC1768\Eg7\CM3 Core" -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"Source Files/lpc_system_init.d" -MT"Source\ Files/lpc_system_init.d" -mcpu=cortex-m3 -mthumb -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Source\ Files/main.o: ../Source\ Files/main.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Windows GCC C Compiler'
	arm-none-eabi-gcc -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\arm-none-eabi\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include" -I"C:\Program Files\GNU Tools ARM Embedded\4.7 2013q2\lib\gcc\arm-none-eabi\4.7.4\include-fixed" -I"C:\E_Workspace\LPC1768\Eg7\Header Files" -I"C:\E_Workspace\LPC1768\Eg7\CM3 Core" -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"Source Files/main.d" -MT"Source\ Files/main.d" -mcpu=cortex-m3 -mthumb -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


