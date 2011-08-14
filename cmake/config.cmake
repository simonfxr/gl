
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
  set(COMP_GCC TRUE)
  add_definitions(-DGNU_EXTENSIONS)
elseif(MSVC_VERSION)
else()
  message(WARNING "untested compiler")
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

