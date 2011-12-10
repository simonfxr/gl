
include(${PROJECT_SOURCE_DIR}/cmake/program.cmake)

def_program(chan SOURCES chan.cpp DEPEND ge)
def_program(ref_test SOURCES ref_test.cpp test_ref1.cpp DEPEND ge)
