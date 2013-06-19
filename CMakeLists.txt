
cmake_minimum_required(VERSION 2.8)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/detect_env.cmake)

macro(set_option var default type docstring)
    if(NOT DEFINED ${var})
        set(${var} ${default})
    endif()
    set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
endmacro()

set_option(BUILD_TYPE DebugOpt STRING "build type Debug|DebugOpt|Release")
if(BUILD_TYPE STREQUAL "DebugOpt")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
else()
  set(CMAKE_BUILD_TYPE ${BUILD_TYPE} CACHE STRING "" FORCE)
endif()

set_option(BUILD_SHARED_LIBS TRUE BOOL "TRUE to build shared libs, FALSE to build static libs")

if(BUILD_TYPE STREQUAL "Release")
  set(GLDEBUG_DEFAULT FALSE)
else()
  set(GLDEBUG_DEFAULT TRUE)
endif()
set_option(GLDEBUG ${GLDEBUG_DEFAULT} BOOL "TRUE to enable error checking after every opengl call")
unset(GLDEBUG_DEFAULT)

set_option(MATH_INLINE TRUE BOOL "TRUE to inline all custom math functions")

set_option(GLEW_ROOT_DIR "" PATH "GLEW installation directory")

if(COMP_GCC OR COMP_ICC)
  set(USE_OPENMP_DEFAULT TRUE)
else()
  set(USE_OPENMP_DEFAULT FALSE)
endif()
set_option(USE_OPENMP TRUE CACHE ${USE_OPENMP_DEFAULT} "enable openmp support")
unset(USE_OPENMP_DEFAULT)

set_option(USE_LTO FALSE BOOL "enable link time optimisations")

set_option(BUILD_ARCH32 FALSE CACHE BOOL "TRUE to force 32-bit build on 64bit platform")

set(USE_VOXEL_OPENCL FALSE CACHE BOOL "use opencl for the voxel-world program")

if(SYS_LINUX)
  set(USE_UNWIND_STACKTRACES TRUE CACHE BOOL "use libunwind and libdw to print a stacktrace on errors")
endif()

set(USE_CXX11 TRUE CACHE BOOL "compile with support for the c++11 standard")

set(USE_CXX11_FINAL_OVERRIDE TRUE CACHE BOOL "use the new c++11 override and final keywords")

set(USE_NO_MATH_H TRUE CACHE BOOL "dont import math.h in math library")

project(gl)

include_directories(src)

add_subdirectory(src)
add_subdirectory(programs)
add_subdirectory(test)
