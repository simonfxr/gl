def_program(mc SOURCES mc.cpp DEPEND ge ${OpenCL_LIBRARY})
target_include_directories(mc PRIVATE ${OpenCL_INCLUDE_DIRS})
