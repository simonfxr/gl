
include(${PROJECT_SOURCE_DIR}/cmake/program.cmake)

def_program(hello-gl SOURCES hello-gl.cpp cutil.c DEPEND ge)
