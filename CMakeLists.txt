
cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(OpenGL_GL_PREFERENCE GLVND)

project(gl LANGUAGES C CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/detect_env.cmake)

macro(set_option var default type docstring)
  if(NOT DEFINED ${var})
    set(${var} ${default})
  endif()
  set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
endmacro()

set(build_shared_default TRUE)
if(SYS_WINDOWS AND COMP_MSVC)
  set(build_shared_default FALSE)
endif()

set_option(BUILD_SHARED_LIBS ${build_shared_default} BOOL "TRUE to build shared libs, FALSE to build static libs")

if(CMAKE_BUILD_TYPE STREQUAL RELEASE)
  set(GLDEBUG_DEFAULT FALSE)
else()
  set(GLDEBUG_DEFAULT TRUE)
endif()
set_option(GLDEBUG ${GLDEBUG_DEFAULT} BOOL "TRUE to enable error checking after every opengl call")

set_option(USE_OPENMP True BOOL "enable openmp support")

set_option(USE_VOXEL_OPENCL FALSE BOOL "use opencl for the voxel-world program")

if(SYS_LINUX)
  set_option(USE_UNWIND_STACKTRACES TRUE BOOL "use libunwind and libdw to print a stacktrace on errors")
endif()

if(COMP_CLANG OR COMP_GCC)
  set_option(USE_ASAN FALSE BOOL "enable -fsanitize=address")
  if(BUILD_DEBUG)
    set(ubsan_default TRUE)
  else()
    set(ubsan_default FALSE)
  endif()
  set_option(USE_UBSAN ${ubsan_default} BOOL "enable -fsanitize=undefined")
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/macros.cmake)

add_subdirectory(deps/cxx-header-utils)

if(NOT TARGET glfw)
  set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  add_subdirectory(deps/glfw)
endif()

if(NOT TARGET glad)
  find_package(PythonInterp 3 REQUIRED)
  set(GLAD_DIR "${CMAKE_CURRENT_LIST_DIR}/deps/glad")
  get_filename_component(GLAD_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/glad-generated" ABSOLUTE)

  if(NOT EXISTS "${GLAD_GEN_DIR}/src/glad.c")
    message(STATUS "invokding glad generator: ${GLAD_GEN_DIR}")
    file(MAKE_DIRECTORY "${GLAD_GEN_DIR}")
    execute_process(
      COMMAND "${PYTHON_EXECUTABLE}" -m glad --out-path "${GLAD_GEN_DIR}" --generator c
      WORKING_DIRECTORY "${GLAD_DIR}"
      RESULT_VARIABLE glad_ec)
    if(NOT glad_ec EQUAL "0")
      message(FATAL_ERROR "glad generator failed with exit code: ${glad_ec}")
    endif()
  endif()

  set(glad_link_mode)
  if(SYS_WINDOWS AND COMP_MSVC)
    set(glad_link_mode STATIC)
  endif()
  add_library(glad ${glad_link_mode} "${GLAD_GEN_DIR}/src/glad.c")
  target_include_directories(glad SYSTEM PUBLIC "${GLAD_GEN_DIR}/include")
endif()

if(UNIX)
  set(HAVE_FIBER TRUE)
  add_subdirectory(deps/fiber)
endif()

include_directories(src)

add_subdirectory(src)
add_subdirectory(programs)
add_subdirectory(test)
