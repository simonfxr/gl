

set(ICC "/opt/intel/composerxe-2011.2.137/bin/intel64/icc")
set(IXX "/opt/intel/composerxe-2011.2.137/bin/intel64/icpc")

set(COMP_C gcc)
set(COMP_CXX g++)

set(CMAKE_C_COMPILER_INIT ${COMP_C})

set(CMAKE_C_COMPILER ${COMP_C})
set(CMAKE_CXX_COMPILER ${COMP_CXX})

project(sim-sfml)

#set(CMAKE_CXX_FLAGS "-xHOST -O3 -ipo -no-prec-div -DMATH_INLINE")

#set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-switch -O3 -DMATH_INLINE -DDEBUG -DGLDEBUG -march=native -fno-exceptions")

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-switch -DMATH_INLINE -DDEBUG -DGLDEBUG -fno-exceptions -O3 -march=native")

#set(CMAKE_CXX_FLAGS "-fopenmp" ${CMAKE_CXX_FLAGS})

set(CMAKE_LDFLAGS "-flto ${CMAKE_CXX_FLAGS}")

#set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-switch-enum -DGLDEBUG -DDEBUG -ggdb")

set(BIBLE $ENV{HOME}/src/oglsuperbible5-read-only)

include_directories(${BIBLE}/Src/GLTools/include)
include_directories(.)

find_package(PkgConfig)

pkg_check_modules(LIBGLEW QUIET glew)
include_directories(${LIBGLEW_INCLUDE_DIRS})
link_libraries(${LIBGLEW_LIBRARIES})

link_libraries(sfml-window sfml-graphics sfml-system)

link_libraries(-L.)
link_libraries(gltools)
link_libraries(-Wl,-rpath,$ENV{PWD})

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
  math/ivec3/impl.cpp)

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
  glt/RenderManager.cpp)

set(GE_SRC
  ge/GameLoop.hpp
  ge/GameLoop.cpp
  ge/GameWindow.hpp
  ge/GameWindow.cpp)

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
