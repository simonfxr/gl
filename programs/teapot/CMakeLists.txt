
include(${PROJECT_SOURCE_DIR}/cmake/program.cmake)

def_program(teapot
  SOURCES teapot.cpp parse_sply.cpp dump_bmp.cpp
  DEPEND ge)
