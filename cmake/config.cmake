
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(SYS_WINDOWS TRUE)
  add_definitions(-DSYSTEM_WINDOWS=1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(SYS_LINUX TRUE)
  set(SYS_UNIX TRUE)
  add_definitions(-DSYSTEM_LINUX=1 -DSYSTEM_UNIX=1)
else()
  message(ERROR "untested OS: ${CMAKE_SYSTEM_NAME}")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  set(COMP_GCCLIKE TRUE)
elseif(MSVC_VERSION)
  set(COMP_CL TRUE)
endif()

if(CMAKE_CXX_COMPILER)
  get_filename_component(CXX_COMPILER ${CMAKE_CXX_COMPILER} NAME)
else()
  set(CXX_COMPILER "")
endif()

if(${CXX_COMPILER} MATCHES "^g\\+\\+")
  set(COMP_GCC TRUE)
  set(COMP_GCCLIKE TRUE)
elseif(${CXX_COMPILER} MATCHES "^clang\\+\\+")
  set(COMP_CLANG TRUE)
  set(COMP_GCCLIKE TRUE)
elseif(${CXX_COMPILER} MATCHES "^icpc")
  set(COMP_ICC TRUE)
  set(COMP_GCCLIKE TRUE)
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
  set(BUILD_DEBUG TRUE)
elseif(${CMAKE_BUILD_TYPE} STREQUAL Release)
  set(BUILD_OPT TRUE)
elseif(${CMAKE_BUILD_TYPE} STREQUAL RelWithDebInfo)
  set(BUILD_DEBUG TRUE)
  set(BUILD_OPT TRUE)
elseif(${CMAKE_BUILD_TYPE} STREQUAL MinSizeRel)
  set(BUILD_OPT TRUE)
else()
  message(WARNING "unknown build type: \"${CMAKE_BUILD_TYPE}\"")
endif()

if(COMP_GCCLIKE)
  add_definitions(-DGNU_EXTENSIONS=1)
endif()

if(COMP_CL)
  add_definitions(-DCOMPILER_CL)
endif()

if(BUILD_OPT)
  if(COMP_GCC)
    add_definitions(-march=native -Ofast)
  elseif(COMP_CLANG)
    add_definitions(-march=native -O3 -ffast-math)
  elseif(COMP_ICC)
    add_definitions(-xHOST -O3 -ipo -no-prec-div)
  endif()

  if(USE_LTO)
    if(COMP_GCC)
#      set(LTO_FLAGS -flto -flto-partition=none -fwhole-program)
      set(LT_FLAGS -flto)
      add_definitions(${LTO_FLAGS})
      link_libraries(${LTO_FLAGS})
    else()
      message(WARNING "LTO not supported for compiler: \"${CMAKE_CXX_COMPILER}\"")
    endif()
  endif()
endif()

if(BUILD_DEBUG AND COMP_CLANG)
  add_definitions(-ftrapv -fcatch-undefined-behavior)
endif()

if(COMP_GCCLIKE)
  add_definitions(-Wall)
  if (NOT COMP_ICC)
    add_definitions(-Wextra)
  else()
    add_definitions(-diag-disable 10120)
    link_libraries("-diag-disable 11000" "-diag-disable 11006" "-diag-disable 11001")
  endif()
  add_definitions(
    # -Wconversion -Wsign-conversion 
    # -Werror 
    # -Wold-style-cast 
    # -Wdouble-promotion 
    -fno-exceptions -fno-rtti)
endif()

if(COMP_GCC)
  add_definitions(
#    -Wsuggest-attribute=const 
#    -Wsuggest-attribute=pure 
#    -Wsuggest-attribute=noreturn
    )
endif()

if(COMP_ICC)
  add_definitions(-Wcheck)
endif()

if(BUILD_DEBUG)
  add_definitions(-DDEBUG=1)
endif()

if(NEED_OPENMP AND USE_OPENMP)
  if(COMP_GCC)
    add_definitions(-fopenmp)
    link_libraries(-fopenmp)
  elseif(COMP_ICC)
    add_definitions(-openmp)
    link_libraries(-openmp)
  else()
    message(WARNING "compiler doesnt seem to support openmp")
  endif()
endif()


if(MATH_INLINE)
  add_definitions(-DMATH_INLINE=1)
endif()

if(GLDEBUG)
  add_definitions(-DGLDEBUG=1)
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

include_directories(${GLEW_INCLUDE_PATH})

# if(SYS_WINDOWS)
#   add_definitions(-DGLEW_STATIC)
# endif()

find_package(SFML 2 COMPONENTS system window graphics REQUIRED)

include_directories(${SFML_INCLUDE_DIR})

