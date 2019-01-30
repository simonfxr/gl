# Detect OS/Arch

macro(add_global_cmake_flags flags)
  set(types ${CMAKE_CONFIGURATION_TYPES})
  if(NOT types)
    set(types DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
  endif()

  foreach(ty "" ${types})
    if(ty)
      set(ty "_${ty}")
    endif()
    foreach(pref "" _C_FLAGS _CXX_FLAGS)
      set(v "CMAKE${pref}${ty}")
      if(DEFINED ${v})
        set($v "${${v}} ${flags}")
        message(STATUS "${v}=${${v}}")
      endif()
    endforeach()
  endforeach()
endmacro()

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
elseif(CMAKE_CXX_COMPILER MATCHES "zapcc[+][+]")
  set(COMP_ZAPCC True)
  set(COMP_CLANG True)
  set(COMP_GCCLIKE True)
  #add_global_cmake_flags("-I/usr/lib/zapcc/7.0.0/include -I/usr/include")
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

string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)

if(BUILD_TYPE STREQUAL DEBUG)
  set(BUILD_DEBUG True)
elseif(BUILD_TYPE STREQUAL RELWITHDEBINFO)
  set(BUILD_OPT True)
  set(BUILD_DEBUG True)
elseif(BUILD_TYPE STREQUAL RELEASE OR BUILD_TYPE STREQUAL MINSIZEREL)
  set(BUILD_OPT True)
else()
  message(WARNING "unknown build type: \"${CMAKE_BUILD_TYPE}\"")
endif()
