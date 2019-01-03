
macro(cache_var_type var type)
  set(${var} ${${var}} CACHE ${type} "" FORCE)
endmacro()

macro(cache_var var)
  cache_var_type(${var} STRING)
endmacro()

if(BUILD_SHARED_LIBS)
  add_definitions(-DBUILD_SHARED=1)
endif()

if(BUILD_OPT)
  if(COMP_GCC)
    if(BUILD_DEBUG)
      add_definitions(-march=native -Og)
    else()
      add_definitions(-march=native -Ofast)
    endif()
  elseif(COMP_CLANG)
    if(BUILD_DEBUG)
      add_definitions(-march=native -O1)
    else()
      add_definitions(-march=native -O3 -ffast-math)
    endif()
  elseif(COMP_ICC)
    add_definitions(-xHOST -O3 -ipo -no-prec-div)
  endif()
endif()

if(COMP_CLANG)
#  add_definitions(-Wglobal-constructors)
endif()

if(COMP_CLANG AND USE_CLANG_TRAP_UNDEFINED)
  add_definitions(-fsanitize=undefined-trap -fsanitize-undefined-trap-on-error -ftrap-function=__clang_trap_function)
endif()

if(COMP_CLANG AND USE_CLANG_ADDRESS_SANITIZER)
  add_definitions(-fsanitize=address)
  link_libraries(-fsanitize=address)
endif()

if(COMP_GCCLIKE)
  add_definitions(-Wall -Wdate-time -Werror=date-time)
  # add_definitions(-Werror)
  # add_definitions(-Weffc++)
  if (NOT COMP_ICC)
    add_definitions(-Wextra)
  else()
    add_definitions("-diag-disable 10120")
    link_libraries("-diag-disable 11000" "-diag-disable 11006" "-diag-disable 11001" "-diag-disable 11021")
  endif()
  if (COMP_GCC)
    add_definitions(-Wdouble-promotion)
  endif()
  add_definitions(
    # -Wconversion -Wsign-conversion 
    # -Werror 
    # -Wold-style-cast
    )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
endif()

if(COMP_CLANG)
  add_definitions(
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
#  add_definitions(
#    -Wsuggest-attribute=const
#    -Wsuggest-attribute=pure
#    -Wsuggest-attribute=noreturn
#  )
endif()

if(COMP_ICC)
  add_definitions(-Wcheck)
endif()

if(BUILD_DEBUG)
  add_definitions(-DDEBUG=1)
  if(COMP_GCCLIKE)
    add_definitions(-ggdb)
  endif()

  # if(COMP_GCCLIKE)
  #   add_definitions(-fno-omit-frame-pointer)
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

if(MATH_INLINE)
  add_definitions(-DMATH_INLINE=1)
endif()

if(GLDEBUG)
  add_definitions(-DGLDEBUG=1)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

if(SYS_WINDOWS)
  set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
endif()

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

find_package(OpenCL REQUIRED)

if(USE_OPENCL_HEADERS_IN_TREE)
  set(OPENCL_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/ext/opencl-headers/include")
else()
  set(OPENCL_INCLUDE_PATH "${OPENCL_INCLUDE_DIR}")
endif()

add_definitions(-DSOURCE_DIR="${CMAKE_SOURCE_DIR}")
