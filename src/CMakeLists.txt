set(FIBER_LIB)
set(FIBER_DEFINES)

if(HAVE_FIBER)
  set(FIBER_LIB fiber)
  set(FIBER_DEFINES -DHAVE_FIBER=1)
endif()

set(bl_SOURCES bl/rand.cpp)
cmu_add_library(bl SOURCES ${bl_SOURCES} DEPEND sfx-bitops::sfx-bitops)

set(BUILD_OPTIMIZED_P 0)
if(CMU_BUILD_TYPE MATCHES "RELWITHDEBINFO|RELEASE|MINSIZEREL")
  set(BUILD_OPTIMIZED_P 1)
endif()

set(config_h ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${config_h} @ONLY)

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

set(SYS_SRC sys/fiber.cpp sys/fs/fs_default.cpp sys/io.cpp sys/io/Stream.cpp
            sys/sys.cpp)

if(CMU_OS_POSIX)
  set(SYS_SRC ${SYS_SRC} sys/clock_unix.cpp sys/fs/fs_unix.cpp
              sys/io/io_unix.cpp)
elseif(CMU_OS_WINDOWS)
  set(SYS_SRC ${SYS_SRC} sys/clock_windows.cpp sys/fs/fs_windows.cpp
              sys/io/io_windows.cpp sys/win_utf_conv.cpp)
endif()

if(CMU_OS_WINDOWS)
  list(APPEND SYS_LIBS shlwapi wsock32 ws2_32)
endif()

cmu_add_library(
  sys
  SOURCES
  ${SYS_SRC}
  ${ERR_SRC}
  DEPEND
  bl
  ${SYS_LIBS}
  ${ERR_LIBS}
  ${FIBER_LIB}
  header-utils)
target_compile_definitions(sys PUBLIC ${FIBER_DEFINES})
if(ERR_DEFINES)
  target_compile_definitions(sys PRIVATE ${ERR_DEFINES})
endif()
target_include_directories(sys PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

cmu_add_library(math SOURCES math/math.cpp DEPEND bl sys)
target_include_directories(math PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

set(GL_REQUIRED_EXTENSIONS_LIST "")
set(sep "")
foreach(v ${GL_REQUIRED_EXTENSIONS})
  set(GL_REQUIRED_EXTENSIONS_LIST
      "${GL_REQUIRED_EXTENSIONS_LIST}${sep}\"${v}\"")
  set(sep ",")
endforeach()

set(GL_OPTIONAL_EXTENSIONS_LIST "")
set(sep "")
foreach(v ${GL_OPTIONAL_EXTENSIONS})
  set(GL_OPTIONAL_EXTENSIONS_LIST
      "${GL_OPTIONAL_EXTENSIONS_LIST}${sep}\"${v}\"")
  set(sep ",")
endforeach()

set(config_gl_h ${CMAKE_CURRENT_BINARY_DIR}/config_gl.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config_gl.h.in ${config_gl_h} @ONLY)

set(GLT_SRC
    glt/Frame.cpp
    glt/GLDebug.cpp
    glt/GLObject.cpp
    glt/GLPerfCounter.cpp
    glt/GLSLPreprocessor.cpp
    glt/GeometryTransform.cpp
    glt/Image.cpp
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
    glt/utils.cpp)

set(GLT_LIBS OpenGL::GL glfw::glfw glad::glad math)

if(CMU_OS_POSIX)
  set(GLT_LIBS ${GLT_LIBS} X11)
endif()

cmu_add_library(
  glt
  SOURCES
  ${GLT_SRC}
  DEPEND
  bl
  sys
  ${GLT_LIBS})
target_include_directories(glt PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

set(GE_SRC
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
    ge/Screenshot.cpp
    ge/Tokenizer.cpp
    ge/WindowRenderTarget.cpp
    ge/ge.cpp)

cmu_add_library(
  ge
  SOURCES
  ${GE_SRC}
  DEPEND
  bl
  sys
  glt
  glfw::glfw)
target_include_directories(ge PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(ge SYSTEM PRIVATE
                           ${CMAKE_SOURCE_DIR}/deps/glfw/deps)
