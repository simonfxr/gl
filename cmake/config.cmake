
macro(cache_var_type var type)
  set(${var} ${${var}} CACHE ${type} "" FORCE)
endmacro()

macro(cache_var var)
  cache_var_type(${var} STRING)
endmacro()

set(GLOBAL_DEFINES)
set(GLOBAL_FLAGS)
set(GLOBAL_LINK_FLAGS)

if(BUILD_SHARED_LIBS)
  list(APPEND GLOBAL_DEFINES BUILD_SHARED=1)
endif()

if(BUILD_OPT)
  if(COMP_GCC)
    if(BUILD_DEBUG)
      list(APPEND GLOBAL_FLAGS -march=native -Og)
    else()
      list(APPEND GLOBAL_FLAGS -march=native -Ofast)
    endif()
  elseif(COMP_CLANG)
    if(BUILD_DEBUG)
      list(APPEND GLOBAL_FLAGS -march=native -O1)
    else()
      list(APPEND GLOBAL_FLAGS -march=native -O3 -ffast-math)
    endif()
  elseif(COMP_ICC)
    list(APPEND GLOBAL_FLAGS -xHOST -O3 -ipo -no-prec-div)
  endif()
endif()

if(COMP_CLANG)
#  list(APPEND GLOBAL_FLAGS -Wglobal-constructors)
endif()

if(COMP_CLANG AND USE_CLANG_TRAP_UNDEFINED)
  list(APPEND GLOBAL_FLAGS -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error -ftrap-function=__clang_trap_function)
endif()

if(USE_ADDRESS_SANITIZER)
  list(APPEND GLOBAL_FLAGS -fsanitize=address)
  list(APPEND GLOBAL_LINK_FLAGS -fsanitize=address)
endif()

if(COMP_GCCLIKE)
  list(APPEND GLOBAL_FLAGS -Wall -Wswitch-enum -Wdate-time -Werror=date-time)
  # list(APPEND GLOBAL_FLAGS -Werror)
  # list(APPEND GLOBAL_FLAGS -Weffc++)
  if (NOT COMP_ICC)
    list(APPEND GLOBAL_FLAGS -Wextra)
  else()
    list(APPEND GLOBAL_FLAGS "-diag-disable 10120")
  endif()
  if (COMP_GCC)
    list(APPEND GLOBAL_FLAGS -Wdouble-promotion)
  endif()
  list(APPEND GLOBAL_FLAGS 
    # -Wconversion -Wsign-conversion 
    # -Werror 
    # -Wold-style-cast
    )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
endif()

if(COMP_CLANG)
  list(APPEND GLOBAL_FLAGS 
    -Weverything
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-missing-prototypes
    -Wno-padded
    -Wno-gnu-anonymous-struct
    -Wno-nested-anon-types
    -Wno-gnu-statement-expression
    -Wno-return-std-move-in-c++11

    -Wno-used-but-marked-unused
    -Wno-missing-noreturn
    -Wno-float-equal
    -Wno-sign-conversion
    -Wno-conversion
    -Wno-double-promotion
    -Wno-shadow
    -Wno-gnu-zero-variadic-macro-arguments
    )
endif()

if(COMP_GCC)
#  list(APPEND GLOBAL_FLAGS 
#    -Wsuggest-attribute=const
#    -Wsuggest-attribute=pure
#    -Wsuggest-attribute=noreturn
#  )
endif()

if(COMP_ICC)
  list(APPEND GLOBAL_FLAGS -Wcheck)
endif()

if(BUILD_DEBUG)
  list(APPEND GLOBAL_DEFINES DEBUG=1)
  if(COMP_GCCLIKE)
    list(APPEND GLOBAL_FLAGS -ggdb)
  endif()

  # if(COMP_GCCLIKE)
  #   list(APPEND GLOBAL_FLAGS -fno-omit-frame-pointer)
  # endif()
endif()

if(USE_OPENMP)
  if(NOT DEFINED OPENMP_FOUND)
    find_package(OpenMP)
    if(OPENMP_FOUND)
      cache_var(OpenMP_C_FLAGS)
      cache_var(OpenMP_CXX_FLAGS)
      cache_var(OpenMP_EXE_LINKER_FLAGS)
    endif()
    cache_var_type(OPENMP_FOUND BOOL)
  endif()

  if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
  endif()
endif()

if(GLDEBUG)
  list(APPEND GLOBAL_DEFINES GLDEBUG=1)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

# set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
# set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

# if(SYS_WINDOWS)
#   set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
# endif()

find_package(OpenGL REQUIRED)

# if(NOT DEFINED GLEW_LIBRARY)
#   if(USE_GLEW_IN_TREE)
#     set(GLEW_ROOT_DIR "${PROJECT_SOURCE_DIR}/ext/glew")
#   endif()
#   find_package(GLEW REQUIRED)
# endif()

# if(NOT DEFINED GLFW_LIBRARY)
#   if(USE_GLFW_IN_TREE)
#     set(GLFW_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/ext/glfw3/include")
#     if(CL_VS_VERSION)
#       set(GLFW_LIBRARY_DIR "${PROJECT_SOURCE_DIR}/ext/glfw3/lib-msvc-${CL_VS_VERSION}")
#     endif()
#     find_library(GLFW_LIBRARY "glfw" HINTS "${GLFW_LIBRARY_DIR}")
#   else()
#     find_package(PkgConfig REQUIRED)
#     pkg_search_module(GLFW REQUIRED glfw3)
#     set(GLFW_LIBRARY "${GLFW_LIBRARIES}")
#     cache_var(GLFW_LIBRARY)
#   endif()
# endif()

find_package(OpenCL)

if(USE_OPENCL_HEADERS_IN_TREE)
  set(OPENCL_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/ext/opencl-headers/include")
else()
  set(OPENCL_INCLUDE_PATH "${OPENCL_INCLUDE_DIR}")
endif()

list(APPEND GLOBAL_DEFINES SOURCE_DIR="${CMAKE_SOURCE_DIR}")

if(BUILD_OPT)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT HAVE_IPO)
  if(HAVE_IPO)
    message(STATUS "Enabling IPO")
    set_state(HAVE_IPO "${HAVE_IPO}")
  endif()
endif()

set_state(GLOBAL_DEFINES "${GLOBAL_DEFINES}")
set_state(GLOBAL_FLAGS "${GLOBAL_FLAGS}")
set_state(GLOBAL_LINK_FLAGS "${GLOBAL_LINK_FLAGS}")
