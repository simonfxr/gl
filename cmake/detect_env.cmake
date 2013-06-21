### Detect OS/Arch

macro(set_bool var val)
  set(${var} ${val} CACHE BOOL "" FORCE)
endmacro()

if(UNIX)
  set_bool(SYS_UNIX TRUE)
endif()

if(WIN32)
  set_bool(SYS_WINDOWS TRUE)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set_bool(SYS_LINUX TRUE)
  set_bool(SYS_UNIX TRUE)
endif()

include(CheckTypeSize)
check_type_size(void* SIZEOF_VOID_PTR)
if(${SIZEOF_VOID_PTR} EQUAL "4")
  set_bool(BITS32 TRUE)
elseif(${SIZEOF_VOID_PTR} EQUAL "8")
  set_bool(BITS64 TRUE)
else()
  message(FATAL_ERROR "Unsupported architecture")
  return()
endif()

### Detect Compiler

if(MSVC OR MSVC_IDE OR CMAKE_COMPILER_2005 OR CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set_bool(COMP_CL TRUE)
elseif(CMAKE_CXX_COMPILER MATCHES ".*clang[+][+]" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set_bool(COMP_CLANG TRUE)
  set_bool(COMP_GCCLIKE TRUE)
elseif(CMAKE_CXX_COMPILER MATCHES ".*icpc" OR CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  set_bool(COMP_ICC TRUE)
  set_bool(COMP_GCCLIKE TRUE)
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set_bool(COMP_GCC TRUE)
  set_bool(COMP_GCCLIKE TRUE)
endif()
