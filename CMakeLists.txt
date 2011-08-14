
cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

set(BUILD_SHARED TRUE CACHE BOOL "TRUE to build shared libs, FALSE to build static libs")

set(GLDEBUG TRUE CACHE BOOL "TRUE to enable error checking after every opengl call")

set(MATH_INLINE TRUE CACHE BOOL "TRUE to inline all custom math functions")

set(GLEW_ROOT_DIRECTORY "" CACHE STRING "GLEW installation directory")

set(SFMLDIR "" CACHE STRING "SFML installation directory")

project(gl-stuff)

include_directories(src)

add_subdirectory(src)
add_subdirectory(programs)
add_subdirectory(test)
