set(FIBER_LIB)
set(FIBER_DEFINES)

if(HAVE_FIBER)
  set(FIBER_LIB fiber)
  set(FIBER_DEFINES -DHAVE_FIBER=1)
endif()

set(ERR_SRC err/err.cpp)

set(ERR_LIBS)

set(ERR_DEFINES)

if(ENABLE_STACKTRACES)
  list(APPEND ERR_DEFINES -DENABLE_STACKTRACES=1)
  if(CMU_OS_POSIX)
    list(APPEND ERR_LIBS PkgConfig::unwind PkgConfig::dw)
  elseif(CMU_OS_WINDOWS)
    list(APPEND ERR_LIBS Dbghelp)
  endif()
endif()

set(SYS_LIBS)

set(
  SYS_SRC
  sys/fiber.cpp
  sys/fs/fs_default.cpp
  sys/io.cpp
  sys/io/Stream.cpp
  sys/sys.cpp
)

if(CMU_OS_POSIX)
  set(
    SYS_SRC
    ${SYS_SRC}
    sys/clock_unix.cpp
    sys/fs/fs_unix.cpp
    sys/io/io_unix.cpp
  )
elseif(CMU_OS_WINDOWS)
  set(
    SYS_SRC
    ${SYS_SRC}
    sys/clock_windows.cpp
    sys/fs/fs_windows.cpp
    sys/io/io_windows.cpp
    sys/win_utf_conv.cpp
  )
endif()

if(CMU_OS_WINDOWS)
  list(
    APPEND SYS_LIBS
    shlwapi
    wsock32
    ws2_32
  )
endif()

cmu_add_library(
  sys
  SOURCES
  ${SYS_SRC}
  ${ERR_SRC}
  DEPEND
  ${SYS_LIBS}
  ${ERR_LIBS}
  ${FIBER_LIB}
  header-utils
)
target_compile_definitions(sys PUBLIC ${FIBER_DEFINES})
if(ERR_DEFINES)
  target_compile_definitions(sys PRIVATE ${ERR_DEFINES})
endif()

cmu_add_library(math SOURCES math/math.cpp DEPEND sys)

set(
  GLT_SRC
  glt/Frame.cpp
  glt/GLDebug.cpp
  glt/GLObject.cpp
  glt/GLPerfCounter.cpp
  glt/GLSLPreprocessor.cpp
  glt/GeometryTransform.cpp
  glt/Mesh.cpp
  glt/Preprocessor.cpp
  glt/RenderManager.cpp
  glt/RenderTarget.cpp
  glt/ShaderCompiler.cpp
  glt/ShaderManager.cpp
  glt/ShaderProgram.cpp
  glt/TextureData.cpp
  glt/TextureRenderTarget.cpp
  glt/TextureRenderTarget3D.cpp
  glt/TextureSampler.cpp
  glt/Transformations.cpp
  glt/Uniforms.cpp
  glt/ViewFrustum.cpp
  glt/glt.cpp
  glt/type_info.cpp
  glt/utils.cpp
)

set(GLT_LIBS OpenGL::GL glfw::glfw glad::glad math)

if(CMU_OS_POSIX)
  set(GLT_LIBS ${GLT_LIBS} X11)
endif()

cmu_add_library(glt SOURCES ${GLT_SRC} DEPEND sys ${GLT_LIBS})

set(
  GE_SRC
  ge/Camera.cpp
  ge/Command.cpp
  ge/CommandArgs.cpp
  ge/CommandProcessor.cpp
  ge/Commands.cpp
  ge/Engine.cpp
  ge/EngineEvents.cpp
  ge/EngineOptions.cpp
  ge/Event.cpp
  ge/GameLoop.cpp
  ge/GameWindow.cpp
  ge/Init.cpp
  ge/KeyBinding.cpp
  ge/KeyHandler.cpp
  ge/MouseLookPlugin.cpp
  ge/Plugin.cpp
  ge/ReplServer.cpp
  ge/Tokenizer.cpp
  ge/WindowRenderTarget.cpp
  ge/ge.cpp
)

set(GE_LIBS glfw::glfw)

cmu_add_library(ge SOURCES ${GE_SRC} DEPEND sys glt ${GE_LIBS})
