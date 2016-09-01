################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../StreamService.pb.cc 

CPP_SRCS += \
../Config.cpp \
../EventLoop.cpp \
../PduBase.cpp \
../StrService.cpp \
../StrUser.cpp \
../TcpConnect.cpp \
../TcpService.cpp \
../UdpService.cpp \
../UtilPdu.cpp \
../log4z.cpp 

CC_DEPS += \
./StreamService.pb.d 

OBJS += \
./Config.o \
./EventLoop.o \
./PduBase.o \
./StrService.o \
./StrUser.o \
./StreamService.pb.o \
./TcpConnect.o \
./TcpService.o \
./UdpService.o \
./UtilPdu.o \
./log4z.o 

CPP_DEPS += \
./Config.d \
./EventLoop.d \
./PduBase.d \
./StrService.d \
./StrUser.d \
./TcpConnect.d \
./TcpService.d \
./UdpService.d \
./UtilPdu.d \
./log4z.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I/usr/local/include/event2 -I/usr/local/include/google/protobuf -I/usr/local/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I/usr/local/include/event2 -I/usr/local/include/google/protobuf -I/usr/local/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


