#Generated by VisualGDB (http://visualgdb.com)
#DO NOT EDIT THIS FILE MANUALLY UNLESS YOU ABSOLUTELY NEED TO
#USE VISUALGDB PROJECT PROPERTIES DIALOG INSTEAD

BINARYDIR := Debug

#Toolchain
CC := /var/jenkins/workspace/ATC_BUILDROOT/src/src_buildroot/buildroot/output/host/bin/arm-buildroot-linux-gnueabihf-gcc
CXX := /var/jenkins/workspace/ATC_BUILDROOT/src/src_buildroot/buildroot/output/host/bin/arm-buildroot-linux-gnueabihf-g++
LD := $(CXX)
AR := /var/jenkins/workspace/ATC_BUILDROOT/src/src_buildroot/buildroot/output/host/bin/arm-buildroot-linux-gnueabihf-ar
OBJCOPY := /var/jenkins/workspace/ATC_BUILDROOT/src/src_buildroot/buildroot/output/host/bin/arm-buildroot-linux-gnueabihf-objcopy

#Additional flags
PREPROCESSOR_MACROS := DEBUG=1
INCLUDE_DIRS := cpp-httplib-master czmq-master/issues mqtt_cpp-master/include rpclib-master/include SHARED/rpclib-master/include . SHARED/cpp-httplib-master
LIBRARY_DIRS := /var/jenkins/workspace/ATC_BUILDROOT/src/src_buildroot/buildroot/output/target/usr/lib/
LIBRARY_NAMES := protobuf
ADDITIONAL_LINKER_INPUTS := -pthread -lprotobuf -lpthread -ldl
MACOS_FRAMEWORKS := 
LINUX_PACKAGES := 

CFLAGS := -ggdb -ffunction-sections -O0 -std=c++17
CXXFLAGS := -ggdb -ffunction-sections -O0 -std=c++17
ASFLAGS := 
LDFLAGS := -Wl,-gc-sections
COMMONFLAGS := 
LINKER_SCRIPT := 

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group

#Additional options detected from testing the toolchain
IS_LINUX_PROJECT := 1