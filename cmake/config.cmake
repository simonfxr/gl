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

if(USE_ASAN)
  list(APPEND GLOBAL_FLAGS -fsanitize=address -fno-omit-frame-pointer)
  list(APPEND GLOBAL_LINK_FLAGS -fsanitize=address)
endif()

if(USE_UBSAN)
  list(APPEND GLOBAL_FLAGS -fsanitize=undefined -fno-omit-frame-pointer)
  list(APPEND GLOBAL_LINK_FLAGS -fsanitize=undefined)
endif()

if(COMP_GCCLIKE)
  list(APPEND GLOBAL_FLAGS -Wall -Wswitch-enum -Wdate-time -Werror=date-time)
  set(APPEND GLOBAL_FLAGS -fno-exceptions -fno-rtti)
elseif(COMP_MSVC)
  list(APPEND GLOBAL_FLAGS "/EH-")
  list(APPEND GLOBAL_LINK_FLAGS "/EH-")
  list(APPEND GLOBAL_FLAGS "/GR-")
endif()

if(COMP_GCC)
  list(APPEND GLOBAL_FLAGS
    -Wcast-align
    -Wcast-qual
    -Wchar-subscripts
    -Wcomment
    -Wdisabled-optimization
    -Wformat
    -Wformat-nonliteral
    -Wformat-security
    -Wformat-y2k
    -Wformat=2
    -Wimport
    -Winit-self
    -Winline
    -Winvalid-pch
    -Wmissing-field-initializers
    -Wmissing-format-attribute
    -Wmissing-include-dirs
    -Wmissing-noreturn
    -Wparentheses
    -Wpointer-arith
    -Wredundant-decls
    -Wreturn-type
    -Wsequence-point
    -Wsign-compare
    -Wstack-protector
    -Wstrict-aliasing
    -Wstrict-aliasing=2
    -Wswitch
    -Wswitch-enum
    -Wtrigraphs
    -Wuninitialized
    -Wunknown-pragmas
    -Wunreachable-code
    -Wunsafe-loop-optimizations
    -Wunused
    -Wunused-function
    -Wunused-label
    -Wunused-parameter
    -Wunused-value
    -Wunused-variable
    -Wvariadic-macros
    -Wvolatile-register-var
    -Wwrite-strings
    )
  list(APPEND GLOBAL_FLAGS -Wextra)
endif()

if(COMP_CLANG)
  list(APPEND GLOBAL_FLAGS
    -Weverything
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-conversion
    -Wno-documentation
    -Wno-documentation-unknown-command
    -Wno-double-promotion
    -Wno-float-equal
    -Wno-gnu-anonymous-struct
    -Wno-gnu-zero-variadic-macro-arguments
    -Wno-missing-noreturn
    -Wno-missing-prototypes
    -Wno-nested-anon-types
    -Wno-packed
    -Wno-padded
    -Wno-return-std-move-in-c++11
    )
endif()

if(COMP_ICC)
  list(APPEND GLOBAL_FLAGS -Wcheck)
endif()

if(BUILD_DEBUG)
  list(APPEND GLOBAL_DEFINES DEBUG=1)
  if(COMP_GCCLIKE)
    list(APPEND GLOBAL_FLAGS -ggdb)
  endif()
endif()

if(USE_OPENMP)
  if(NOT DEFINED OPENMP_FOUND)
    find_package(OpenMP)
    if(NOT OpenMP_FOUND)
      set(USE_OPENMP FALSE)
    endif()
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
find_package(OpenCL)

list(APPEND GLOBAL_DEFINES SOURCE_DIR="${CMAKE_SOURCE_DIR}")

if(BUILD_OPT)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT HAVE_IPO)
endif()

message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message(STATUS "BUILD_DEBUG=${BUILD_DEBUG}")
message(STATUS "BUILD_OPT=${BUILD_OPT}")
message(STATUS "HAVE_IPO=${HAVE_IPO}")
message(STATUS "USE_ASAN=${USE_ASAN}")
message(STATUS "USE_UBSAN=${USE_UBSAN}")
message(STATUS "USE_OPENMP=${USE_OPENMP}")
message(STATUS "GLDEBUG=${GLDEBUG}")
