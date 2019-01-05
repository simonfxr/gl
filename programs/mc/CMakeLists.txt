def_program(mc SOURCES mc.cpp DEPEND ge ${OpenCL_LIBRARY})
target_include_directories(mc PRIVATE ${OpenCL_INCLUDE_DIRS})
if(COMP_GCC)
  # BUG in cl.hpp: it can not be used with ubsan
  target_compile_options(mc PRIVATE "-fno-sanitize=undefined")
endif()
