cmake_minimum_required(VERSION 3.10)

project(SIDBlasterLib)

set(CMAKE_CXX_STANDARD 11)

set(SIDBlasterLib_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/CommandDispatcher.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/CommandDispatcher.h
	${CMAKE_CURRENT_SOURCE_DIR}/CommandReceiver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/CommandReceiver.h
	${CMAKE_CURRENT_SOURCE_DIR}/DebugSIDBlasterInterface.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/DebugSIDBlasterInterface.h
	${CMAKE_CURRENT_SOURCE_DIR}/ILogger.h
	${CMAKE_CURRENT_SOURCE_DIR}/SIDBlasterInterface.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SIDBlasterInterface.h
	${CMAKE_CURRENT_SOURCE_DIR}/SIDBlasterInterfaceImpl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SIDBlasterInterfaceImpl.h
)

add_library(SIDBlasterLib STATIC ${SIDBlasterLib_SOURCES})

set_property(TARGET SIDBlasterLib PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(SIDBlasterLib PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../D2XXLib/include
)
