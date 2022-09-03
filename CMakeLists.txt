cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(OpenGL_GL_PREFERENCE GLVND)

project(gl LANGUAGES C CXX)

include("${CMAKE_SOURCE_DIR}/cmake/cmake-utils/cmake-utils.cmake")

macro(set_option var default type docstring)
  if(NOT DEFINED ${var})
    set(${var} ${default})
  endif()
  set(
    ${var}
    ${${var}}
    CACHE ${type} ${docstring} FORCE
  )
endmacro()

set(build_shared_default True)
if(CMU_OS_WINDOWS AND CMU_COMP_MSVC)
  set(build_shared_default False)
endif()

set_option(
  BUILD_SHARED_LIBS ${build_shared_default} BOOL
  "True to build shared libs, False to build static libs"
)

message(STATUS "BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}")

# FIXME
# if(COMP_CLANG)
#   set_option(
#     ENABLE_LIBCXX False BOOL
#     "True to use libc++ when compiling with clang"
#   )
# endif()

set(gldebug_default True)
if(build_type STREQUAL RELEASE)
  set(gldebug_default False)
endif()
set_option(
  ENABLE_GLDEBUG ${gldebug_default} BOOL
  "True to enable error checking after every opengl call"
)

set_option(ENABLE_OPENMP True BOOL "enable openmp support")

set_option(
  ENABLE_VOXEL_OPENCL False BOOL
  "use opencl for the voxel-world program"
)

set_option(ENABLE_ASAN True BOOL "enable -fsanitize=address")
set_option(ENABLE_UBSAN True BOOL "enable -fsanitize=undefined")
set_option(ENABLE_LTO True BOOL "enable lto")

if(UNIX)
  set_option(ENABLE_STACKTRACES True BOOL "print a stacktrace on errors")
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/macros.cmake)

set(CMU_FP_MODE ASSOC)

set(CMU_IPO ${ENABLE_LTO})
set(CMU_OPT_NATIVE True)
set(CMU_OPT_LEVEL 4)

set(CMU_SANITIZERS)
# if(ENABLE_ASAN)
#   list(APPEND CMU_SANITIZERS asan lsan)
# endif()
# if(ENABLE_UBSAN)
#   list(APPEND CMU_SANITIZERS ubsan)
# endif()

set(CMU_NO_EXCEPTIONS True)
set(CMU_NO_RTTI True)
cmu_configure()

if(NOT TARGET glfw)
  set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  add_subdirectory(deps/glfw)
  add_library(glfw::glfw ALIAS glfw)
endif()

if(NOT TARGET glad::glad)
  set(GLAD_PROFILE compatibility CACHE STRING "" FORCE)
  set(GLAD_ALL_EXTENSIONS On CACHE BOOL "" FORCE)
  set(GLAD_REPRODUCIBLE On CACHE BOOL "" FORCE)
  add_subdirectory(deps/glad)
  add_library(glad_dl INTERFACE)
  target_link_libraries(glad_dl INTERFACE glad)
  if(CMU_OBJFMT_ELF)
    target_link_libraries(glad_dl INTERFACE dl)
  endif()
  add_library(glad::glad ALIAS glad_dl)
endif()

add_subdirectory(deps/cxx-header-utils)

set(HAVE_FIBER True)
add_subdirectory(deps/fiber)

include_directories(src)

add_subdirectory(src)
add_subdirectory(programs)
add_subdirectory(test)
