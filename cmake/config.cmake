set(CMAKE_POSITION_INDEPENDENT_CODE True)

macro(replace_cmake_flags pat repl)
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
      if(DEFINED "${v}")
        string(REGEX
               REPLACE "${pat}"
                       "${repl}"
                       "${v}"
                       "${${v}}")
      endif()
    endforeach()
  endforeach()
endmacro()

set(GLOBAL_DEFINES)
set(GLOBAL_FLAGS)
set(GLOBAL_LINK_FLAGS)

set(build_shared_val 0)
if(BUILD_SHARED_LIBS)
  set(build_shared_val 1)
endif()

list(APPEND GLOBAL_DEFINES "BUILD_SHARED_P=${build_shared_val}")

if(BUILD_OPT)
  if(COMP_GCC OR COMP_CLANG)
    list(APPEND GLOBAL_FLAGS -march=native -Ofast)
  elseif(COMP_ICC)
    list(APPEND GLOBAL_FLAGS
                -xHOST
                -O3
                -ipo
                -no-prec-div)
  endif()
endif()

if(BUILD_DEBUG)
  if(COMP_GCC OR COMP_CLANG)
    list(APPEND GLOBAL_FLAGS -march=native -Og)
  endif()
endif()

if(COMP_CLANG)
  # list(APPEND GLOBAL_FLAGS -Wglobal-constructors)
endif()

set(san_flags)

if(ENABLE_ASAN)
  set(san_flags address)
endif()

if(ENABLE_UBSAN)
  if(san_flags)
    set(san_flags "${san_flags},undefined")
  else()
    set(san_flags undefined)
  endif()
endif()

if(san_flags)
  list(APPEND GLOBAL_FLAGS "-fsanitize=${san_flags}" -fno-omit-frame-pointer)
  list(APPEND GLOBAL_LINK_FLAGS "-fsanitize=${san_flags}")
endif()

if(COMP_GCCLIKE)
  list(APPEND GLOBAL_FLAGS -Wall -Wswitch-enum)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    list(APPEND GLOBAL_FLAGS -Wdate-time -Werror=date-time)
  endif()
  list(APPEND GLOBAL_FLAGS
              -Werror=return-type
              -fno-exceptions
              -fno-rtti)
elseif(COMP_MSVC)
  list(APPEND GLOBAL_FLAGS
              /wd4201 # anon struct/union
              /wd4251 # inconsistent dll linkage
              /wd4204 # non-constant aggregate initializer
       )
  # list(APPEND GLOBAL_FLAGS "/EH-") list(APPEND GLOBAL_LINK_FLAGS "/EH-")
  # list(APPEND GLOBAL_FLAGS "/GR-")
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
              -Wwrite-strings)
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
              -Wno-gnu-statement-expression
              -Wno-assume)
endif()

if(COMP_CLANG AND ENABLE_LIBCXX)
  list(APPEND GLOBAL_FLAGS -stdlib=libc++)
  list(APPEND GLOBAL_LINK_FLAGS -stdlib=libc++)
  list(APPEND GLOBAL_DEFINES _LIBCPP_ENABLE_NODISCARD=1)
endif()

if(COMP_ICC)
  list(APPEND GLOBAL_FLAGS -Wcheck)
endif()

if(BUILD_DEBUG)
  if(COMP_GCCLIKE)
    list(APPEND GLOBAL_FLAGS -ggdb)
  endif()
endif()

set(CMAKE_THREAD_PREFER_PTHREAD True)
set(THREADS_PREFER_PTHREAD_FLAG True)

find_package(Threads)

if(ENABLE_OPENMP)
  if(NOT DEFINED OPENMP_FOUND)
    find_package(OpenMP)
    if(NOT OpenMP_FOUND)
      set(ENABLE_OPENMP False)
    endif()
  endif()
endif()


set(gldebug_val 0)
if(ENABLE_GLDEBUG)
  set(gldebug_val 1)
endif()

list(APPEND GLOBAL_DEFINES "ENABLE_GLDEBUG_P=${gldebug_val}")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# if(SYS_WINDOWS) set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin) endif()

find_package(OpenGL REQUIRED)
find_package(OpenCL)

list(APPEND GLOBAL_DEFINES "SOURCE_DIR=${CMAKE_SOURCE_DIR}")

if(BUILD_OPT)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT ENABLE_IPO)
endif()

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  if(ENABLE_STACKTRACES AND SYS_UNIX)
    pkg_check_modules(unwind IMPORTED_TARGET libunwind)
    pkg_check_modules(dw IMPORTED_TARGET libdw)
    if(NOT TARGET PkgConfig::unwind OR NOT TARGET PkgConfig::dw)
      message(
        WARNING "ENABLE_STACKTRACES set, but libunwind or libdw not found")
      set(ENABLE_STACKTRACES False)
    endif()
  endif()
endif()

if(COMP_CLANG OR COMP_GCC)
  set_option(ENABLE_ASAN False BOOL "enable -fsanitize=address")
  if(BUILD_DEBUG)
    set(ubsan_default True)
  else()
    set(ubsan_default False)
  endif()
  set_option(ENABLE_UBSAN ${ubsan_default} BOOL "enable -fsanitize=undefined")
endif()

if(COMP_MSVC)
  replace_cmake_flags("/GR" "/GR-")
endif()

message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

message(STATUS "BUILD_DEBUG=${BUILD_DEBUG}")
message(STATUS "BUILD_OPT=${BUILD_OPT}")

message(STATUS "ENABLE_ASAN=${ENABLE_ASAN}")
message(STATUS "ENABLE_GLDEBUG=${ENABLE_GLDEBUG}")
message(STATUS "ENABLE_IPO=${ENABLE_IPO}")
message(STATUS "ENABLE_LIBCXX=${ENABLE_LIBCXX}")
message(STATUS "ENABLE_OPENMP=${ENABLE_OPENMP}")
message(STATUS "ENABLE_UBSAN=${ENABLE_UBSAN}")
