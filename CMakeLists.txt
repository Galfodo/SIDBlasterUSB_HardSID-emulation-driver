cmake_minimum_required(VERSION 3.10)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")
# Universal binary (Intel + Apple Silicon) - CMake automatically raises the
# effective minimum for the arm64 slice to whatever macOS actually requires
# for that architecture (11.0+), CMAKE_OSX_DEPLOYMENT_TARGET above only
# constrains the x86_64 slice. x86_64 slice verified on real Mac hardware
# via real SIDBlaster-USB playback; arm64 slice compiles cleanly but is
# only build-tested so far (built from an Intel VM, no Apple Silicon
# hardware available yet).
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build a universal binary on macOS")

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

if (APPLE)
    add_custom_command(TARGET hardsid 
        POST_BUILD COMMAND 
        ${CMAKE_INSTALL_NAME_TOOL} -change libftd2xx.dylib /usr/local/lib/libftd2xx.dylib
        $<TARGET_FILE:hardsid>)
endif ()

add_custom_command(TARGET hardsid POST_BUILD COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/hardsiddll/hardsid.hpp ${PROJECT_BINARY_DIR})

if (NOT WIN32)
	# AIASS/ASID Player's Linux/Mac paths both hardcode
	# /usr/local/lib/libhardsid.so|.dylib (see their Sid.cpp), so install
	# there explicitly rather than relying on CMAKE_INSTALL_PREFIX
	# defaulting to the same place - makes `cmake --install build` (as
	# root/sudo, or with a prefix override) do the right thing regardless
	# of how the prefix is configured.
	install(TARGETS hardsid LIBRARY DESTINATION /usr/local/lib)
endif ()
