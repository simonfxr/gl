
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

set(SYS_LIBS)

set(SYS_SRC
  sys/fiber.cpp
  sys/fs/fs_default.cpp
  sys/io.cpp
  sys/io/Stream.cpp
  sys/mem.cpp
  sys/sys.cpp
  )

if(SYS_UNIX)
  set(SYS_SRC
    ${SYS_SRC}
    sys/clock_unix.cpp
    sys/fs/fs_unix.cpp
    sys/io/io_unix.cpp
    )
elseif(SYS_WINDOWS)
  set(SYS_SRC
    ${SYS_SRC}
    sys/clock_windows.cpp
    sys/fs/fs_windows.cpp
    sys/io/io_windows.cpp
    sys/win_utf_conv.cpp
    )
endif()

if(SYS_LINUX)
  list(APPEND SYS_LIBS rt)
endif()

if(SYS_WINDOWS)
  list(APPEND SYS_LIBS Shlwapi wsock32 ws2_32)
endif()

message(STATUS "SYS_LIBS=${SYS_LIBS}")
def_lib(sys SOURCES ${SYS_SRC} ${ERR_SRC} DEPEND ${SYS_LIBS} ${ERR_LIBS} ${FIBER_LIB})
target_compile_definitions(sys PUBLIC ${FIBER_DEFINES})
if(ERR_DEFINES)
  target_compile_definitions(sys PRIVATE ${ERR_DEFINES})
endif()

set(GLT_SRC
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
  glt/utils.cpp
  )

set(GLT_LIBS OpenGL::GL glfw glad)

if(SYS_LINUX)
  set(GLT_LIBS ${GLT_LIBS} X11)
endif()

def_lib(glt SOURCES ${GLT_SRC} DEPEND sys ${GLT_LIBS})

set(GE_SRC
  ge/NewParam.cpp
  ge/Camera.cpp
  ge/Command.cpp
  ge/CommandArgs.cpp
  ge/CommandParams.cpp
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

set(GE_LIBS ${GLFW_LIBRARY})

def_lib(ge SOURCES ${GE_SRC} DEPEND sys glt ${GE_LIBS})
