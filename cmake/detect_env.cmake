### Detect OS/Arch

macro(set_state var val)
  set(${var} ${val} CACHE INTERNAL "" FORCE)
endmacro()

if(UNIX)
  set_state(SYS_UNIX TRUE)
endif()

if(WIN32)
  set_state(SYS_WINDOWS TRUE)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set_state(SYS_LINUX TRUE)
  set_state(SYS_UNIX TRUE)
endif()

include(CheckTypeSize)
check_type_size(void* SIZEOF_VOID_PTR)
if(${SIZEOF_VOID_PTR} EQUAL "4")
  set_state(BITS32 TRUE)
elseif(${SIZEOF_VOID_PTR} EQUAL "8")
  set_state(BITS64 TRUE)
else()
  message(FATAL_ERROR "Unsupported architecture")
  return()
endif()

### Detect Compiler

if(MSVC OR MSVC_IDE OR CMAKE_COMPILER_2005 OR CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set_state(COMP_CL TRUE)
  if (MSVC_VERSION LESS 1700)
    message(FATAL_ERROR "needs at least MSVC 2011")
  elseif(MSVC_VERSION EQUAL 1700)
    set_state(CL_VS_VERSION 11)
  elseif(MSVC_VERSION EQUAL 1800)
    set_state(CL_VS_VERSION 12)
  elseif(MSVC_VERSION GREATER 1800)
    set_state(CL_VS_VERSION 99)
  endif()
elseif(CMAKE_CXX_COMPILER MATCHES ".*clang[+][+]" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set_state(COMP_CLANG TRUE)
  set_state(COMP_GCCLIKE TRUE)
elseif(CMAKE_CXX_COMPILER MATCHES ".*icpc" OR CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  set_state(COMP_ICC TRUE)
  set_state(COMP_GCCLIKE TRUE)
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set_state(COMP_GCC TRUE)
  set_state(COMP_GCCLIKE TRUE)
endif()

if(${BUILD_TYPE} STREQUAL Debug)
  set_state(BUILD_DEBUG TRUE)
elseif(${BUILD_TYPE} STREQUAL DebugOpt)
  set_state(BUILD_OPT TRUE)
  set_state(BUILD_DEBUG TRUE)
elseif(${BUILD_TYPE} STREQUAL Release)
  set_state(BUILD_OPT TRUE)
else()
  message(WARNING "unknown build type: \"${BUILD_TYPE}\"")
endif()
