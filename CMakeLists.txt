
set(MATH_INLINE TRUE CACHE BOOL "TRUE to inline all math functions")
set(GLDEBUG TRUE CACHE BOOL "TRUE to check for errors after every OpenGL call")

if (CMAKE_CXX_COMPILER)
  string(REGEX REPLACE "[^/]*/" "" CXX_COMP ${CMAKE_CXX_COMPILER})
else()
  set(CXX_COMP "")
endif()

if(CXX_COMP STREQUAL "clang++")
  set(COMP_CLANG "yes")
  add_definitions("-DGNU_EXTENSIONS")
elseif(CXX_COMP STREQUAL "g++")
  set(COMP_GCC "yes")
  add_definitions("-DGNU_EXTENSIONS")
elseif(CXX_COMP STREQUAL "icpc")
  set(COMP_ICC "yes")
  add_definitions("-DGNU_EXTENSIONS")
elseif(CXX_COMP STREQUAL "ghc-gcc")
  set(COMP_GCC "yes")
  add_definitions("-DGNU_EXTENSIONS")
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)

if(BUILD_TYPE STREQUAL "DEBUG")
  set(BUILD_DEBUG "yes")
elseif(BUILD_TYPE STREQUAL "RELEASE")
  set(BUILD_RELEASE "yes")
  set(BUILD_OPT "yes")
elseif(BUILD_TYPE STREQUAL "DEBUGOPT")
  set(CMAKE_BUILD_TYPE "Debug")
  set(BUILD_OPT "yes")
  set(BUILD_DEBUG "yes")
else()
  message(WARNING "unknown build type: \"${CMAKE_BUILD_TYPE}\", using Release")
  set(CMAKE_BUILD_TYPE "Release")
  set(BUILD_RELEASE "yes")
  set(BUILD_OPT "yes")
endif()

project(sim-sfml)

#add_definitions("-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free")
link_libraries(tcmalloc)

include_directories("${CMAKE_SOURCE_DIR}/ext/include")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(SYS_UNIX "yes")
  add_definitions("-DSYSTEM_LINUX")
else()
    message(WARNING "Unsupported operating system")
endif()

add_definitions("-Wall -fno-exceptions -fno-rtti")

if(GLDEBUG)
  add_definitions("-DGLDEBUG")
endif()

if(COMP_GCC)
  add_definitions("-fopenmp")
  link_libraries("-fopenmp")

  # if(BUILD_DEBUG)
  #   add_definitions("-Weffc++")
  # endif()
endif()

if(COMP_GCC OR COMP_CLANG)
  add_definitions("-Wextra")
  if(BUILD_DEBUG)
    add_definitions("-ggdb -fstack-protector")
  endif()
endif()

if(BUILD_DEBUG AND COMP_CLANG)
  add_definitions("-ftrapv -fcatch-undefined-behavior")
endif()

if(BUILD_OPT)
  if(COMP_GCC OR COMP_CLANG)
    add_definitions("-O3 -march=native -ffast-math")
  elseif(COMP_ICC)
    add_definitions("-xHOST -O3 -ipo -no-prec-div")
  endif()
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if(SYS_UNIX)
  link_libraries(rt)
endif()

find_package(SFML 2 COMPONENTS system window graphics REQUIRED)
include_directories(${SFML_INCLUDE_DIR})
link_libraries(${SFML_LIBRARIES})

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

include_directories(${GLEW_INCLUDE_PATH})

add_definitions("-DGLEW_STATIC")

link_libraries(${GLEW_LIBRARY} ${OPENGL_gl_LIBRARY})

include_directories(${CMAKE_SOURCE_DIR})


set(MATH_SRC
  math/vec2.hpp
  math/vec2/type.hpp
  math/vec3.hpp
  math/vec4.hpp
  math/mat3.hpp
  math/mat4.hpp
  math/real.hpp
  math/plane.hpp
  math/plane.cpp
  math/ivec3.hpp
  math/quat.hpp)

if(NOT MATH_INLINE)
  set(MATH_SRC
    ${MATH_SRC}
    math/vec2/impl.cpp
    math/vec3/impl.cpp
    math/vec4/impl.cpp
    math/mat3/impl.cpp
    math/mat4/impl.cpp
    math/real/impl.cpp
    math/ivec3/impl.cpp
    math/quat/impl.cpp)
else()
  add_definitions("-DMATH_INLINE")
endif()

set(DATA_SRC
  data/Ref.hpp
  data/Array.hpp)

set(ERROR_SRC
  error/error.hpp
  error/error.cpp)

add_library(err SHARED ${ERROR_SRC})
link_directories(${ERR_BINARY_DIR})

set(GLT_SRC
  glt/utils.hpp
  glt/utils.cpp
  glt/ShaderManager.hpp
  glt/ShaderManager.cpp
  glt/ShaderProgram.hpp
  glt/ShaderProgram.cpp
  glt/ShaderCompiler.hpp
  glt/ShaderCompiler.cpp
  glt/Uniforms.hpp
  glt/Uniforms.cpp
  glt/Preprocessor.hpp
  glt/Preprocessor.cpp
  glt/GLSLPreprocessor.hpp
  glt/GLSLPreprocessor.cpp
  glt/Frame.hpp
  glt/Frame.cpp
  glt/GeometryTransform.hpp
  glt/GeometryTransform.cpp
  glt/Transformations.hpp
  glt/Transformations.cpp
  glt/ViewFrustum.hpp
  glt/ViewFrustum.cpp
  glt/RenderManager.hpp
  glt/RenderManager.cpp
  glt/RenderTarget.hpp
  glt/RenderTarget.cpp
  glt/TextureHandle.hpp
  glt/TextureHandle.cpp
  glt/TextureRenderTarget.hpp
  glt/TextureRenderTarget.cpp
  glt/TextureRenderTarget3D.hpp
  glt/TextureRenderTarget3D.cpp
  glt/Mesh.hpp
  glt/Mesh.cpp
  glt/glstate.hpp
  glt/glstate.cpp)

add_library(glt SHARED ${GLT_SRC})
link_directories(${GLT_BINARY_DIR})

set(SYS_SRC
  sys/fs/fs.hpp
  sys/fs/fs_default.cpp
  sys/clock.hpp)

if(SYS_UNIX)
  set(SYS_SRC
    ${SYS_SRC}
    sys/fs/fs_unix.hpp
    sys/fs/fs_unix.cpp
    sys/clock_unix.cpp)
endif()

add_library(sys SHARED ${SYS_SRC})
link_directories(${SYS_BINARY_DIR})

set(GE_SRC
  ge/GameLoop.hpp
  ge/GameLoop.cpp
  ge/GameWindow.hpp
  ge/GameWindow.cpp
  ge/WindowRenderTarget.hpp
  ge/WindowRenderTarget.cpp
  ge/Event.hpp
  ge/EngineEvents.hpp
  ge/Engine.hpp
  ge/Engine.cpp
  ge/EngineOptions.hpp
  ge/EngineOptions.cpp
  ge/Init.hpp
  ge/Init.cpp
  ge/Command.hpp
  ge/Command.cpp
  ge/Commands.hpp
  ge/Commands.cpp
  ge/CommandArgs.hpp
  ge/CommandArgs.cpp
  ge/CommandParams.hpp
  ge/CommandParams.cpp
  ge/KeyBinding.hpp
  ge/KeyBinding.cpp
  ge/CommandProcessor.hpp
  ge/CommandProcessor.cpp
  ge/Tokenizer.hpp
  ge/Tokenizer.cpp
  ge/KeyHandler.hpp
  ge/KeyHandler.cpp
  ge/Camera.hpp
  ge/Camera.cpp
  ge/Timer.hpp)

add_library(ge SHARED ${GE_SRC})
link_directories(${GE_BINARY_DIR})

add_executable(sim-sfml
  sim-sfml.cpp
  defs.h
  sim.hpp
  sim.cpp
  
  ${MATH_SRC}
  ${DATA_SRC})

target_link_libraries(sim-sfml sys err glt ge)

link_directories("${CMAKE_SOURCE_DIR}/ext/lib")

add_executable(voxel-world
  voxel-world.cpp
  defs.h
  
  ${MATH_SRC}
  ${DATA_SRC})

target_link_libraries(voxel-world sys err glt ge)

add_executable(queryExtension
  queryExtension.cpp
  defs.h
  ${MATH_SRC})

target_link_libraries(queryExtension sys glt err ge)

add_executable(math_test
  math_test.cpp
  ${MATH_SRC})

target_link_libraries(math_test sys err)

# add_executable(ref_test
#   ref_test.cpp
#   ${MATH_SRC})

# target_link_libraries(ref_test err)

# add_executable(teapot
#   teapot.cpp
#   defs.h
  
#   teapot/mesh.h
#   teapot/parse_sply.hpp
#   teapot/parse_sply.cpp)

# target_link_libraries(teapot sys glt err ge)
  
