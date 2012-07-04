# Enable Intel Threading Building Blocks (TBB) lib
# ===================================================
SET(MRPT_HAS_TBB OFF CACHE BOOL "Enable Intel TBB threading library for parallelization.")
SET(CMAKE_MRPT_HAS_TBB  0)

# The following code block comes from OpenCV (BSD)
if (MRPT_HAS_TBB)
    if (UNIX AND NOT APPLE AND NOT ANDROID)
        PKG_CHECK_MODULES(TBB tbb)
        message(STATUS "TBB detected: ${TBBLIB_FOUND}")

        if (TBB_FOUND)
            set(CMAKE_MRPT_HAS_TBB 1)
            if(NOT ${TBB_INCLUDE_DIRS} STREQUAL "")
                include_directories(${TBB_INCLUDE_DIRS})
            endif(NOT ${TBB_INCLUDE_DIRS} STREQUAL "")
            link_directories(${TBB_LIBRARY_DIRS})
            APPEND_MRPT_LIBS(${TBB_LIBRARIES})
        endif(TBB_FOUND)
    endif(UNIX AND NOT APPLE AND NOT ANDROID)

    if (NOT CMAKE_MRPT_HAS_TBB)
        set(TBB_DEFAULT_INCLUDE_DIRS "/opt/intel/tbb" "/usr/local/include" "/usr/include" "C:/Program Files/Intel/TBB" "C:/Program Files (x86)/Intel/TBB" "C:/Program Files (x86)/TBB" "${CMAKE_INSTALL_PREFIX}/include")

        find_path(TBB_INCLUDE_DIR "tbb/tbb.h" PATHS ${TBB_DEFAULT_INCLUDE_DIRS} DOC "The path to TBB headers")
        if (TBB_INCLUDE_DIR)
            if (UNIX)
                set(TBB_LIB_DIR "${TBB_INCLUDE_DIR}/../lib" CACHE PATH "Full path of TBB library directory")
                link_directories("${TBB_LIB_DIR}")
            endif(UNIX)
            if (APPLE)
                APPEND_MRPT_LIBS(libtbb.dylib)
            elseif (ANDROID)
                APPEND_MRPT_LIBS(tbbmalloc tbb)
                add_definitions(-DTBB_USE_GCC_BUILTINS)
            elseif (UNIX)
                APPEND_MRPT_LIBS(tbb)
            elseif (WIN32)
                if (CMAKE_COMPILER_IS_GNUCXX)
                    set(TBB_LIB_DIR "${TBB_INCLUDE_DIR}/../lib" CACHE PATH "Full path of TBB library directory")
                    link_directories("${TBB_LIB_DIR}")
                    APPEND_MRPT_LIBS(tbb)
                else(CMAKE_COMPILER_IS_GNUCXX)
                    get_filename_component(_TBB_LIB_PATH "${TBB_INCLUDE_DIR}/../lib" ABSOLUTE)

                    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES amd64*|x86_64*)
                        set(_TBB_LIB_PATH "${_TBB_LIB_PATH}/intel64")
                    endif(${CMAKE_SYSTEM_PROCESSOR} MATCHES amd64*|x86_64*)
                    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES x86*|i386*|i686*)
                        set(_TBB_LIB_PATH "${_TBB_LIB_PATH}/ia32")
                    endif(${CMAKE_SYSTEM_PROCESSOR} MATCHES x86*|i386*|i686*)

                    if (MSVC80)
                       set(_TBB_LIB_PATH "${_TBB_LIB_PATH}/vc8")
                    elseif(MSVC90)
                       set(_TBB_LIB_PATH "${_TBB_LIB_PATH}/vc9")
                    elseif(MSVC10)
                        set(_TBB_LIB_PATH "${_TBB_LIB_PATH}/vc10")
                    endif(MSVC80)
                    set(TBB_LIB_DIR "${_TBB_LIB_PATH}" CACHE PATH "Full path of TBB library directory")
                    link_directories("${TBB_LIB_DIR}")
                endif(CMAKE_COMPILER_IS_GNUCXX)
            endif(UNIX)

            set(CMAKE_MRPT_HAS_TBB 1)
            if(NOT "${TBB_INCLUDE_DIR}" STREQUAL "")
                include_directories("${TBB_INCLUDE_DIR}")
            endif(NOT "${TBB_INCLUDE_DIR}" STREQUAL "")
        endif()
    endif(NOT CMAKE_MRPT_HAS_TBB)
endif(MRPT_HAS_TBB)
