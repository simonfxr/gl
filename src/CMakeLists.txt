set(FIBER_LIB)
set(FIBER_DEFINES)
if(HAVE_FIBER)
  set(FIBER_LIB fiber)
  set(FIBER_DEFINES -DHAVE_FIBER=1)
endif()

link_libraries(header-utils)

set(ERR_SRC
  err/err.cpp)

set(ERR_LIBS)

set(ERR_DEFINES)

if(USE_UNWIND_STACKTRACES)
  list(APPEND ERR_DEFINES -DUNWIND_STACKTRACES=1)
  list(APPEND ERR_LIBS PkgConfig::unwind PkgConfig::dw)
endif()

if(SYS_WINDOWS)
	list(APPEND ERR_LIBS Dbghelp)
endif()

set(SYS_SRC
  sys/fs/fs_default.cpp
  sys/io.cpp
  sys/io/Stream.cpp
  sys/sys.cpp
  sys/fiber.cpp
  sys/mem.cpp)

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
  set(SYS_LIBS ${FIBER_LIB} ${SYS_LIBS})
endif()

if(SYS_WINDOWS)
  set(SYS_LIBS Shlwapi ${SYS_LIBS})
endif()

def_lib(sys SOURCES ${SYS_SRC} ${ERR_SRC} DEPEND ${SYS_LIBS} ${ERR_LIBS})
target_compile_definitions(sys PUBLIC ${FIBER_DEFINES})
if(ERR_DEFINES)
  target_compile_definitions(sys PRIVATE ${ERR_DEFINES})
endif()

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
  glt/GLObject.cpp
  glt/GLPerfCounter.cpp
  glt/GLDebug.cpp
  glt/glt.cpp)

set(GLT_LIBS OpenGL::GL glfw glad)

if(SYS_LINUX)
  set(GLT_LIBS ${GLT_LIBS} X11)
endif()

def_lib(glt SOURCES ${GLT_SRC} DEPEND sys ${GLT_LIBS})

set(GE_SRC
  ge/GameLoop.cpp
  ge/GameWindow.cpp
  ge/WindowRenderTarget.cpp
  ge/Engine.cpp
  ge/EngineEvents.cpp
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
  ge/Camera.cpp
  ge/ge.cpp)

set(GE_LIBS
  ${GLFW_LIBRARY})

def_lib(ge SOURCES ${GE_SRC} DEPEND sys glt ${GE_LIBS})
