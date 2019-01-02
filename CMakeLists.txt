
cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(OpenGL_GL_PREFERENCE GLVND)

if(NOT DEFINED BUILD_TYPE)
  set(BUILD_TYPE Debug)
endif()
set(BUILD_TYPE ${BUILD_TYPE} CACHE STRING "build type Debug|DebugOpt|Release" FORCE)

if(BUILD_TYPE STREQUAL "DebugOpt")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
else()
  set(CMAKE_BUILD_TYPE ${BUILD_TYPE} CACHE STRING "" FORCE)
endif()

project(gl)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/detect_env.cmake)

macro(set_option var default type docstring)
  if(NOT DEFINED ${var})
    set(${var} ${default})
  endif()
  set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
endmacro()

set_option(BUILD_SHARED_LIBS TRUE BOOL "TRUE to build shared libs, FALSE to build static libs")

if(BUILD_TYPE STREQUAL "Release")
  set(GLDEBUG_DEFAULT FALSE)
else()
  set(GLDEBUG_DEFAULT TRUE)
endif()
set_option(GLDEBUG ${GLDEBUG_DEFAULT} BOOL "TRUE to enable error checking after every opengl call")

set_option(MATH_INLINE TRUE BOOL "TRUE to inline all custom math functions")

if(SYS_WINDOWS)
  set(GLEW_INC_DEF ${CMAKE_CURRENT_LIST_DIR}/ext/glew/include)
  set(GLEW_LIB_DEF ${CMAKE_CURRENT_LIST_DIR}/ext/glew/lib/glew32.lib)
  set(GLFW_INC_DEF ${CMAKE_CURRENT_LIST_DIR}/ext/glfw3/include)
  set(GLFW_LIB_DEF ${CMAKE_CURRENT_LIST_DIR}/ext/glfw3/lib-msvc110/glfw3dll.lib)

  set_option(GLEW_INCLUDE_PATH "${GLEW_INC_DEF}" PATH "path to GLEW headers")
  set_option(GLEW_LIBRARY "${GLEW_LIB_DEF}" PATH "path to GLEW library")
  set_option(GLFW_INCLUDE_PATH "${GLFW_INC_DEF}" PATH "path to GLFW headers")
  set_option(GLFW_LIBRARY "${GLFW_LIB_DEF}" PATH "path to GLEW library")
endif()

set_option(USE_OPENMP True BOOL "enable openmp support")

set_option(USE_LTO FALSE BOOL "enable link time optimisations")

if(BITS64)
  set_option(BUILD_ARCH32 FALSE BOOL "TRUE to force 32-bit build on 64bit platform")
endif()

set_option(USE_VOXEL_OPENCL FALSE BOOL "use opencl for the voxel-world program")

if(SYS_LINUX)
  set_option(USE_UNWIND_STACKTRACES TRUE BOOL "use libunwind and libdw to print a stacktrace on errors")
endif()

if(SYS_WINDOWS)
  set(LIBS_IN_TREE TRUE)
else()
  set(LIBS_IN_TREE FALSE)
endif()

set_option(USE_GLFW_IN_TREE ${LIBS_IN_TREE} BOOL "use glfw under ext/glfw3")
set_option(USE_GLEW_IN_TREE ${LIBS_IN_TREE} BOOL "use glew under ext/glew")
set_option(USE_OPENCL_HEADERS_IN_TREE ${LIBS_IN_TREE} BOOL "use OpenCL headerse under ext/opencl-headers")

if(COMP_CLANG)
  set_option(USE_CLANG_ADDRESS_SANITIZER FALSE BOOL "enable -fsanitize=address")
  set_option(USE_CLANG_TRAP_UNDEFINED FALSE BOOL "enable -fsanitize=undefined-trap")
endif()

add_subdirectory(deps/cxx-header-utils)

include_directories(src)

add_subdirectory(src)
add_subdirectory(programs)
add_subdirectory(test)
