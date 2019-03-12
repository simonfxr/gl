set(CMAKE_POSITION_INDEPENDENT_CODE True)

macro(replace_global_cmake_flags pat repl)
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

macro(add_global_cmake_linker_flags)
  set(types ${CMAKE_CONFIGURATION_TYPES})
  if(NOT types)
    set(types DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
  endif()

  foreach(ty "" ${types})
    if(ty)
      set(ty "_${ty}")
    endif()
    foreach(kind EXE SHARED STATIC MODULE)
      set(v "CMAKE_${kind}_LINKER_FLAGS${ty}")
      list(APPEND "${v}" ${ARGN})
    endforeach()
  endforeach()
endmacro()

set(GLOBAL_CXX_FLAGS)
set(GLOBAL_DEFINES)
set(GLOBAL_FLAGS)
set(GLOBAL_FLAGS_BOTH)
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
    # list(APPEND GLOBAL_FLAGS -march=native -Og)
  endif()
endif()

if(COMP_CLANG)
  # list(APPEND GLOBAL_FLAGS -Wglobal-constructors)
endif()

if(COMP_CLANG OR COMP_GCC)

  set(asan_default False)
  # if(BUILD_TYPE STREQUAL DEBUG) set(asan_default True) endif()
  set_option(ENABLE_ASAN ${asan_default} BOOL "enable -fsanitize=address")

  set(ubsan_default False)
  # if(BUILD_DEBUG) set(ubsan_default True) endif()
  set_option(ENABLE_UBSAN ${ubsan_default} BOOL "enable -fsanitize=undefined")

endif()

if(ENABLE_ASAN)
  list(APPEND GLOBAL_FLAGS_BOTH -fsanitize=address)
endif()

if(ENABLE_UBSAN)
  list(APPEND GLOBAL_FLAGS_BOTH -fsanitize=undefined
              # -fsanitize-undefined-trap-on-error
       )
endif()

if(ENABLE_ASAN OR ENABLE_UBSAN)
  list(APPEND GLOBAL_FLAGS -fno-omit-frame-pointer)
endif()

if(COMP_GCCLIKE)
  list(APPEND GLOBAL_FLAGS -Wall -Wswitch-enum)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    list(APPEND GLOBAL_FLAGS -Wdate-time -Werror=date-time)
  endif()
  list(APPEND GLOBAL_FLAGS -Werror=return-type)
  list(APPEND GLOBAL_CXX_FLAGS -fno-exceptions -fno-rtti)
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
              -Wno-gnu-statement-expression
              -Wno-assume)
  if(NOT COMP_ZAPCC)
    list(APPEND GLOBAL_FLAGS -Wno-return-std-move-in-c++11)
  endif()
endif()

if(COMP_CLANG AND ENABLE_LIBCXX)
  list(APPEND GLOBAL_CXX_FLAGS -stdlib=libc++)
  list(APPEND GLOBAL_LINK_FLAGS -stdlib=libc++)
  list(APPEND GLOBAL_DEFINES _LIBCPP_ENABLE_NODISCARD=1 _LIBCPP_ABI_VERSION=2)
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

if(NOT TARGET Threads::Threads AND COMP_ZAPCC)
  set(CMAKE_THREAD_LIBS_INIT -pthread)
  set(CMAKE_USE_PTHREADS_INIT 1)
  set(Threads_FOUND TRUE)
  add_library(Threads::Threads INTERFACE IMPORTED)
  set_property(TARGET Threads::Threads
               PROPERTY INTERFACE_COMPILE_OPTIONS
                        $<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler
                        -pthread>
                        $<$<NOT:$<COMPILE_LANGUAGE:CUDA>>:-pthread>)
  set_property(TARGET Threads::Threads
               PROPERTY INTERFACE_LINK_LIBRARIES -pthread)
endif()

if(ENABLE_OPENMP)
  if(NOT DEFINED OPENMP_FOUND)
    find_package(OpenMP)
    if(NOT OpenMP_FOUND)
      set(ENABLE_OPENMP False)
    endif()
  endif()
endif()

list(APPEND GLOBAL_DEFINES "GLDEBUG_LEVEL=${GLDEBUG_LEVEL}")

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
  if(ENABLE_STACKTRACES AND SYS_LINUX)
    pkg_check_modules(dw IMPORTED_TARGET libdw)
    if(NOT TARGET PkgConfig::dw)
      message(WARNING "ENABLE_STACKTRACES set, but libdw (elfutils) not found")
      set(ENABLE_STACKTRACES False)
    endif()
  endif()
endif()

if(COMP_MSVC)
  replace_global_cmake_flags("/GR" "/GR-")
endif()

if(COMP_CLANG)
  add_global_cmake_linker_flags("-fuse-ld=lld")
elseif(COMP_GCC)
  add_global_cmake_linker_flags("-fuse-ld=gold")
endif()

message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

message(STATUS "BUILD_DEBUG=${BUILD_DEBUG}")
message(STATUS "BUILD_OPT=${BUILD_OPT}")

message(STATUS "ENABLE_ASAN=${ENABLE_ASAN}")
message(STATUS "ENABLE_GLDEBUG=${ENABLE_GLDEBUG}")
message(STATUS "ENABLE_IPO=${ENABLE_IPO}")
message(STATUS "ENABLE_LIBCXX=${ENABLE_LIBCXX}")
message(STATUS "ENABLE_OPENMP=${ENABLE_OPENMP}")
message(STATUS "ENABLE_STACKTRACES=${ENABLE_STACKTRACES}")
message(STATUS "ENABLE_UBSAN=${ENABLE_UBSAN}")
