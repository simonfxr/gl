
include(${PROJECT_SOURCE_DIR}/cmake/config.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/macros.cmake)

set(MATH_SRC
  math/vec2/impl.cpp
  math/vec3/impl.cpp
  math/vec4/impl.cpp
  math/mat2/impl.cpp
  math/mat3/impl.cpp
  math/mat4/impl.cpp
  math/real/impl.cpp
  math/ivec3/impl.cpp
  math/quat/impl.cpp
  math/plane.cpp)

if(NOT MATH_INLINE)
  def_lib(vec-math SOURCES ${MATH_SRC})
  set(VEC_LIB vec-math)
else()
  set(VEC_LIB)
endif()

set(ERR_SRC
  err/err.cpp)

set(ERR_LIBS)

if(USE_UNWIND_STACKTRACES)
  add_definitions(-DUNWIND_STACKTRACES=1)
  set(ERR_LIBS unwind dw)
endif()

set(SYS_SRC
  sys/fs/fs_default.cpp
  sys/io.cpp
  sys/io/Stream.cpp)

if(SYS_UNIX)
  set(SYS_SRC
    ${SYS_SRC}
    sys/fs/fs_unix.cpp
    sys/clock_unix.cpp
    sys/io/io_unix.cpp)
elseif(SYS_WINDOWS)
  set(SYS_SRC
    ${SYS_SRC}
    sys/fs/fs_windows.cpp
    sys/clock_windows.cpp
    sys/io/io_windows.cpp)
endif()

set(SYS_LIBS)

if(SYS_LINUX)
  set(SYS_LIBS rt ${SYS_LIBS})
endif()

if(SYS_WINDOWS)
  set(SYS_LIBS Shlwapi ${SYS_LIBS})
endif()

def_lib(sys SOURCES ${SYS_SRC} ${ERR_SRC} LIB_DEPEND ${SYS_LIBS} ${ERR_LIBS})

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
  glt/TextureData.cpp
  glt/TextureSampler.cpp
  glt/TextureRenderTarget.cpp
  glt/TextureRenderTarget3D.cpp
  glt/Mesh.cpp
  glt/GLObject.cpp)

set(GLT_LIBS ${GLEW_LIBRARY} ${OPENGL_gl_LIBRARY})

if(SYS_LINUX)
  set(GLT_LIBS ${GLT_LIBS} X11)
endif()

def_lib(glt SOURCES ${GLT_SRC} DEPEND ${VEC_LIB} sys LIB_DEPEND ${GLT_LIBS})

set(GE_SRC
  ge/GameLoop.cpp
  ge/GameWindowGLFW.cpp
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
  ge/ReplServer.cpp
  ge/Plugin.cpp
  ge/MouseLookPlugin.cpp
  ge/Camera.cpp)

set(GE_LIBS 
  ${GLFW_LIBRARY})

def_lib(ge SOURCES ${GE_SRC} DEPEND ${VEC_LIB} sys glt LIB_DEPEND ${GE_LIBS})
