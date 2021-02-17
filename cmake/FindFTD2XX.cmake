# - Find FTD2XX installation
# This module tries to find the libftd2xx installation on your system.
# Once done this will define
#
#  FTD2XX_FOUND - system has ftd2xx
#  FTD2XX_INCLUDE_DIR - ~ the ftd2xx include directory 
#  FTD2XX_LIBRARY - Link these to use ftd2xx

if (WIN32)
    set(FTD2XX_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
else ()
	FIND_PATH(FTD2XX_INCLUDE_DIR 
	NAMES   ftd2xx.h
	PATHS   /usr/local/include
			/usr/include
			/usr/include/libftd2xx
			/usr/local/include/libftd2xx
			/opt/local/include
			/sw/include
	)
endif()

# determine if we run a 64bit compiler or not
set(bitness i386)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(bitness amd64)
endif()

SET(FTD2XX_LIBNAME ftd2xx)
IF(WIN32)
  SET(FTD2XX_LIBNAME ftd2xx.lib)
ENDIF(WIN32)

if (WIN32)
	set(FTD2XX_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/lib/${bitness}/${FTD2XX_LIBNAME})
else ()
	FIND_LIBRARY(FTD2XX_LIBRARY
	NAMES ${FTD2XX_LIBNAME}
	PATHS 	/usr/lib
			/usr/local/lib
			/opt/local/lib
			/sw/lib
	)
endif()

IF (FTD2XX_LIBRARY)
  IF(FTD2XX_INCLUDE_DIR)
      SET(FTD2XX_FOUND TRUE)
      MESSAGE(STATUS "Found libFTD2XX: ${FTD2XX_INCLUDE_DIR}, ${FTD2XX_LIBRARY}")
  ELSE(FTD2XX_INCLUDE_DIR)
    SET(FTD2XX_FOUND FALSE)
    MESSAGE(STATUS "libFTD2XX headers NOT FOUND. Make sure to install the development headers! Please refer to the documentation for instructions.")
  ENDIF(FTD2XX_INCLUDE_DIR)
ELSE (FTD2XX_LIBRARY)
    SET(FTD2XX_FOUND FALSE)
    MESSAGE(STATUS "libFTD2XX NOT FOUND.")
ENDIF (FTD2XX_LIBRARY)

SET(FTD2XX_INCLUDE_DIR
    ${FTD2XX_INCLUDE_DIR}
)