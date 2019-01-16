
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
				string(REGEX REPLACE "${pat}" "${repl}" "${v}" "${${v}}")
			endif()
		endforeach()
	endforeach()
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

set(san_flags)

if(USE_ASAN)
  set(san_flags address)
endif()

if(USE_UBSAN)
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
    -fno-exceptions
    -fno-rtti
    )
elseif(COMP_MSVC)
	list(APPEND GLOBAL_FLAGS
		/wd4201 #anon struct/union
		/wd4251 #inconsistent dll linkage
		/wd4204 #non-constant aggregate initializer
	)
  #list(APPEND GLOBAL_FLAGS "/EH-")
  #list(APPEND GLOBAL_LINK_FLAGS "/EH-")
  #list(APPEND GLOBAL_FLAGS "/GR-")
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

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

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

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  if(USE_UNWIND_STACKTRACES)
    pkg_check_modules(unwind IMPORTED_TARGET libunwind)
    pkg_check_modules(dw IMPORTED_TARGET libdw)
    if(NOT TARGET PkgConfig::unwind OR NOT TARGET PkgConfig::dw)
      message(WARNING "USE_UNWIND_STACKTRACES set, but libunwind or libdw not found")
      set(USE_UNWIND_STACKTRACES False)
    endif()
  endif()
endif()

if(COMP_CLANG OR COMP_GCC)
  set_option(USE_ASAN FALSE BOOL "enable -fsanitize=address")
  if(BUILD_DEBUG)
    set(ubsan_default TRUE)
  else()
    set(ubsan_default FALSE)
  endif()
  set_option(USE_UBSAN ${ubsan_default} BOOL "enable -fsanitize=undefined")
endif()

if(COMP_MSVC)
	replace_cmake_flags("/GR" "/GR-")
endif()

message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message(STATUS "BUILD_DEBUG=${BUILD_DEBUG}")
message(STATUS "BUILD_OPT=${BUILD_OPT}")
message(STATUS "HAVE_IPO=${HAVE_IPO}")
message(STATUS "USE_ASAN=${USE_ASAN}")
message(STATUS "USE_UBSAN=${USE_UBSAN}")
message(STATUS "USE_OPENMP=${USE_OPENMP}")
message(STATUS "GLDEBUG=${GLDEBUG}")
