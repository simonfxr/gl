def_program(teapot
  SOURCES teapot.cpp dump_bmp.c
  DEPEND ge)
target_include_directories(teapot PRIVATE .. .)
