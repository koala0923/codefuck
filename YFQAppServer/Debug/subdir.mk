################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../Main.cc \
../Message.cc \
../ProtolCodec.cc \
../WorkThreadPool.cc \
../YFQAppServer.cc \
../YFQAppService.pb.cc \
../YfqUser.cc 

CC_DEPS += \
./Main.d \
./Message.d \
./ProtolCodec.d \
./WorkThreadPool.d \
./YFQAppServer.d \
./YFQAppService.pb.d \
./YfqUser.d 

OBJS += \
./Main.o \
./Message.o \
./ProtolCodec.o \
./WorkThreadPool.o \
./YFQAppServer.o \
./YFQAppService.pb.o \
./YfqUser.o 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I/usr/local/include/boost -I/usr/local/include -I/home/koala/muduo-1.0.7 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


