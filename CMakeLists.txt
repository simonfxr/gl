
set(CMAKE_C_COMPILER_INIT clang)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-switch-enum -DGLDEBUG -DDEBUG")

project(sim-sfml)

set(BIBLE $ENV{HOME}/src/oglsuperbible5-read-only)

include_directories(${BIBLE}/Src/GLTools/include)
include_directories(.)

find_package(PkgConfig)

pkg_check_modules(LIBGLEW QUIET glew)
include_directories(${LIBGLEW_INCLUDE_DIRS})
link_libraries(${LIBGLEW_LIBRARIES})

link_libraries(-L.)
link_libraries(sfml-window sfml-graphics sfml-system)
link_libraries(gltools)
link_libraries(-Wl,-rpath,$ENV{PWD})

add_executable(sim-sfml
  sim-sfml.cpp
  defs.h

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

  ge/GameLoop.hpp
  ge/GameLoop.cpp
  ge/GameWindow.hpp
  ge/GameWindow.cpp
)
