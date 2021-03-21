cmake_minimum_required(VERSION 3.10)

project(hardsid)

add_subdirectory(D2XXLib)
add_subdirectory(SIDBlasterLib)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(FTD2XX REQUIRED)

set(hardsid_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/hardsiddll/DriverDefs.h
	${CMAKE_CURRENT_SOURCE_DIR}/hardsiddll/hardsiddll.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/hardsiddll/SyncDispatcher.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/hardsiddll/SyncDispatcher.h
	${CMAKE_CURRENT_SOURCE_DIR}/hardsiddll/ThreadDispatcher.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/hardsiddll/ThreadDispatcher.h
	${CMAKE_CURRENT_SOURCE_DIR}/hardsiddll/ThreadReceiver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/hardsiddll/ThreadReceiver.h
)
if (WIN32)
    set(hardsid_EXTRA_LIBRARIES
		winmm
	)
	set(hardsid_EXTRA_SOURCES 
		${CMAKE_CURRENT_SOURCE_DIR}/hardsiddll/dllmain.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/hardsiddll/hardsid.def
	)
endif ()

add_library(hardsid SHARED 
	${hardsid_SOURCES}
	${hardsid_EXTRA_SOURCES}
)

set_property(TARGET hardsid PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(hardsid PUBLIC
  ${FTD2XX_INCLUDE_DIR}
)
target_link_libraries(hardsid 
  D2XXLib
  SIDBlasterLib
  ${FTD2XX_LIBRARY}
  ${hardsid_EXTRA_LIBRARIES}
)
