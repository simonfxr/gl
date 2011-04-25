
# set(COMP_C clang)
# set(COMP_CXX clang++)

# set(CMAKE_C_COMPILER_INIT ${COMP_C})

# set(CMAKE_C_COMPILER ${COMP_C})
# set(CMAKE_CXX_COMPILER ${COMP_CXX})

project(sim-sfml)

#set(CMAKE_CXX_FLAGS "-xHOST -O3 -ipo -no-prec-div -DMATH_INLINE")

#set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-switch -O3 -DMATH_INLINE -DDEBUG -DGLDEBUG -march=native -fno-exceptions")

#set(GRAPHITE_OPTS "-floop-strip-mine -floop-block -floop-interchange")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-switch -fno-exceptions -fno-rtti -fopenmp")

set(CMAKE_LDFLAGS "${CMAKE_CXX_FLAGS}")

#set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-switch-enum -DGLDEBUG -DDEBUG -ggdb")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")

#string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

find_package(SFML REQUIRED)
include_directories(${SFML_INCLUDE_DIR})
link_libraries(${SFML_LIBRARIES})

link_libraries(sfml-window sfml-graphics sfml-system)

# foreach(lib ${SFML_LIBS})
#   link_libraries(${SFML_${lib}_LIBRARY_RELEASE})
# endforeach()

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

include_directories(${GLEW_INCLUDE_PATH})

add_definitions("-DGLEW_STATIC")

link_libraries(${GLEW_LIBRARY} ${OPENGL_gl_LIBRARY})

include_directories(.)

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

set(GLT_SRC

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
  glt/Ref.hpp)

set(GE_SRC
  ge/GameLoop.hpp
  ge/GameLoop.cpp
  ge/GameWindow.hpp
  ge/GameWindow.cpp
  ge/WindowRenderTarget.hpp
  ge/WindowRenderTarget.cpp)

add_executable(sim-sfml
  sim-sfml.cpp
  defs.h
  sim.hpp
  sim.cpp
  
  ${MATH_SRC}
  ${GLT_SRC}
  ${GE_SRC})

add_executable(voxel-world
  voxel-world.cpp
  defs.h
  
  ${MATH_SRC}
  ${GLT_SRC}
  ${GE_SRC})
