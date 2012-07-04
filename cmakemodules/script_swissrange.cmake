# Support for SWISSRANGE 3D camera:
# ===================================================
SET(CMAKE_MRPT_HAS_SWISSRANGE 0)

IF(UNIX)
	# Linux: Look for the libMesaSR.h include:
	FIND_FILE(SWR_HEADER_FILE  libMesaSR.h)
	IF(SWR_HEADER_FILE)
		SET(CMAKE_MRPT_HAS_SWISSRANGE 1)
		MARK_AS_ADVANCED(SWR_HEADER_FILE)
		APPEND_MRPT_LIBS(mesasr)
	ELSE(SWR_HEADER_FILE)
		SET(CMAKE_MRPT_HAS_SWISSRANGE 0)
	ENDIF(SWR_HEADER_FILE)
ELSE(UNIX)
	IF (MSVC)
		# Windows: ...
		FIND_PATH(SWR_LIBMESASR_DIR  MesaImaging/Swissranger/libMesaSR/)

		IF(SWR_LIBMESASR_DIR)
			SET(SWR_LIBMESASR_DIR "${SWR_LIBMESASR_DIR}/MesaImaging/Swissranger/libMesaSR/")
			MESSAGE(STATUS "SwissRanger Library found in: ${SWR_LIBMESASR_DIR}")

			# We expect to find there "libMesaSR.lib" & "libMesaSR.h"
			IF (EXISTS "${SWR_LIBMESASR_DIR}/libMesaSR.lib" AND EXISTS "${SWR_LIBMESASR_DIR}/libMesaSR.h")
				SET(CMAKE_MRPT_HAS_SWISSRANGE 1)
				INCLUDE_DIRECTORIES("${SWR_LIBMESASR_DIR}")
				#LINK_DIRECTORIES("${SWR_LIBMESASR_DIR}")
				APPEND_MRPT_LIBS("${SWR_LIBMESASR_DIR}/libMesaSR.lib")
				MARK_AS_ADVANCED(SWR_LIBMESASR_DIR)
			ELSE (EXISTS "${SWR_LIBMESASR_DIR}/libMesaSR.lib" AND EXISTS "${SWR_LIBMESASR_DIR}/libMesaSR.h")
				MESSAGE(STATUS "*** ERROR *** SwissRanger Library directory found but doesn't contain expected files. Not using it.")
				SET(CMAKE_MRPT_HAS_SWISSRANGE 0)
			ENDIF (EXISTS "${SWR_LIBMESASR_DIR}/libMesaSR.lib" AND EXISTS "${SWR_LIBMESASR_DIR}/libMesaSR.h")
		ELSE(SWR_LIBMESASR_DIR)
			SET(CMAKE_MRPT_HAS_SWISSRANGE 0)
		ENDIF(SWR_LIBMESASR_DIR)
	ENDIF(MSVC)
ENDIF(UNIX)

# Leave at the user's choice to disable the SWR libs:
OPTION(DISABLE_SWISSRANGER_3DCAM_LIBS "Disable the usage (if found) of SWR libs" "OFF")
MARK_AS_ADVANCED(DISABLE_SWISSRANGER_3DCAM_LIBS)
IF(DISABLE_SWISSRANGER_3DCAM_LIBS)
	SET(CMAKE_MRPT_HAS_SWISSRANGE 0)
ENDIF(DISABLE_SWISSRANGER_3DCAM_LIBS)
