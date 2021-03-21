cmake_minimum_required(VERSION 3.10)

project(D2XXLib)

set(CMAKE_CXX_STANDARD 11)

set(D2XXLib_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/D2XXDevice.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/D2XXDeviceManager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/D2XXLib/D2XXDevice.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/D2XXLib/D2XXDeviceManager.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/D2XXLib/Utilities.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/ftd2xx.h
)

add_library(D2XXLib STATIC ${D2XXLib_SOURCES})

set_property(TARGET D2XXLib PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(D2XXLib PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
