include(${PROJECT_SOURCE_DIR}/cmake/program.cmake)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIR})

def_program(smistad-mc SOURCES main.cpp gpu-mc.cpp rawUtilities.cpp openCLGLUtilities.cpp  openCLUtilities.cpp DEPEND ge LIB_DEPEND OpenCL ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

set_target_properties(smistad-mc PROPERTIES
  COMPILE_FLAGS "-fexceptions -frtti")
