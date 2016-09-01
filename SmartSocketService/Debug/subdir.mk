################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../Config.cc \
../ConnectionPool.cc \
../Main.cc \
../Message.cc \
../OracleConnection.cc \
../ProtolCodec.cc \
../SmartSocketService.cc \
../WorkThreadPool.cc 

CC_DEPS += \
./Config.d \
./ConnectionPool.d \
./Main.d \
./Message.d \
./OracleConnection.d \
./ProtolCodec.d \
./SmartSocketService.d \
./WorkThreadPool.d 

OBJS += \
./Config.o \
./ConnectionPool.o \
./Main.o \
./Message.o \
./OracleConnection.o \
./ProtolCodec.o \
./SmartSocketService.o \
./WorkThreadPool.o 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I/usr/local/include/boost -I/home/koala/oracle/11.2/client64/include -I/home/koala/muduo-1.0.7 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


