set(CMU_PIC True)
set(CMAKE_POSITION_INDEPENDENT_CODE True)

set(build_shared_val 0)
if(BUILD_SHARED_LIBS)
  set(build_shared_val 1)
endif()

list(APPEND CMU_DEFINES "BUILD_SHARED_P=${build_shared_val}")

set(CMU_THREADS True)

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

list(APPEND CMU_DEFINES "ENABLE_GLDEBUG_P=${gldebug_val}")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

find_package(OpenGL REQUIRED)
find_package(OpenCL)

list(APPEND CMU_DEFINES "SOURCE_DIR=${CMAKE_SOURCE_DIR}")

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  if(ENABLE_STACKTRACES AND (CMU_OBJFMT_ELF OR CMU_OBJFMT_MACHO))
    pkg_check_modules(unwind IMPORTED_TARGET libunwind)
    pkg_check_modules(dw IMPORTED_TARGET libdw)
    if(NOT TARGET PkgConfig::unwind OR NOT TARGET PkgConfig::dw)
      message(WARNING "ENABLE_STACKTRACES set, but libunwind or libdw not found")
      set(ENABLE_STACKTRACES False)
    endif()
  endif()
endif()

message(STATUS "ENABLE_ASAN=${ENABLE_ASAN}")
message(STATUS "ENABLE_GLDEBUG=${ENABLE_GLDEBUG}")
message(STATUS "ENABLE_LTO=${ENABLE_LTO}")
message(STATUS "ENABLE_LIBCXX=${ENABLE_LIBCXX}")
message(STATUS "ENABLE_OPENMP=${ENABLE_OPENMP}")
message(STATUS "ENABLE_STACKTRACES=${ENABLE_STACKTRACES}")
message(STATUS "ENABLE_UBSAN=${ENABLE_UBSAN}")
message(STATUS "ENABLE_FANALYZER=${ENABLE_FANALYZER}")
