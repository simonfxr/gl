
string(REGEX REPLACE "[^/]*/" "" CXX_COMP ${CMAKE_CXX_COMPILER})

if(CXX_COMP STREQUAL "clang++")
  set(COMP_CLANG "yes")
elseif(CXX_COMP STREQUAL "g++")
  set(COMP_GCC "yes")
elseif(CXX_COMP STREQUAL "icpc")
  set(COMP_ICC "yes")
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

add_definitions("-Wall -fno-exceptions -fno-rtti")

if(COMP_GCC)
  add_definitions("-fopenmp")
  link_libraries("-fopenmp")
endif()

if(COMP_GCC OR COMP_CLANG)
  add_definitions("-Wextra -Wno-switch")
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

find_package(SFML REQUIRED)
include_directories(${SFML_INCLUDE_DIR})
link_libraries(${SFML_LIBRARIES})
link_directories("/usr/local/lib")

link_libraries(sfml-window sfml-graphics sfml-system)

# foreach(lib ${SFML_LIBS})
#   link_libraries(${SFML_${lib}_LIBRARY_RELEASE})
# endforeach()

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

include_directories(${GLEW_INCLUDE_PATH})

#add_definitions("-DGLEW_STATIC")

link_libraries(${GLEW_LIBRARY} ${OPENGL_gl_LIBRARY})

include_directories(${CMAKE_SOURCE_DIR})

set(MATH_SRC

  math/vec2.hpp
  math/vec2/impl.cpp
  math/vec3.hpp
  math/vec3/impl.cpp
  math/vec4.hpp
  math/vec4/impl.cpp
  math/mat3.hpp
  math/mat3/impl.cpp
  math/mat4.hpp
  math/mat4/impl.cpp
  math/math.hpp
  math/math/impl.cpp
  math/plane.hpp
  math/plane.cpp
  math/ivec3.hpp
  math/ivec3/impl.cpp
  math/quat.hpp
  math/quat/impl.cpp)

set(DATA_SRC
  data/Ref.hpp
  data/Array.hpp)

set(GLT_SRC

  error/error.hpp
  error/error.cpp

  glt/utils.hpp
  glt/utils.cpp
  glt/ShaderManager.hpp
  glt/ShaderManager.cpp
  glt/ShaderProgram.hpp
  glt/ShaderProgram.cpp
  glt/Uniforms.hpp
  glt/Uniforms.cpp
  glt/GenBatch.hpp
  glt/GenBatch.cpp
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
  glt/ShaderObject.hpp
  glt/ShaderObject.cpp
  glt/Mesh.hpp
  glt/Mesh.cpp)

set(SYS_SRC
  sys/fs/fs.hpp
  sys/fs/fs_unix.hpp
  sys/fs/fs_unix.cpp)

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
  ge/Init.hpp
  ge/Init.cpp
  ge/Command.hpp
  ge/Command.cpp
  ge/Commands.hpp
  ge/Commands.cpp
  ge/CommandArgs.hpp
  ge/CommandArgs.cpp
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

add_executable(sim-sfml
  sim-sfml.cpp
  defs.h
  sim.hpp
  sim.cpp
  
  ${MATH_SRC}
  ${DATA_SRC}
  ${GLT_SRC}
  ${GE_SRC}
  ${SYS_SRC})

add_executable(voxel-world
  voxel-world.cpp
  defs.h
  
  ${MATH_SRC}
  ${DATA_SRC}
  ${GLT_SRC}
  ${GE_SRC}
  ${SYS_SRC})

add_executable(engine-test
  engine-test.cpp
  defs.h
  
  ${MATH_SRC}
  ${DATA_SRC}
  ${GLT_SRC}
  ${GE_SRC}
  ${SYS_SRC})

add_executable(tokenizer
  tokenizer.cpp
  defs.h

  ${MATH_SRC}
  ${DATA_SRC}
  ${GLT_SRC}
  ${GE_SRC}
  ${SYS_SRC})
