def_program(nbody SOURCES nbody.cpp nbody_phys.cpp DEPEND ge)
target_include_directories(nbody PRIVATE ..)
