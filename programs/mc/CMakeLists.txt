
include(${PROJECT_SOURCE_DIR}/cmake/program.cmake)

def_program(mc SOURCES mc.cpp DEPEND ge LIB_DEPEND OpenCL)
