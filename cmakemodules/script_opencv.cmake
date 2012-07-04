# Check for the OpenCV libraries:
#  pkg-config if available (Linux), otherwise CMake module
# =========================================================
SET(CMAKE_MRPT_HAS_OPENCV 0)
SET(MRPT_OPENCV_VERSION 0.0.0)
SET(MRPT_OPENCV_VERSION_HEX "0x000")
SET(MRPT_OPENCV_SRC_DIR "")
SET(OpenCV_IGNORE_PKGCONFIG OFF CACHE BOOL "Forces using OpenCVConfig.cmake to find OpenCV")
MARK_AS_ADVANCED(OpenCV_IGNORE_PKGCONFIG)

# Invoke pkg-config for getting the configuration:
IF(PKG_CONFIG_FOUND AND NOT OpenCV_IGNORE_PKGCONFIG)
	PKG_CHECK_MODULES(OPENCV opencv)
	IF(OPENCV_FOUND)
		SET(CMAKE_MRPT_HAS_OPENCV 1)
		SET(MRPT_OPENCV_VERSION ${OPENCV_VERSION})

		IF ("${OPENCV_INCLUDEDIR}")
			INCLUDE_DIRECTORIES(${OPENCV_INCLUDEDIR})
		ENDIF("${OPENCV_INCLUDEDIR}")
		IF ("${OPENCV_LIBDIR}")
			LINK_DIRECTORIES(${OPENCV_LIBDIR})
		ENDIF ("${OPENCV_LIBDIR}")
		SET(OpenCV_LIBRARIES ${OPENCV_LIBRARIES})

		MESSAGE(STATUS " opencv include: ${OPENCV_INCLUDE_DIRS} (Version: ${OPENCV_VERSION})")
		INCLUDE_DIRECTORIES(${OPENCV_INCLUDE_DIRS})
		
#		SET(MRPT_LINKER_LIBS_FOR_PKGCONFIG "${MRPT_LINKER_LIBS_FOR_PKGCONFIG} ${opencv_LDFLAGS}")
	ENDIF(OPENCV_FOUND)
ENDIF(PKG_CONFIG_FOUND AND NOT OpenCV_IGNORE_PKGCONFIG)

# Use CMAKE module if opencv's not been detected yet:
IF(NOT CMAKE_MRPT_HAS_OPENCV)
	# 1st: Try to find OpenCV config file (NO_MODULE: Don't find a module, but OpenCVConfig.cmake):
	FIND_PACKAGE(OpenCV  QUIET NO_MODULE)
	IF(OpenCV_FOUND)
		SET(MRPT_OPENCV_VERSION ${OpenCV_VERSION})
		SET(OpenCV_LIBRARIES ${OpenCV_LIBS})
		SET(OPENCV_LIBDIR ${OpenCV_LIB_DIR})
		if (NOT "${BASEDIR}" STREQUAL "")
			SET(MRPT_OPENCV_SRC_DIR "${BASEDIR}")
		endif (NOT "${BASEDIR}" STREQUAL "")
		MESSAGE(STATUS "OpenCV ${OpenCV_VERSION} found through OpenCVConfig.cmake")

		SET(CMAKE_MRPT_HAS_OPENCV 1)
	ENDIF(OpenCV_FOUND)
ENDIF(NOT CMAKE_MRPT_HAS_OPENCV)


IF(NOT CMAKE_MRPT_HAS_OPENCV)
	# 2nd: OK, let's use the module:
	FIND_PACKAGE(OpenCV)
	IF(OpenCV_FOUND)
		# MRPT_OPENCV_VERSION
		IF($ENV{VERBOSE})
			MESSAGE(STATUS "OPENCV_EXE_LINKER_FLAGS: ${OpenCV_EXE_LINKER_FLAGS}")
			MESSAGE(STATUS "OPENCV_INCLUDE_DIR: ${OpenCV_INCLUDE_DIR}")
			MESSAGE(STATUS "OpenCV_LIBRARIES: ${OpenCV_LIBRARIES}")
		ENDIF($ENV{VERBOSE})

		FILE(GLOB_RECURSE CV_VER_H "${OpenCV_CXCORE_INCLUDE_DIR}/cvver.h")
		file(READ "${CV_VER_H}" STR_CV_VERSION)

		# Extract the CV version from the cvver.h file, lines "#define CV_MAJOR_VERSION  XX", etc...

		#STRING(REGEX MATCHALL "[0-9]+.[0-9]+.[0-9]+" MRPT_OPENCV_VERSION "${STR_CV_VERSION}")
		STRING(REGEX MATCH "CV_MAJOR_VERSION[ ]+[0-9]+" CMAKE_OPENCV_VERSION_NUMBER_MAJOR "${STR_CV_VERSION}")
		STRING(REGEX MATCH "[0-9]+" CMAKE_OPENCV_VERSION_NUMBER_MAJOR "${CMAKE_OPENCV_VERSION_NUMBER_MAJOR}")

		STRING(REGEX MATCH "CV_MINOR_VERSION[ ]+[0-9]+" CMAKE_OPENCV_VERSION_NUMBER_MINOR "${STR_CV_VERSION}")
		STRING(REGEX MATCH "[0-9]+" CMAKE_OPENCV_VERSION_NUMBER_MINOR "${CMAKE_OPENCV_VERSION_NUMBER_MINOR}")

		STRING(REGEX MATCH "CV_SUBMINOR_VERSION[ ]+[0-9]+" CMAKE_OPENCV_VERSION_NUMBER_PATCH "${STR_CV_VERSION}")
		STRING(REGEX MATCH "[0-9]+" CMAKE_OPENCV_VERSION_NUMBER_PATCH "${CMAKE_OPENCV_VERSION_NUMBER_PATCH}")

		SET(MRPT_OPENCV_VERSION "${CMAKE_OPENCV_VERSION_NUMBER_MAJOR}.${CMAKE_OPENCV_VERSION_NUMBER_MINOR}.${CMAKE_OPENCV_VERSION_NUMBER_PATCH}")

		MESSAGE(STATUS "OpenCV version detected: ${MRPT_OPENCV_VERSION}")

		SET(CMAKE_MRPT_HAS_OPENCV 1)
	ENDIF(OpenCV_FOUND)
ENDIF(NOT CMAKE_MRPT_HAS_OPENCV)


# Opencv version as Hex. number:
STRING(REGEX MATCHALL "[0-9]+" CMAKE_OPENCV_VERSION_PARTS "${MRPT_OPENCV_VERSION}")
LIST(GET CMAKE_OPENCV_VERSION_PARTS 0 CMAKE_OPENCV_VERSION_NUMBER_MAJOR)
LIST(GET CMAKE_OPENCV_VERSION_PARTS 1 CMAKE_OPENCV_VERSION_NUMBER_MINOR)
LIST(GET CMAKE_OPENCV_VERSION_PARTS 2 CMAKE_OPENCV_VERSION_NUMBER_PATCH)
SET(MRPT_OPENCV_VERSION_HEX "0x${CMAKE_OPENCV_VERSION_NUMBER_MAJOR}${CMAKE_OPENCV_VERSION_NUMBER_MINOR}${CMAKE_OPENCV_VERSION_NUMBER_PATCH}")

# DISABLE_OPENCV
# ---------------------
OPTION(DISABLE_OPENCV "Disable the OpenCV library" "OFF")
MARK_AS_ADVANCED(DISABLE_OPENCV)
IF(DISABLE_OPENCV)
	SET(CMAKE_MRPT_HAS_OPENCV 0)
ENDIF(DISABLE_OPENCV)


# OpenCV (all compilers):
IF(CMAKE_MRPT_HAS_OPENCV)
	INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIR})
	APPEND_MRPT_LIBS(${OpenCV_LIBRARIES})

	IF($ENV{VERBOSE})
		MESSAGE(STATUS "OpenCV:")
		MESSAGE(STATUS "        OpenCV_LIBRARIES:   ${OpenCV_LIBRARIES}")
		MESSAGE(STATUS "        OpenCV_INCLUDE_DIR: ${OpenCV_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS}")
	ENDIF($ENV{VERBOSE})

	# Add OpenCV directories as "-isystem" to avoid warnings with :
	ADD_DIRECTORIES_AS_ISYSTEM(OpenCV_INCLUDE_DIRS)

	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OPENCV_EXE_LINKER_FLAGS}")

	SET(CMAKE_MRPT_HAS_OPENCV_SYSTEM 1)

ENDIF(CMAKE_MRPT_HAS_OPENCV)
