
include(${PROJECT_SOURCE_DIR}/cmake/program.cmake)

def_program(nbody SOURCES nbody.cpp nbody_phys.cpp DEPEND ge)
