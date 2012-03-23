
cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "build type Debug|Release|RelWithDebInfo|MinSizeRel" FORCE)
endif()

set(BUILD_SHARED_LIBS TRUE CACHE BOOL "TRUE to build shared libs, FALSE to build static libs")

set(GLDEBUG TRUE CACHE BOOL "TRUE to enable error checking after every opengl call")

set(MATH_INLINE TRUE CACHE BOOL "TRUE to inline all custom math functions")

set(GLEW_ROOT_DIR "" CACHE STRING "GLEW installation directory")

set(SFMLDIR "" CACHE STRING "SFML installation directory")

set(USE_OPENMP TRUE CACHE BOOL "enable openmp support")

set(USE_LTO FALSE CACHE BOOL "enable link time optimisations")

set(BUILD_ARCH32 FALSE CACHE BOOL "TRUE to force 32-bit build on 64bit platform")

set(USE_VOXEL_OPENCL FALSE CACHE BOOL "use opencl for the voxel-world program")

set(USE_UNWIND_STACKTRACES TRUE CACHE BOOL "use libunwind and libdw to print a stacktrace on errors")

set(USE_CXX11 TRUE CACHE BOOL "compile with support for the c++11 standard")

set(USE_NO_MATH_H TRUE CACHE BOOL "dont import math.h in math library")

project(gl-stuff)

include_directories(src)

add_subdirectory(src)
add_subdirectory(programs)
add_subdirectory(test)
