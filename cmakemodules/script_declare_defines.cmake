# ----------------------------------------------------------------------------
# Transform the variables MRPT_XXX="ON/OFF" to CMAKE_MRPT_XXX="1/0"
# ----------------------------------------------------------------------------
MACRO(CREATE_CMAKE_MRPT_DEFINE defName)
	IF(${defName} MATCHES "ON")
		SET(CMAKE_${defName} "1")
	ELSE(${defName} MATCHES "ON")
		SET(CMAKE_${defName} "0")
	ENDIF(${defName} MATCHES "ON")
ENDMACRO(CREATE_CMAKE_MRPT_DEFINE)

CREATE_CMAKE_MRPT_DEFINE(MRPT_ALWAYS_CHECKS_DEBUG)
CREATE_CMAKE_MRPT_DEFINE(MRPT_ALWAYS_CHECKS_DEBUG_MATRICES)
CREATE_CMAKE_MRPT_DEFINE(MRPT_HAS_BUMBLEBEE)
CREATE_CMAKE_MRPT_DEFINE(MRPT_HAS_PHIDGET)
CREATE_CMAKE_MRPT_DEFINE(MRPT_HAS_SVS)
CREATE_CMAKE_MRPT_DEFINE(MRPT_HAS_ASSERT)
CREATE_CMAKE_MRPT_DEFINE(MRPT_HAS_STACKED_EXCEPTIONS)
CREATE_CMAKE_MRPT_DEFINE(MRPT_ENABLE_EMBEDDED_GLOBAL_PROFILER)
CREATE_CMAKE_MRPT_DEFINE(MRPT_HAS_ASIAN_FONTS)
CREATE_CMAKE_MRPT_DEFINE(CMAKE_MRPT_HAS_INOTIFY)
