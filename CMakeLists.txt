cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(OpenGL_GL_PREFERENCE GLVND)

project(gl LANGUAGES C CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/detect_env.cmake)

macro(set_option
      var
      default
      type
      docstring)
  if(NOT DEFINED ${var})
    set(${var} ${default})
  endif()
  set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
endmacro()

set(build_shared_default True)
if(SYS_WINDOWS AND COMP_MSVC)
  set(build_shared_default False)
endif()

set_option(BUILD_SHARED_LIBS
           ${build_shared_default}
           BOOL
           "True to build shared libs, False to build static libs")

message(STATUS "BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}")

if(COMP_CLANG)
  set_option(ENABLE_LIBCXX
             False
             BOOL
             "True to use libc++ when compiling with clang")
endif()

set(gldebug_level_default 0)
if(BUILD_TYPE STREQUAL DEBUG)
  set(gldebug_level_default 2)
elseif(BUILD_TYPE STREQUAL RELWITHDEBINFO)
  set(gldebug_level_default 1)
endif()
set_option(GLDEBUG_LEVEL
           ${gldebug_level_default}
           BOOL
           "Enable OpenGL debug output and checks")

set(openmp_default True)
if(BUILD_TYPE STREQUAL DEBUG)
  set(openmp_default False)
endif()
set_option(ENABLE_OPENMP
           ${openmp_default}
           BOOL
           "Enable OpenMP support")

set_option(ENABLE_VOXEL_OPENCL
           False
           BOOL
           "Use OpenCL for the voxel-world program")

set_option(ENABLE_STACKTRACES
           True
           BOOL
           "print a stacktrace on errors")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/macros.cmake)

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
  get_filename_component(GLAD_GEN_DIR
                         "${CMAKE_CURRENT_BINARY_DIR}/glad-generated" ABSOLUTE)

  if(NOT EXISTS "${GLAD_GEN_DIR}/src/glad.c")
    message(STATUS "invokding glad generator: ${GLAD_GEN_DIR}")
    file(MAKE_DIRECTORY "${GLAD_GEN_DIR}")
    execute_process(COMMAND "${PYTHON_EXECUTABLE}"
                            -m
                            glad
                            --out-path "${GLAD_GEN_DIR}"
                            --generator c
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
  add_library(glad "${GLAD_GEN_DIR}/src/glad.c" ${glad_link_mode})
  target_include_directories(glad SYSTEM PUBLIC "${GLAD_GEN_DIR}/include")
  if(SYS_UNIX)
    target_link_libraries(glad dl)
  endif()
endif()

if(COMP_MSVC)
  replace_cmake_flags("/W[0-4]" "/W4")
endif()

add_subdirectory(deps/cxx-header-utils)

set(HAVE_FIBER True)
add_subdirectory(deps/fiber)

include_directories(src)

add_subdirectory(src)
add_subdirectory(programs)
add_subdirectory(test)
