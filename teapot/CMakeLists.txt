
# set(CMAKE_C_COMPILER_INIT clang)

# set(CMAKE_C_COMPILER clang)
# set(CMAKE_CXX_COMPILER clang++)

project(teapot)

#set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-switch-enum -DGLDEBUG -DDEBUG -O3 -march=native")

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-switch -O3")

include_directories(..)

find_package(PkgConfig)

pkg_check_modules(LIBGLEW QUIET glew)
include_directories(${LIBGLEW_INCLUDE_DIRS})
link_libraries(${LIBGLEW_LIBRARIES})

link_libraries(sfml-window sfml-graphics sfml-system)

add_executable(teapot
  teapot.cpp
  parse_sply.hpp
  parse_sply.cpp
  ../defs.h

  ../math/vec2.hpp
  ../math/vec2/impl.cpp
  ../math/vec3.hpp
  ../math/vec3/impl.cpp
  ../math/vec4.hpp
  ../math/vec4/impl.cpp
  ../math/mat3.hpp
  ../math/mat3/impl.cpp
  ../math/mat4.hpp
  ../math/mat4/impl.cpp
  ../math/math.hpp
  ../math/math/impl.cpp
  ../math/plane.hpp
  ../math/plane.cpp

  ../glt/utils.hpp
  ../glt/utils.cpp
  ../glt/error.hpp
  ../glt/error.cpp
  ../glt/ShaderManager.hpp
  ../glt/ShaderManager.cpp
  ../glt/ShaderProgram.hpp
  ../glt/ShaderProgram.cpp
  ../glt/Uniforms.hpp
  ../glt/Uniforms.cpp
  ../glt/GenBatch.hpp
  ../glt/GenBatch.cpp
  ../glt/Preprocessor.hpp
  ../glt/Preprocessor.cpp
  ../glt/GLSLPreprocessor.hpp
  ../glt/GLSLPreprocessor.cpp
  ../glt/Frame.hpp
  ../glt/Frame.cpp
  ../glt/GeometryTransform.hpp
  ../glt/GeometryTransform.cpp
  ../glt/Transformations.hpp
  ../glt/Transformations.cpp
  ../glt/ViewFrustum.hpp
  ../glt/ViewFrustum.cpp
  ../glt/RenderManager.hpp
  ../glt/RenderManager.cpp
  ../glt/TextureHandle.hpp
  ../glt/TextureHandle.cpp
  ../glt/RenderTarget.hpp
  ../glt/RenderTarget.cpp
  ../glt/ShaderObject.hpp
  ../glt/ShaderObject.cpp
  ../glt/TextureRenderTarget.hpp
  ../glt/TextureRenderTarget.cpp
  ../glt/Mesh.hpp
  ../glt/Mesh.cpp
  ../glt/CubeMesh.hpp

  ../ge/GameLoop.hpp
  ../ge/GameLoop.cpp
  ../ge/GameWindow.hpp
  ../ge/GameWindow.cpp
  ../ge/WindowRenderTarget.hpp
  ../ge/WindowRenderTarget.cpp)
