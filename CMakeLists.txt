cmake_minimum_required(VERSION 3.25)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(OpenGL_GL_PREFERENCE GLVND)

project(gl LANGUAGES C CXX)

include("${CMAKE_SOURCE_DIR}/cmake/cmake-utils/cmake-utils.cmake")

macro(set_option var default type docstring)
  if(NOT DEFINED ${var})
    set(${var} ${default})
  endif()
  set(${var}
      ${${var}}
      CACHE ${type} ${docstring} FORCE)
endmacro()

set(build_shared_default True)
if(CMU_OS_WINDOWS AND CMU_COMP_MSVC)
  set(build_shared_default False)
endif()

set_option(BUILD_SHARED_LIBS ${build_shared_default} BOOL
           "True to build shared libs, False to build static libs")

message(STATUS "BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}")

# FIXME if(COMP_CLANG) set_option( ENABLE_LIBCXX False BOOL "True to use libc++
# when compiling with clang" ) endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" gl_build_type)

set(is_devel True)
if(CMU_BUILD_TYPE MATCHES "RELEASE|RELWITHDEBINFO|MINSIZEREL")
  set(is_devel False)
endif()
set_option(ENABLE_GLDEBUG ${is_devel} BOOL
           "True to enable error checking after every opengl call")

set_option(ENABLE_OPENMP True BOOL "enable openmp support")

set_option(ENABLE_VOXEL_OPENCL False BOOL
           "use opencl for the voxel-world program")

set_option(ENABLE_ASAN ${is_devel} BOOL "enable -fsanitize=address")
set_option(ENABLE_UBSAN ${is_devel} BOOL "enable -fsanitize=undefined")
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
list(APPEND CMU_CXX_FLAGS "-Wno-non-template-friend")

set(CMU_SANITIZERS)
if(ENABLE_ASAN)
  list(APPEND CMU_SANITIZERS asan lsan)
endif()
if(ENABLE_UBSAN)
  list(APPEND CMU_SANITIZERS ubsan)
endif()

set(CMU_DEBUG_GLIBCXX_SANITIZE_VECTOR False)
set(CMU_DEBUG_GLIBCXX_DEBUG False)
set(CMU_DEBUG_GLIBCXX_DEBUG_PEDANTIC False)
# set(CMU_PREFERRED_LINKERS lld gold bfd)

set(CMU_NO_EXCEPTIONS True)
set(CMU_NO_RTTI True)
cmu_configure()

if(NOT TARGET glfw)
  set(GLFW_INSTALL
      OFF
      CACHE BOOL "" FORCE)
  set(GLFW_BUILD_DOCS
      OFF
      CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS
      OFF
      CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES
      OFF
      CACHE BOOL "" FORCE)
  add_subdirectory(deps/glfw)
  add_library(glfw::glfw ALIAS glfw)
endif()

if(NOT TARGET glad::glad)
  find_package(
    Python3 3.9
    COMPONENTS Interpreter
    REQUIRED)
  set(PythonInterp_FOUND TRUE)
  set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
  add_subdirectory(deps/glad/cmake "glad_cmake")
  set(glad_extensions
      GL_AMD_debug_output
      GL_ARB_debug_output
      GL_ATI_meminfo
      GL_EXT_debug_marker
      GL_EXT_direct_state_access
      GL_KHR_debug
      GL_NVX_gpu_memory_info
      GL_ARB_multisample)

  glad_add_library(
    glad_generated
    MERGE
    LANGUAGE
    C
    REPRODUCABLE
    API
    gl:compatibility=4.6
    EXTENSIONS
    ${glad_extensions})
  add_library(glad::glad ALIAS glad_generated)
endif()

add_subdirectory(deps/cxx-header-utils)

set(HAVE_FIBER True)
add_subdirectory(deps/fiber)

add_subdirectory(deps/bitops)

include_directories(src)

add_subdirectory(src)
add_subdirectory(programs)
add_subdirectory(test)
