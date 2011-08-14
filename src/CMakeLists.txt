
include(${PROJECT_SOURCE_DIR}/cmake/config.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/macros.cmake)

set(MATH_SRC
  math/vec2/impl.cpp
  math/vec3/impl.cpp
  math/vec4/impl.cpp
  math/mat3/impl.cpp
  math/mat4/impl.cpp
  math/real/impl.cpp
  math/ivec3/impl.cpp
  math/quat/impl.cpp
  math/plane.cpp)

if(NOT MATH_INLINE)
  def_lib(vec-math SOURCES ${MATH_SRC})
endif()

set(ERR_SRC
  err/err.cpp)

def_lib(err SOURCES ${ERR_SRC})

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
elseif(SYS_WINDOWS)
  set(SYS_SRC
    ${SYS_SRC}
    sys/fs/fs_windows.hpp
    sys/fs/fs_windows.cpp
    sys/clock_windows.cpp)
endif()

set(SYS_LIBS)

if(SYS_LINUX)
  set(SYS_LIBS rt ${SYS_LIBS})
endif()

def_lib(sys SOURCES ${SYS_SRC} LIB_DEPEND ${SYS_LIBS})

set(GLT_SRC
  glt/utils.cpp
  glt/ShaderManager.cpp
  glt/ShaderProgram.cpp
  glt/ShaderCompiler.cpp
  glt/Uniforms.cpp
  glt/Preprocessor.cpp
  glt/GLSLPreprocessor.cpp
  glt/Frame.cpp
  glt/GeometryTransform.cpp
  glt/Transformations.cpp
  glt/ViewFrustum.cpp
  glt/RenderManager.cpp
  glt/RenderTarget.cpp
  glt/TextureHandle.cpp
  glt/TextureRenderTarget.cpp
  glt/TextureRenderTarget3D.cpp
  glt/Mesh.cpp
  glt/glstate.cpp)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

include_directories(${GLEW_INCLUDE_PATH})

set(GLT_LIBS ${GLEW_LIBRARY} ${OPENGL_gl_LIBRARY})

def_lib(glt SOURCES ${GLT_SRC} DEPEND vec-math err sys LIB_DEPEND ${GLT_LIBS})

set(GE_SRC
  ge/GameLoop.cpp
  ge/GameWindow.cpp
  ge/WindowRenderTarget.cpp
  ge/Engine.cpp
  ge/EngineOptions.cpp
  ge/Init.cpp
  ge/Command.cpp
  ge/Commands.cpp
  ge/CommandArgs.cpp
  ge/CommandParams.cpp
  ge/KeyBinding.cpp
  ge/CommandProcessor.cpp
  ge/Tokenizer.cpp
  ge/KeyHandler.cpp
  ge/Camera.cpp)

find_package(SFML 2 COMPONENTS system window graphics REQUIRED)

include_directories(${SFML_INCLUDE_DIR})

set(GE_LIBS ${SFML_LIBRARIES})

def_lib(ge SOURCES ${GE_SRC} DEPEND vec-math err sys glt LIB_DEPEND ${GE_LIBS})