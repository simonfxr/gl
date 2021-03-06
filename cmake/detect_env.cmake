# Detect OS/Arch

if(UNIX)
  set(SYS_UNIX True)
endif()

if(WIN32)
  set(SYS_WINDOWS True)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(SYS_LINUX True)
  set(SYS_UNIX True)
endif()

if(MSVC
   OR MSVC_IDE
   OR CMAKE_COMPILER_2005
   OR CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(COMP_MSVC True)
elseif(CMAKE_CXX_COMPILER MATCHES ".*clang[+][+]"
       OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(COMP_CLANG True)
  set(COMP_GCCLIKE True)
elseif(CMAKE_CXX_COMPILER MATCHES ".*icpc"
       OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")
  set(COMP_ICC True)
  set(COMP_GCCLIKE True)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  set(COMP_GCC True)
  set(COMP_GCCLIKE True)
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" build_type)

if(build_type STREQUAL DEBUG)
  set(BUILD_DEBUG True)
elseif(build_type STREQUAL RELWITHDEBINFO)
  set(BUILD_OPT True)
  set(BUILD_DEBUG True)
elseif(build_type STREQUAL RELEASE OR build_type STREQUAL MINSIZEREL)
  set(BUILD_OPT True)
else()
  message(WARNING "unknown build type: \"${CMAKE_BUILD_TYPE}\"")
endif()
