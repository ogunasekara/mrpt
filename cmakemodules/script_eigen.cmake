# Eigen library plugins:
# ===================================================
SET(EIGEN_MATRIXBASE_PLUGIN "<mrpt/math/eigen_plugins.h>" CACHE STRING "Eigen plugin header")
SET(EIGEN_MATRIXBASE_PLUGIN_POST_IMPL "<mrpt/math/eigen_plugins_impl.h>" CACHE STRING "Eigen plugin implementation header")
SET(EIGEN_EMBEDDED_INCLUDE_DIR "${MRPT_SOURCE_DIR}/otherlibs/eigen3/" CACHE PATH "Eigen path for embedded use")

MARK_AS_ADVANCED(EIGEN_MATRIXBASE_PLUGIN)
MARK_AS_ADVANCED(EIGEN_MATRIXBASE_PLUGIN_POST_IMPL)
MARK_AS_ADVANCED(EIGEN_EMBEDDED_INCLUDE_DIR)

# By default: Use system version if pkg-config says it exists:
SET(DEFAULT_EIGEN_USE_EMBEDDED_VERSION ON)
IF(PKG_CONFIG_FOUND)
	PKG_CHECK_MODULES(PKG_EIGEN3 eigen3)	# Find eigen3 with pkg-config:
	IF(PKG_EIGEN3_FOUND)
		# Special case: There's a problem with GCC 4.7+ and Eigen before ~May 2012, so
		#  if we have GCC>=4.7 and Eigen<4.1.0, switch to our embedded version which is 
		#  already fixed to cope with this.
		# (See: http://forum.kde.org/viewtopic.php?f=74&t=99314 )
		IF ("${PKG_EIGEN3_VERSION}" VERSION_LESS 4.1.0 AND NOT "${CMAKE_MRPT_GCC_VERSION}" VERSION_LESS "47")
			# Remain using embedded version.
		ELSE("${PKG_EIGEN3_VERSION}" VERSION_LESS 4.1.0 AND NOT "${CMAKE_MRPT_GCC_VERSION}" VERSION_LESS "47")
			# Use system version:
			SET(DEFAULT_EIGEN_USE_EMBEDDED_VERSION OFF)
		ENDIF("${PKG_EIGEN3_VERSION}" VERSION_LESS 4.1.0 AND NOT "${CMAKE_MRPT_GCC_VERSION}" VERSION_LESS "47")
	ENDIF(PKG_EIGEN3_FOUND)
ENDIF(PKG_CONFIG_FOUND)

SET(EIGEN_USE_EMBEDDED_VERSION ${DEFAULT_EIGEN_USE_EMBEDDED_VERSION} CACHE BOOL "Use embedded Eigen3 version or system version")
IF (EIGEN_USE_EMBEDDED_VERSION)
	# Include embedded version headers:
	SET(MRPT_EIGEN_INCLUDE_DIR "${EIGEN_EMBEDDED_INCLUDE_DIR}")
ELSE(EIGEN_USE_EMBEDDED_VERSION)
	# Find Eigen headers in the system:
	IF(NOT PKG_CONFIG_FOUND)
		MESSAGE(SEND_ERROR "pkg-config is required for this operation!")
	ELSE(NOT PKG_CONFIG_FOUND)
		# Find eigen3 with pkg-config:
		PKG_CHECK_MODULES(PKG_EIGEN3 eigen3)
		IF(PKG_EIGEN3_FOUND)
			SET(MRPT_EIGEN_INCLUDE_DIR "${PKG_EIGEN3_INCLUDE_DIRS}")
		ELSE(PKG_EIGEN3_FOUND)
			MESSAGE(SEND_ERROR "pkg-config was unable to find eigen3: install libeigen3-dev or enable EIGEN_USE_EMBEDDED_VERSION")
		ENDIF(PKG_EIGEN3_FOUND)
	ENDIF(NOT PKG_CONFIG_FOUND)
ENDIF(EIGEN_USE_EMBEDDED_VERSION)

INCLUDE_DIRECTORIES("${MRPT_EIGEN_INCLUDE_DIR}")
IF(EXISTS "${MRPT_EIGEN_INCLUDE_DIR}/unsupported/")
	INCLUDE_DIRECTORIES("${MRPT_EIGEN_INCLUDE_DIR}/unsupported/")
ENDIF(EXISTS "${MRPT_EIGEN_INCLUDE_DIR}/unsupported/")

# Create variables just for the final summary of the configuration (see bottom of this file):
SET(CMAKE_MRPT_HAS_EIGEN 1)        # Always, it's a fundamental dep.!

# Create numeric (0/1) variable EIGEN_USE_EMBEDDED_VERSION_BOOL for the .cmake.in file:
IF(EIGEN_USE_EMBEDDED_VERSION)
	SET(EIGEN_USE_EMBEDDED_VERSION_BOOL 1)
	SET(CMAKE_MRPT_HAS_EIGEN_SYSTEM 0)
ELSE(EIGEN_USE_EMBEDDED_VERSION)
	SET(EIGEN_USE_EMBEDDED_VERSION_BOOL 0)
	SET(CMAKE_MRPT_HAS_EIGEN_SYSTEM 1)
ENDIF(EIGEN_USE_EMBEDDED_VERSION)
