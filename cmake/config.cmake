
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(SYS_WINDOWS TRUE)
  add_definitions(-DSYSTEM_WINDOWS)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(SYS_LINUX TRUE)
  set(SYS_UNIX TRUE)
  add_definitions(-DSYSTEM_LINUX -DSYSTEM_UNIX)
else()
  message(ERROR "untested OS: ${CMAKE_SYSTEM_NAME}")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  set(COMP_GCC TRUE)
  add_definitions(-DGNU_EXTENSIONS)
elseif(MSVC_VERSION)
else()
  message(WARNING "untested compiler")
endif()

if(MATH_INLINE)
  add_definitions(-DMATH_INLINE)
endif()

if(GLDEBUG)
  add_definitions(-DGLDEBUG)
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
